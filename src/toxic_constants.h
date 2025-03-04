/*  toxic_constants.h
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

#ifndef TOXIC_CONSTANTS_H
#define TOXIC_CONSTANTS_H

#define UNKNOWN_NAME "Anonymous"
#define DEFAULT_TOX_NAME "Tox User"   /* should always be the same as toxcore's default name */

#define MAX_STR_SIZE TOX_MAX_MESSAGE_LENGTH    /* must be >= TOX_MAX_MESSAGE_LENGTH */
#define MAX_CMDNAME_SIZE 64
#define TOXIC_MAX_NAME_LENGTH 32   /* Must be <= TOX_MAX_NAME_LENGTH */
#define KEY_IDENT_BYTES 6    /* number of hex digits to display for the public key identifier */
#define TIME_STR_SIZE 32
#define COLOR_STR_SIZE 10 /* should fit every color option */

#define NCURSES_DEFAULT_REFRESH_RATE 100
#define NCURSES_GAME_REFRESH_RATE 25

#ifndef MAX_PORT_RANGE
#define MAX_PORT_RANGE 65535
#endif

#define MIN_PASSWORD_LEN 6
#define MAX_PASSWORD_LEN 64

/* Fixes text color problem on some terminals.
   Uncomment if necessary */
/* #define URXVT_FIX */

/* ASCII key codes */
#define T_KEY_ESC        0x1B     /* ESC key */
#define T_KEY_KILL       0x0B     /* ctrl-k */
#define T_KEY_DISCARD    0x15     /* ctrl-u */
#define T_KEY_NEXT       0x10     /* ctrl-p */
#define T_KEY_PREV       0x0F     /* ctrl-o */
#define T_KEY_C_E        0x05     /* ctrl-e */
#define T_KEY_C_A        0x01     /* ctrl-a */
#define T_KEY_C_V        0x16     /* ctrl-v */
#define T_KEY_C_F        0x06     /* ctrl-f */
#define T_KEY_C_H        0x08     /* ctrl-h */
#define T_KEY_C_Y        0x19     /* ctrl-y */
#define T_KEY_C_L        0x0C     /* ctrl-l */
#define T_KEY_C_W        0x17     /* ctrl-w */
#define T_KEY_C_B        0x02     /* ctrl-b */
#define T_KEY_C_R        0x12     /* ctrl-r */
#define T_KEY_C_T        0x14     /* ctrl-t */
#define T_KEY_C_LEFT     0x221    /* ctrl-left arrow */
#define T_KEY_C_RIGHT    0x230    /* ctrl-right arrow */
#define T_KEY_C_UP       0x236    /* ctrl-up arrow */
#define T_KEY_C_DOWN     0x20D    /* ctrl-down arrow */
#define T_KEY_TAB        0x09     /* TAB key */

#define ONLINE_CHAR  "o"
#define OFFLINE_CHAR "o"

typedef enum _FATAL_ERRS {
    FATALERR_MEMORY = -1,           /* heap memory allocation failed */
    FATALERR_FILEOP = -2,           /* critical file operation failed */
    FATALERR_THREAD_CREATE = -3,    /* thread creation failed for critical thread */
    FATALERR_MUTEX_INIT = -4,       /* mutex init for critical thread failed */
    FATALERR_THREAD_ATTR = -5,      /* thread attr object init failed */
    FATALERR_LOCALE_NOT_SET = -6,   /* system locale not set */
    FATALERR_STORE_DATA = -7,       /* store_data failed in critical section */
    FATALERR_INFLOOP = -8,          /* infinite loop detected */
    FATALERR_WININIT = -9,          /* window init failed */
    FATALERR_PROXY = -10,           /* Tox network failed to init using a proxy */
    FATALERR_ENCRYPT = -11,         /* Data file encryption failure */
    FATALERR_TOX_INIT = -12,        /* Tox instance failed to initialize */
    FATALERR_TOXIC_INIT = -13,      /* Toxic instance failed to initialize */
    FATALERR_CURSES = -14,          /* Unrecoverable Ncurses error */
} FATAL_ERRS;

#endif  // TOXIC_CONSTANTS_H
