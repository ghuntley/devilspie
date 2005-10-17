/*
 * Copyright (C) 2005 Ross Burton <ross@burtonini.com>
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

#ifndef _ACTIONS_H
#define _ACTIONS_H

#include "e-sexp.h"

ESExpResult *func_debug(ESExp *f, int argc, ESExpResult **argv, Context *c);

ESExpResult *func_geometry(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_fullscreen(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_maximize(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_maximize_vertically(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_maximize_horizontally(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_minimize(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_shade(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_unshade(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_close(ESExp *f, int argc, ESExpResult **argv, Context *c);

ESExpResult *func_pin(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_unpin(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_set_workspace(ESExp *f, int argc, ESExpResult **argv, Context *c);

ESExpResult *func_skip_tasklist(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_skip_pager(ESExp *f, int argc, ESExpResult **argv, Context *c);

ESExpResult *func_above(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_below(ESExp *f, int argc, ESExpResult **argv, Context *c);

ESExpResult *func_undecorate(ESExp *f, int argc, ESExpResult **argv, Context *c);

ESExpResult *func_wintype(ESExp *f, int argc, ESExpResult **argv, Context *c);

#endif /* _ACTIONS_H */
