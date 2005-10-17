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

#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include "devilspie.h"
#include "parser.h"
#include "e-sexp.h"

GList *sexps = NULL;
gboolean debug = FALSE;
Context context = {NULL};

int main(int argc, char **argv)
{
  int count = 0;
  GError *error = NULL;
  char *srcdir;
  const char *testpath, *name;
  GDir *dir;

  g_type_init ();

  srcdir = getenv("srcdir");
  if (!srcdir) srcdir = ".";
  testpath = g_build_filename (srcdir, "../tests", NULL);

  dir = g_dir_open (testpath, 0, &error);
  if (!dir) {
    g_printerr ("Cannot open %s: %s\n", testpath, error->message);
    g_error_free (error);
    return 1;
  }
  
  while ((name = g_dir_read_name (dir)) != NULL) {
    char *filepath;
    gboolean expected;
    ESExp *sexp;
    ESExpResult *result;
    
    if (!g_str_has_suffix (name, ".ds"))
      continue;
    
    if (g_str_has_suffix (name, "-true.ds"))
      expected = TRUE;
    else if (g_str_has_suffix (name, "-false.ds"))
      expected = FALSE;
    else {
      g_printerr ("Cannot determine expected result for %s\n", name);
      return 1;
    }
    
    filepath = g_build_filename (testpath, name, NULL);
    sexp = load_configuration_file (filepath);
    g_free (filepath);
    
    result = e_sexp_eval(sexp);
    if (result->type != ESEXP_RES_BOOL) {
      g_printerr("Invalid result type for test %s\n", name);
      return 1;
    }
    if (result->value.bool != expected) {
      g_printerr("Incorrect result for test %s\n", name);
      return 1;
    }
    
    g_print(".");
    count++;
  }
  g_dir_close (dir);

  if (count) {
    g_print("\nSuccessfully completed %d tests\n", count);
    return 0;
  } else {
    g_print("Didn't find any tests\n");
    return 1;
  }
}
