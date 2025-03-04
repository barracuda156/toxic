/*  name_lookup.c
 *
 *
 *  Copyright (C) 2024 Toxic All Rights Reserved.
 *
 *  This file is part of Toxic.
 *
 *  Toxic is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Toxic is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Toxic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "name_lookup.h"

#include <curl/curl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "configdir.h"
#include "curl_util.h"
#include "global_commands.h"
#include "line_info.h"
#include "misc_tools.h"
#include "run_options.h"
#include "toxic.h"
#include "windows.h"

#define NAMESERVER_API_PATH "api"
#define SERVER_KEY_SIZE 32
#define MAX_SERVERS 50
#define MAX_DOMAIN_SIZE 32
#define MAX_SERVER_LINE MAX_DOMAIN_SIZE + (SERVER_KEY_SIZE * 2) + 3

static struct Nameservers {
    int     lines;
    char    names[MAX_SERVERS][MAX_DOMAIN_SIZE];
    char    keys[MAX_SERVERS][SERVER_KEY_SIZE];
} Nameservers;

static struct thread_data {
    ToxWindow *self;
    char    id_bin[TOX_ADDRESS_SIZE];
    char    addr[MAX_STR_SIZE];
    char    msg[MAX_STR_SIZE];
    bool    disabled;
    volatile bool busy;
} t_data;

static struct lookup_thread {
    pthread_t tid;
    pthread_attr_t attr;
} lookup_thread;

static void clear_thread_data(void)
{
    t_data = (struct thread_data) {
        0
    };
}

__attribute__((format(printf, 3, 4)))
static int lookup_error(ToxWindow *self, const Client_Config *c_config, const char *errmsg, ...)
{
    char frmt_msg[MAX_STR_SIZE];

    va_list args;
    va_start(args, errmsg);
    vsnprintf(frmt_msg, sizeof(frmt_msg), errmsg, args);
    va_end(args);

    pthread_mutex_lock(&Winthread.lock);
    line_info_add(self, c_config, false, NULL, NULL, SYS_MSG, 0, 0, "name lookup failed: %s", frmt_msg);
    pthread_mutex_unlock(&Winthread.lock);

    return -1;
}

/* Attempts to load the nameserver list pointed at by path into the Nameservers structure.
 *
 * Returns 0 on success.
 * -1 is reserved.
 * Returns -2 if the supplied path does not exist.
 * Returns -3 if the list does not contain any valid entries.
 */
static int load_nameserver_list(const char *path)
{
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        return -2;
    }

    char line[MAX_SERVER_LINE];

    while (fgets(line, sizeof(line), fp) && Nameservers.lines < MAX_SERVERS) {
        size_t linelen = strlen(line);

        if (linelen < SERVER_KEY_SIZE * 2 + 5) {
            continue;
        }

        if (line[linelen - 1] == '\n') {
            --linelen;
            line[linelen] = '\0';
        }

        const char *name = strtok(line, " ");
        const char *keystr = strtok(NULL, " ");

        if (name == NULL || keystr == NULL) {
            continue;
        }

        if (strlen(keystr) != SERVER_KEY_SIZE * 2) {
            continue;
        }

        const size_t idx = Nameservers.lines;
        snprintf(Nameservers.names[idx], sizeof(Nameservers.names[idx]), "%s", name);

        int res = hex_string_to_bytes(Nameservers.keys[idx], SERVER_KEY_SIZE, keystr);

        if (res == -1) {
            continue;
        }

        ++Nameservers.lines;
    }

    fclose(fp);

    if (Nameservers.lines < 1) {
        return -3;
    }

    return 0;
}

/* Takes address addr in the form "username@domain", puts the username in namebuf,
 * and the domain in dombuf.
 *
 * Returns 0 on success.
 * Returns -1 on failure
 */
static int parse_addr(const char *addr, char *namebuf, size_t namebuf_sz, char *dombuf, size_t dombuf_sz)
{
    if (strlen(addr) >= (MAX_STR_SIZE - strlen(NAMESERVER_API_PATH))) {
        return -1;
    }

    char tmpaddr[MAX_STR_SIZE];
    char *tmpname = NULL;
    char *tmpdom = NULL;

    snprintf(tmpaddr, sizeof(tmpaddr), "%s", addr);
    tmpname = strtok(tmpaddr, "@");
    tmpdom = strtok(NULL, "");

    if (tmpname == NULL || tmpdom == NULL) {
        return -1;
    }

    str_to_lower(tmpdom);
    snprintf(namebuf, namebuf_sz, "%s", tmpname);
    snprintf(dombuf, dombuf_sz, "%s", tmpdom);

    return 0;
}

