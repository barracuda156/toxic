/*  game_base.h
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

#ifndef GAME_BASE
#define GAME_BASE

#include <ncurses.h>
#include <time.h>

#include <tox/tox.h>

#include "game_util.h"
#include "windows.h"

#define GAME_BORDER_COLOUR BAR_SOLID


/* Max size of a default square game window */
#define GAME_MAX_SQUARE_Y_DEFAULT 26
#define GAME_MAX_SQUARE_X_DEFAULT (GAME_MAX_SQUARE_Y_DEFAULT * 2)

/* Max size of a large square game window */
#define GAME_MAX_SQUARE_Y_LARGE 52
#define GAME_MAX_SQUARE_X_LARGE (GAME_MAX_SQUARE_Y_LARGE * 2)

/* Max size of a default size rectangle game window */
#define GAME_MAX_RECT_Y_DEFAULT 24
#define GAME_MAX_RECT_X_DEFAULT (GAME_MAX_RECT_Y_DEFAULT * 4)

/* Max size of a large rectangle game window */
#define GAME_MAX_RECT_Y_LARGE 52
#define GAME_MAX_RECT_X_LARGE (GAME_MAX_RECT_Y_LARGE * 4)

/* Maximum length of a game message set with game_set_message() */
#define GAME_MAX_MESSAGE_SIZE 64

/* Default number of seconds a game message is displayed for */
#define GAME_MESSAGE_DEFAULT_TIMEOUT 3


/***** START NETWORKING CONSTANTS *****/

/* Header starts after custom packet type byte. Comprised of: NetworkVersion (1b) + GameType (1b) + id (4b) */
#define GAME_PACKET_HEADER_SIZE (1 + 1 + sizeof(uint32_t))

/* Max size of a game packet including the header */
#define GAME_MAX_PACKET_SIZE 1024

/* Max size of a game packet payload */
#define GAME_MAX_DATA_SIZE (GAME_MAX_PACKET_SIZE - GAME_PACKET_HEADER_SIZE - 1)

/* Current version of networking protocol */
#define GAME_NETWORKING_VERSION 0x01

/***** END NETWORKING CONSTANTS *****/


typedef void cb_game_update_state(GameData *game, void *cb_data);
typedef void cb_game_render_window(GameData *game, WINDOW *window, void *cb_data);
typedef void cb_game_kill(GameData *game, void *cb_data);
typedef void cb_game_pause(GameData *game, bool is_paused, void *cb_data);
typedef void cb_game_key_press(GameData *game, int key, void *cb_data);
typedef void cb_game_on_packet(GameData *game, const uint8_t *data, size_t length, void *cb_data);

typedef enum GamePacketType {
    GP_Invite = 0u,
    GP_Data,
} GamePacketType;

typedef enum GameWindowShape {
    GW_ShapeSquare = 0u,
    GW_ShapeSquareLarge,
    GW_ShapeRectangle,
    GW_ShapeRectangleLarge,
    GW_ShapeInvalid,
} GameWindowShape;

typedef enum GameStatus {
    GS_None = 0u,
    GS_Paused,
    GS_Running,
    GS_Finished,
    GS_Invalid,
} GameStatus;

typedef enum GameType {
    GT_Centipede = 0u,
    GT_Chess,
    GT_Life,
    GT_Snake,
    GT_Invalid,
} GameType;

typedef struct GameMessage {
    char         message[GAME_MAX_MESSAGE_SIZE + 1];
    size_t       length;
    const Coords *coords;          // pointer to coords so we can track movement
    Coords       original_coords;  // static coords at time of being set
    time_t       timeout;
    time_t       set_time;
    int          attributes;
    int          colour;
    Direction    direction;
    bool         sticky;
    bool         priority;
} GameMessage;

struct GameData {
    TIME_MS    last_frame_time;
    TIME_MS    update_interval;  // determines the refresh rate (lower means faster)
    long int   score;
    size_t     high_score;
    int        lives;
    size_t     level;
    GameStatus status;
    GameType   type;

