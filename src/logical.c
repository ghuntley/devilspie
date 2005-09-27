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

#include <string.h>
#include <regex.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "e-sexp.h"
#include "devilspie.h"
#include "logical.h"

/*
 * Logical operators.
 */

/**
 * String equality, (is a b) means a is the same as b.
 */
ESExpResult *func_is(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  ESExpResult *r;

  if (argc != 2 || argv[0]->type != ESEXP_RES_STRING || argv[1]->type != ESEXP_RES_STRING) {
    g_printerr(_("is expects two string arguments\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  r = e_sexp_result_new(f, ESEXP_RES_BOOL);
  /* TODO: decent UTF-8 string compare */
  r->value.bool = strcmp(argv[0]->value.string, argv[1]->value.string) == 0;
  return r;
}

/**
 * Substring, (contains haystack needle) means haystack contains needle.
 */
ESExpResult *func_contains(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  ESExpResult *r;

  if (argc != 2 || argv[0]->type != ESEXP_RES_STRING || argv[1]->type != ESEXP_RES_STRING) {
    g_printerr(_("contains expects two string arguments\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  r = e_sexp_result_new(f, ESEXP_RES_BOOL);
  /* TODO: decent UTF-8 string compare */
  r->value.bool = strstr(argv[0]->value.string, argv[1]->value.string) != NULL;
  return r;
}

/**
 * Regexp matches, (matches str pattern) means the regexp pattern matches str.
 */
ESExpResult *func_matches(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  /* TODO: cache regex_t objects to avoid continual re-creation */
  const char *pattern, *string;
  regex_t regex;
  ESExpResult *r;
  int res;
  
  if (argc != 2 || argv[0]->type != ESEXP_RES_STRING || argv[1]->type != ESEXP_RES_STRING) {
    g_printerr(_("matches expects two string arguments\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  pattern = argv[1]->value.string;
  string = argv[0]->value.string;

  res = regcomp (&regex, pattern, REG_EXTENDED|REG_NOSUB);
  if (res != 0) {
    char buffer[255];
    regerror (res, &regex, buffer, 255);
    g_warning (_("Invalid regular expression '%s': %s"), pattern, buffer);
    return e_sexp_result_new_bool(f, FALSE);
  }

  r = e_sexp_result_new(f, ESEXP_RES_BOOL);
  r->value.bool = (regexec(&regex, string, 0, NULL, 0) == 0) ? TRUE : FALSE;
  regfree (&regex);
  return r;
}
