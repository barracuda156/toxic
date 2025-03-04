/*  x11focus.h
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

#ifndef X11FOCUS_H
#define X11FOCUS_H

#include <stdbool.h>

#ifndef __APPLE__
#include <X11/Xlib.h>

struct X11_Focus {
    Display *display;
    Window terminal_window;
};

typedef struct X11_Focus X11_Focus;

int init_x11focus(X11_Focus *focus);
void terminate_x11focus(X11_Focus *focus);
bool is_focused(const X11_Focus *focus);

#endif /* __APPLE__ */
#endif /* X11FOCUS */
