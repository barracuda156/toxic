/*  conference_commands.h
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

#ifndef CONFERENCE_COMMANDS_H
#define CONFERENCE_COMMANDS_H

#include "toxic.h"
#include "windows.h"

void cmd_conference_chatid(WINDOW *window, ToxWindow *self, Toxic *toxic, int argc, char (*argv)[MAX_STR_SIZE]);
void cmd_conference_set_title(WINDOW *, ToxWindow *, Toxic *, int argc, char (*argv)[MAX_STR_SIZE]);
void cmd_enable_audio(WINDOW *, ToxWindow *, Toxic *, int argc, char (*argv)[MAX_STR_SIZE]);
void cmd_conference_mute(WINDOW *, ToxWindow *, Toxic *, int argc, char (*argv)[MAX_STR_SIZE]);
void cmd_conference_sense(WINDOW *, ToxWindow *, Toxic *, int argc, char (*argv)[MAX_STR_SIZE]);
void cmd_conference_push_to_talk(WINDOW *, ToxWindow *, Toxic *, int argc, char (*argv)[MAX_STR_SIZE]);

#endif /* CONFERENCE_COMMANDS_H */

