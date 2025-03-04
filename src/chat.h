/*  chat.h
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

#ifndef CHAT_H
#define CHAT_H

#include "toxic.h"
#include "windows.h"

/* set CTRL to -1 if we don't want to send a control signal.
   set msg to NULL if we don't want to display a message */
void chat_close_file_receiver(Tox *tox, int filenum, int friendnum, int CTRL);
void kill_chat_window(ToxWindow *self, Toxic *tox);
ToxWindow *new_chat(Tox *tox, uint32_t friendnum);

#endif /* end of include guard: CHAT_H */