    bool       is_multiplayer;
    bool       winner;  // true if you won the game

    bool       show_lives;
    bool       show_score;
    bool       show_high_score;
    bool       show_level;

    GameMessage *messages;
    size_t      messages_size;

    int        game_max_x; // max dimensions of game window
    int        game_max_y;

    int        parent_max_x; // max dimensions of parent window
    int        parent_max_y;

    int64_t    window_id;
    WINDOW     *window;

    Toxic      *toxic;  // must be locked with Winthread mutex

    GameWindowShape    window_shape;

    uint32_t id;  // indentifies multiplayer instance
    uint32_t friend_number; // friendnumber associated with parent window

    cb_game_update_state *cb_game_update_state;
    void *cb_game_update_state_data;

    cb_game_render_window *cb_game_render_window;
    void *cb_game_render_window_data;

    cb_game_kill *cb_game_kill;
    void *cb_game_kill_data;

    cb_game_pause *cb_game_pause;
    void *cb_game_pause_data;

    cb_game_key_press *cb_game_key_press;
    void *cb_game_key_press_data;

    cb_game_on_packet *cb_game_on_packet;
    void *cb_game_on_packet_data;
};

/*
 * Sets the callback for game state updates.
 */
void game_set_cb_update_state(GameData *game, cb_game_update_state *func, void *cb_data);

/*
 * Sets the callback for frame rendering.
 */
void game_set_cb_render_window(GameData *game, cb_game_render_window *func, void *cb_data);

/*
 * Sets the callback for game termination.
 */
void game_set_cb_kill(GameData *game, cb_game_kill *func, void *cb_data);

/*
 * Sets the callback for the game pause event.
 */
void game_set_cb_on_pause(GameData *game, cb_game_pause *func, void *cb_data);

/*
 * Sets the callback for the key press event.
 */
void game_set_cb_on_keypress(GameData *game, cb_game_key_press *func, void *cb_data);

/*
 * Sets the callback for the game packet event.
 */
void game_set_cb_on_packet(GameData *game, cb_game_on_packet *func, void *cb_data);

/*
 * Initializes game instance.
 *
 * `type` must be a valid GameType.
 *
 * `id` should be a unique integer to indentify the game instance. If we're being invited to a game
 *  this identifier should be sent via the invite packet.
 *
 * if `multiplayer_data` is non-null it contains information that we received from the inviter
 * necessary to initialize the game state.
 *
 * if `self_host` is true, the caller is the host of the game. If the game is not initialized from a
 * friend's chat window this parameter has no effect.
 *
 * Return 0 on success.
 * Return -1 if screen is too small.
 * Return -2 on network related error.
 * Return -3 if multiplayer game is being initialized outside of a contact's window.
 * Return -4 on other failure.
 */
int game_initialize(const ToxWindow *self, Toxic *toxic, GameType type, uint32_t id, const uint8_t *multiplayer_data,
                    size_t length, bool self_host);

/*
 * Sets game window to `shape`.
 *
 * This must be called on game initialization.
 *
 * Return 0 on success.
 * Return -1 if window is too small or shape is invalid.
 * Return -2 if function is called while the game state is valid.
 */
int game_set_window_shape(GameData *game, GameWindowShape shape);

/*
 * Returns the GameType associated with `game_string`.
 */
GameType game_get_type(const char *game_string);

/*
 * Returns the name represented as a string associated with `type`.
 */
const char *game_get_name_string(GameType type);

/*
 * Prints all available games to window associated with `self`.
 */
void game_list_print(ToxWindow *self, const Client_Config *c_config);

/*
 * Return true if game `type` has a multiplayer mode.
 */
bool game_type_has_multiplayer(GameType type);

/*
 * Returns true if coordinates designated by `x` and `y` are within the game window boundaries.
 */
bool game_coordinates_in_bounds(const GameData *game, int x, int y);

