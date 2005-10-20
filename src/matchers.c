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

#include <libwnck/application.h>
#include <libwnck/class-group.h>
#include <libwnck/window.h>
#include <glib.h>
#include "e-sexp.h"
#include "xutils.h"
#include "devilspie.h"
#include "matchers.h"

/*
 * Matchers, functions which evaluate to something interesting about the current
 * window.
 */

/**
 * Return the title of the current window.
 */
ESExpResult *func_window_name(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  ESExpResult *r;
  r = e_sexp_result_new(f, ESEXP_RES_STRING);
  r->value.string = g_strdup(wnck_window_get_name(c->window));
  if (r->value.string == NULL) r->value.string = g_strdup("");
  return r;
}

/**
 * Return the application name (as determined by libwnck) of the current window.
 */
ESExpResult *func_application_name(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  ESExpResult *r;
  r = e_sexp_result_new(f, ESEXP_RES_STRING);
  r->value.string = g_strdup (wnck_application_get_name(wnck_window_get_application(c->window)));
  if (r->value.string == NULL) r->value.string = g_strdup("");
  return r;
}


/**
 * Return the role (as determined by the WM_WINDOW_ROLE hint) of the current window.
 */
ESExpResult *func_window_role(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  ESExpResult *r;
  r = e_sexp_result_new(f, ESEXP_RES_STRING);
  r->value.string = my_wnck_get_string_property_latin1 (wnck_window_get_xid (c->window),
                                                        my_wnck_atom_get("WM_WINDOW_ROLE"));
  if (r->value.string == NULL) r->value.string = g_strdup("");
  return r;
}


/**
 * Return the class of the current window.
 */
ESExpResult *func_window_class(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  ESExpResult *r;
  r = e_sexp_result_new(f, ESEXP_RES_STRING);
  r->value.string = g_strdup (wnck_class_group_get_res_class (wnck_window_get_class_group (c->window)));
  if (r->value.string == NULL) r->value.string = g_strdup("");
  return r;
}

