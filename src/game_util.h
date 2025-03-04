/*  game_util.h
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

#ifndef GAME_UTIL
#define GAME_UTIL

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct Coords {
    int x;
    int y;
} Coords;

// don't change these
typedef enum Direction {
    NORTH = 0u,
    SOUTH = 1,
    EAST  = 3,
    WEST  = 4,
    INVALID_DIRECTION = 5
} Direction;


/*
 * Use these for ms and second timestamps respectively so we don't accidentally interchange them
 */
typedef int64_t TIME_MS;
typedef time_t  TIME_S;


/*
 * Return true if coordinates x1, y1 overlap with x2, y2.
 */
#define COORDINATES_OVERLAP(x1, y1, x2, y2)(((x1) == (x2)) && ((y1) == (y2)))

/*
 * Halves speed if moving north or south. This accounts for the fact that Y steps are twice as large as X steps.
 */
#define GAME_UTIL_REAL_SPEED(dir, speed)((((dir) == (NORTH)) || ((dir) == (SOUTH))) ? (MAX(1, ((speed) / 2))) : (speed))

/*
 * Return true if dir is a valid Direction.
 */
#define GAME_UTIL_DIRECTION_VALID(dir)((dir) < (INVALID_DIRECTION))

/*
 * Returns cardinal direction mapped to `key`.
 */
Direction game_util_get_direction(int key);

/*
 * Returns the direction that will move `coords_a` closest to `coords_b`.
 *
 * If `inverse` is true, returns the opposite result.
 */
Direction game_util_move_towards(const Coords *coords_a, const Coords *coords_b, bool inverse);

/*
 * Returns a random direction.
 */
Direction game_util_random_direction(void);

/*
 * Moves `coords` one square towards `direction`.
 */
void game_util_move_coords(Direction direction, Coords *coords);

/*
 * Returns a random colour.
 */
int game_util_random_colour(void);

/*
 * Converts relative window coordinates to static game board coordinates
 * and puts the result in `coords`.
 */
void game_util_win_coords_to_board(int win_x, int win_y, int x_left_bound, int y_top_bound, Coords *coords);

/*
 * Converts static game board coordinates to relative window coordinates
 * and puts the result in `coords.
 */
void game_util_board_to_win_coords(int board_x, int board_y, int x_left_bound, int y_top_bound, Coords *coords);

/*
 * Packs an unsigned 32 bit integer `v` into `bytes`.
 */
size_t game_util_pack_u32(uint8_t *bytes, uint32_t v);

/*
 * Unpacks an unsigned 32 bit integer in `bytes` to `v`.
 */
size_t game_util_unpack_u32(const uint8_t *bytes, uint32_t *v);

#endif  // GAME_UTIL
