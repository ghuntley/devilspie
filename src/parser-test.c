/*
 * Devil's Pie -- a window matching tool.
 * Copyright (C) 2002-4 Ross Burton <ross@burtonini.com>
 * Licensed under the GPL (2.0 or above)
 * 
 * Inspired by the Matched Windows feature in Sawfish, and implemented
 * as a seperate program so that Metacity can remain clean, lean, and
 * crack-free.
 */

#include "devilspie.h"
#include "flurb.h"
#include "devilspie-matcher.h"
#include "devilspie-action.h"

#include "gtk/gtk.h"
#include "glib/glist.h"
#include "libwnck/libwnck.h"

/**
 * The list of flurbs to execute
 */
GList *flurbs = NULL;

/**
 * Helper to free a single flub.
 */
static void free_a_flurb(Flurb *flurb, gpointer user_data) {
  flurb_free(flurb);
  return;
}

/**
 * Quick test to repeatedly load and free a known configuration file.
 */
int main(int argc, char **argv) {
  int counter = 1000;
  const char* srcdir;
  char *filename;

  /* Find the sample configuration file */
  srcdir = g_getenv("srcdir");
  g_return_val_if_fail (srcdir != NULL, 1);
  filename = g_build_filename (srcdir, "..", "sample-config.xml", NULL);

  /* Initialize Devil's Pie */
  g_type_init();
  flurb_init();

  /* Repeatedly load and free the configuration file */
  while (counter--) {
    load_configuration(filename);
    g_list_foreach (flurbs, (GFunc)free_a_flurb, NULL);
    g_list_free(flurbs);
    flurbs = NULL;
  }

  g_free (filename);
  return 0;
}