/* matches input domain name with domains in list and obtains key.
 * Turns out_domain into the full domain we need to make a POST request.
 *
 * Return true on match.
 * Returns false on no match.
 */
static bool get_domain_match(char *pubkey, char *out_domain, size_t out_domain_size, const char *inputdomain)
{
    int i;

    for (i = 0; i < Nameservers.lines; ++i) {
        if (strcmp(Nameservers.names[i], inputdomain) == 0) {
            memcpy(pubkey, Nameservers.keys[i], SERVER_KEY_SIZE);
            snprintf(out_domain, out_domain_size, "https://%s/%s", Nameservers.names[i], NAMESERVER_API_PATH);
            return true;
        }
    }

    return false;
}

/* Converts Tox ID string contained in recv_data to binary format and puts it in thread's ID buffer.
 *
 * Returns 0 on success.
 * Returns -1 on failure.
 */
#define ID_PREFIX "\"tox_id\": \""
static int process_response(struct Recv_Curl_Data *recv_data)
{
    size_t prefix_size = strlen(ID_PREFIX);

    if (recv_data->length < TOX_ADDRESS_SIZE * 2 + prefix_size) {
        return -1;
    }

    const char *IDstart = strstr(recv_data->data, ID_PREFIX);

    if (IDstart == NULL) {
        return -1;
    }

    if (strlen(IDstart) < TOX_ADDRESS_SIZE * 2 + prefix_size) {
        return -1;
    }

    char ID_string[TOX_ADDRESS_SIZE * 2 + 1];
    memcpy(ID_string, IDstart + prefix_size, TOX_ADDRESS_SIZE * 2);
    ID_string[TOX_ADDRESS_SIZE * 2] = 0;

    if (tox_pk_string_to_bytes(ID_string, strlen(ID_string), t_data.id_bin, sizeof(t_data.id_bin)) == -1) {
        return -1;
    }

    return 0;
}

