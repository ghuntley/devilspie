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

#ifndef _MATCHERS_H
#define _MATCHERS_H

#include "e-sexp.h"

/*
 * Matchers
 */

ESExpResult *func_window_name(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_application_name(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_window_role(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_window_class(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_window_property(ESExp *f, int argc, ESExpResult **argv, Context *c);
ESExpResult *func_window_workspace(ESExp *f, int argc, ESExpResult **argv, Context *c);

#endif /* _MATCHERS_H */
