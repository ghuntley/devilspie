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

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "e-sexp.h"
#include "devilspie.h"
#include "parser.h"
#include "logical.h"
#include "matchers.h"
#include "actions.h"

/**
 * Table mapping function names to their implementations.
 */
static const struct {
  char *name;
  void *func;
  /* TRUE if a function can perform shortcut evaluation or doesn't execute everything, FALSE otherwise */
  gboolean shortcut;
} symbols[] = {
  /* Logical operations */
  { "is", func_is, FALSE },
  { "contains", func_contains, FALSE },
  { "matches", func_matches, FALSE },
  /* Matchers */
  { "window_name", func_window_name, FALSE },
  { "window_role", func_window_role, FALSE },
  { "window_class", func_window_class, FALSE },
  { "application_name", func_application_name, FALSE },
  { "window_property", func_window_property, FALSE },
  /* Actions */
  { "debug", func_debug, FALSE },
  { "print", func_print, FALSE },
  { "geometry", func_geometry, FALSE },
  { "fullscreen", func_fullscreen, FALSE },
  { "focus", func_focus, FALSE },
  { "center", func_center, FALSE },
  { "maximize", func_maximize, FALSE },
  { "maximize_vertically", func_maximize_vertically, FALSE },
  { "maximize_horizontally", func_maximize_horizontally, FALSE },
  { "unmaximize", func_unmaximize, FALSE },
  { "minimize", func_minimize, FALSE },
  { "unminimize", func_unminimize, FALSE },
  { "shade", func_shade, FALSE },
  { "unshade", func_unshade, FALSE },
  { "close", func_close, FALSE },
  { "pin", func_pin, FALSE },
  { "unpin", func_unpin, FALSE },
  { "stick", func_stick, FALSE },
  { "unstick", func_unstick, FALSE },
  { "set_workspace", func_set_workspace, FALSE },
  { "skip_pager", func_skip_pager, FALSE },
  { "skip_tasklist", func_skip_tasklist, FALSE },
  { "above", func_above, FALSE },
  { "below", func_below, FALSE },
  { "undecorate", func_undecorate, FALSE },
  { "wintype", func_wintype, FALSE },
  { "opacity", func_opacity, FALSE },

};

/**
 * Load a single configuration file.
 */
ESExp *load_configuration_file (const char *path)
{
  /* TODO: GError argument */
  ESExp *sexp = NULL;
  size_t i;
  FILE *f;

  g_return_val_if_fail (path != NULL, NULL);

  if (debug) g_printerr(_("Loading %s\n"), path);

  if (!g_file_test (path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
    if (debug) g_printerr(_("%s is not a normal file, skipping\n"), path);
    return NULL;
  }

  f = fopen(path, "r");
  if (!f) {
    g_printerr(_("Cannot open %s\n"), path);
    return NULL;
  }

  sexp = e_sexp_new ();
  for(i=0; i < sizeof(symbols)/sizeof(symbols[0]); i++) {
    if (symbols[i].shortcut) {
      e_sexp_add_ifunction(sexp, 0, symbols[i].name, symbols[i].func, &context);
    } else {
      e_sexp_add_function(sexp, 0, symbols[i].name, symbols[i].func, &context);
    }
  }
  
  e_sexp_input_file(sexp, fileno(f));
    
  if (e_sexp_parse(sexp) == -1) {
    g_printerr(_("Cannot parse %s: %s\n"), path, e_sexp_error (sexp));
    g_object_unref (sexp);
    fclose(f);
    return NULL;
  }

  fclose(f);

  return sexp;
}

/**
 * Load all *.ds files in a particular directory.
 */
static void load_dir (const char *path)
{
  /* TODO: GError argument*/
  GError *error = NULL;
  GDir *dir;
  const char *name;

  g_return_if_fail (path != NULL);

  if (debug) g_printerr(_("Loading %s\n"), path);

  if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
    if (debug) g_printerr(_("%s doesn't exist\n"), path);
    return;
  }

  if (!g_file_test (path, G_FILE_TEST_IS_DIR)) {
    g_printerr(_("%s isn't a directory\n"), path);
    return;
  }

  dir = g_dir_open (path, 0, &error);
  if (!dir) {
    g_printerr (_("Cannot open %s: %s\n"), path, error->message);
    g_error_free (error);
    return;
  }

  while ((name = g_dir_read_name (dir)) != NULL) {
    char *filepath;
    ESExp *s;

    if (!g_str_has_suffix (name, ".ds"))
      continue;
    
    filepath = g_build_filename (path, name, NULL);
    s = load_configuration_file (filepath);
    if (s) sexps = g_list_append (sexps, s);
    g_free (filepath);
  }

  g_dir_close (dir);
}

/**
 * Load the configuration files.  This will load all .ds files in
 * ${sysconfdir}/devilspie and ~/.devilspie/.
 */
void load_configuration(void) {
  char *p;

  p = g_build_filename (SYSCONFDIR, "devilspie", NULL);
  load_dir (p);
  g_free (p);

  p = g_build_filename (g_get_home_dir (), ".devilspie", NULL);
  load_dir (p);
  g_free (p);
}