static void *lookup_thread_func(void *data)
{
    struct curl_slist *headers = NULL;
    CURL *c_handle = NULL;
    struct Recv_Curl_Data *recv_data = NULL;

    Toxic *toxic = (Toxic *) data;

    if (toxic == NULL) {
        goto on_exit;
    }

    const Client_Config *c_config = toxic->c_config;
    const Run_Options *run_opts = toxic->run_opts;

    ToxWindow *self = t_data.self;

    char input_domain[MAX_STR_SIZE];
    char name[MAX_STR_SIZE];

    if (parse_addr(t_data.addr, name, sizeof(name), input_domain, sizeof(input_domain)) == -1) {
        lookup_error(self, c_config, "Input must be a 76 character Tox ID or an address in the form: username@domain");
        goto on_exit;
    }

    char nameserver_key[SERVER_KEY_SIZE];
    char real_domain[MAX_DOMAIN_SIZE];

    if (!get_domain_match(nameserver_key, real_domain, sizeof(real_domain), input_domain)) {
        lookup_error(self, c_config, "Name server domain not found.");
        goto on_exit;
    }

    c_handle = curl_easy_init();

    if (c_handle == NULL) {
        lookup_error(self, c_config, "curl handler error");
        goto on_exit;
    }

    recv_data = calloc(1, sizeof(struct Recv_Curl_Data));

    if (recv_data == NULL) {
        lookup_error(self, c_config, "memory allocation error");
        goto on_exit;
    }

    char post_data[MAX_STR_SIZE + 30];

    snprintf(post_data, sizeof(post_data), "{\"action\": 3, \"name\": \"%s\"}", name);

    headers = curl_slist_append(headers, "Content-Type: application/json");

    headers = curl_slist_append(headers, "charsets: utf-8");

    int ret = curl_easy_setopt(c_handle, CURLOPT_HTTPHEADER, headers);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "Failed to set http headers (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_URL, real_domain);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "Failed to set url (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_WRITEFUNCTION, curl_cb_write_data);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "Failed to set write function callback (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_WRITEDATA, recv_data);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "Failed to set write data (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "Failed to set useragent (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_POSTFIELDS, post_data);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "Failed to set post data (libcurl error %d)", ret);
        goto on_exit;
    }

    int proxy_ret = set_curl_proxy(c_handle, run_opts->proxy_address, run_opts->proxy_port, run_opts->proxy_type);

    if (proxy_ret != 0) {
        lookup_error(self, c_config, "Failed to set proxy (error %d)\n", proxy_ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "TLS could not be enabled (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "TLSv1.2 could not be set (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_setopt(c_handle, CURLOPT_SSL_CIPHER_LIST, TLS_CIPHER_SUITE_LIST);

    if (ret != CURLE_OK) {
        lookup_error(self, c_config, "Failed to set TLS cipher list (libcurl error %d)", ret);
        goto on_exit;
    }

    ret = curl_easy_perform(c_handle);

    if (ret != CURLE_OK) {
        /* If system doesn't support any of the specified ciphers suites, fall back to default */
        if (ret == CURLE_SSL_CIPHER) {
            ret = curl_easy_setopt(c_handle, CURLOPT_SSL_CIPHER_LIST, NULL);

            if (ret != CURLE_OK) {
                lookup_error(self, c_config, "Failed to set TLS cipher list (libcurl error %d)", ret);
                goto on_exit;
            }

            ret = curl_easy_perform(c_handle);
        }

        if (ret != CURLE_OK) {
            lookup_error(self, c_config, "HTTPS lookup error (libcurl error %d)", ret);
            goto on_exit;
        }
    }

    if (process_response(recv_data) == -1) {
        lookup_error(self, c_config, "Bad response.");
        goto on_exit;
    }

    pthread_mutex_lock(&Winthread.lock);
    cmd_add_helper(self, toxic, t_data.id_bin, t_data.msg);
    pthread_mutex_unlock(&Winthread.lock);

on_exit:

    free(recv_data);
    curl_slist_free_all(headers);  // passing null has no effect
    curl_easy_cleanup(c_handle);  // passing null has no effect
    clear_thread_data();
    pthread_attr_destroy(&lookup_thread.attr);
    pthread_exit(NULL);
}

/* Attempts to do a tox name lookup.
 *
 * Returns true on success.
 */
bool name_lookup(ToxWindow *self, Toxic *toxic, const char *id_bin, const char *addr, const char *message)
{
    const Client_Config *c_config = toxic->c_config;

    if (t_data.disabled) {
        line_info_add(self, c_config, false, NULL, NULL, SYS_MSG, 0, 0, "nameservers list is empty or does not exist.");
        return false;
    }

    if (t_data.busy) {
        line_info_add(self, c_config, false, NULL, NULL, SYS_MSG, 0, 0,
                      "Please wait for previous name lookup to finish.");
        return false;
    }

    snprintf(t_data.id_bin, sizeof(t_data.id_bin), "%s", id_bin);
    snprintf(t_data.addr, sizeof(t_data.addr), "%s", addr);
    snprintf(t_data.msg, sizeof(t_data.msg), "%s", message);
    t_data.self = self;
    t_data.busy = true;

    if (pthread_attr_init(&lookup_thread.attr) != 0) {
        line_info_add(self, c_config, false, NULL, NULL, SYS_MSG, 0, RED, "Error: lookup thread attr failed to init");
        clear_thread_data();
        return false;
    }

    if (pthread_attr_setdetachstate(&lookup_thread.attr, PTHREAD_CREATE_DETACHED) != 0) {
        line_info_add(self, c_config, false, NULL, NULL, SYS_MSG, 0, RED, "Error: lookup thread attr failed to set");
        pthread_attr_destroy(&lookup_thread.attr);
        clear_thread_data();
        return false;
    }

    if (pthread_create(&lookup_thread.tid, &lookup_thread.attr, lookup_thread_func, (void *) toxic) != 0) {
        line_info_add(self, c_config, false, NULL, NULL, SYS_MSG, 0, RED, "Error: lookup thread failed to init");
        pthread_attr_destroy(&lookup_thread.attr);
        clear_thread_data();
        return false;
    }

    return true;
}

int name_lookup_init(const char *nameserver_path, int curl_init_status)
{
    if (curl_init_status != 0) {
        t_data.disabled = true;
        return -1;
    }

    const char *path = !string_is_empty(nameserver_path) ? nameserver_path : PACKAGE_DATADIR "/nameservers";
    const int ret = load_nameserver_list(path);

    if (ret != 0) {
        t_data.disabled = true;
        return ret;
    }

    return 0;
}
