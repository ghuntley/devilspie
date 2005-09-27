/*
 * Copyright (C) 2001 Havoc Pennington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef WNCK_XUTILS_H
#define WNCK_XUTILS_H

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "libwnck/window.h"

G_BEGIN_DECLS

Atom my_wnck_atom_get  (const char *atom_name);

void my_wnck_change_state (Screen *screen, Window xwindow, gboolean add, Atom state1, Atom state2);

void my_wnck_error_trap_push (void);
int my_wnck_error_trap_pop (void);

char* my_wnck_get_string_property_latin1 (Window  xwindow, Atom atom);

Screen* my_wnck_window_get_xscreen (WnckWindow *window);

G_END_DECLS

#endif /* WNCK_XUTILS_H */