/*
 * Put random coordinates that fit within the game window in `coords`.
 */
void game_random_coords(const GameData *game, Coords *coords);

/*
 * Gets the current max dimensions of the game window.
 */
void game_max_x_y(const GameData *game, int *x, int *y);

/*
 * Returns the respective coordinate boundary of the game window.
 */
int game_y_bottom_bound(const GameData *game);
int game_y_top_bound(const GameData *game);
int game_x_right_bound(const GameData *game);
int game_x_left_bound(const GameData *game);

/*
 * Toggle whether the respective game info is shown around the game window.
 */
void game_show_score(GameData *game, bool show_score);
void game_show_high_score(GameData *game, bool show_high_score);
void game_show_lives(GameData *game, bool show_lives);
void game_show_level(GameData *game, bool show_level);

/*
 * Sends a notification to the window associated with `game`.
 *
 * `message` - the notification message that will be displayed.
 */
void game_window_notify(const GameData *game, const char *message);

/*
 * Updates game score.
 */
void game_update_score(GameData *game, long int points);

/*
 * Sets game score to `val`.
 */
void game_set_score(GameData *game, long int score);

/*
 * Returns the game's current score.
 */
long int game_get_score(const GameData *game);

/*
 * Increments level.
 *
 * This function should be called on initialization if game wishes to display level.
 */
void game_increment_level(GameData *game);

/*
 * Updates lives with `amount`.
 *
 * If lives becomes negative the lives counter will not be drawn.
 */
void game_update_lives(GameData *game, int amount);

/*
 * Returns the remaining number of lives for the game.
 */
int game_get_lives(const GameData *game);

/*
 * Returns the current level.
 */
size_t game_get_current_level(const GameData *game);

/*
 * Sets the game status to `status`.
 */
void game_set_status(GameData *game, GameStatus status);

/*
 * Sets winner flag. This should only be called when the game status is set to finished.
 */
void game_set_winner(GameData *game, bool winner);

/*
 * Sets the game base update interval.
 *
 * Lower values of `update_interval` make the game faster, where 1 is the fastest and 50 is slowest.
 * If this function is never called the game chooses a reasonable default.
 */
void game_set_update_interval(GameData *game, TIME_MS update_interval);

/*
 * Creates a message `message` of size `length` to be displayed at `coords` for `timeout` seconds.
 *
 * Message must be no greater than GAME_MAX_MESSAGE_SIZE bytes in length.
 *
 * If `sticky` is true the message will follow coords if they move.
 *
 * If `dir` is a valid direction, the message will be positioned a few squares away from `coords`
 * so as to not overlap with its associated object.
 *
 * If `timeout` is zero, the default timeout value will be used.
 *
 * If `priority` true, messages will be printed on top of game objects.
 *
 * Return 0 on success.
 * Return -1 on failure.
 */
int game_set_message(GameData *game, const char *message, size_t length, Direction dir, int attributes, int colour,
                     time_t timeout, const Coords *coords, bool sticky, bool priority);

/*
 * Returns true if game should update an object's state according to its last moved time and current speed.
 *
 * This is used to independently control the speed of various game objects.
 */
bool game_do_object_state_update(const GameData *game, TIME_MS current_time, TIME_MS last_moved_time, TIME_MS speed);

/*
 * Returns the current wall time in milliseconds.
 */
TIME_MS get_time_millis(void);

/*
 * Ends game associated with `self` and cleans up.
 */
void game_kill(ToxWindow *self, Windows *windows, const Client_Config *c_config);

/*
 * Sends a packet containing payload `data` of size `length` to the friendnumber associated with the game's
 * parent window.
 *
 * `length` must not exceed GAME_MAX_DATA_SIZE bytes.
 *
 * `packet_type` should be GP_Invite for an invite packet or GP_Data for all other game data.
 */
int game_packet_send(const GameData *game, const uint8_t *data, size_t length, GamePacketType packet_type);

#endif // GAME_BASE

