/*
 * Devil's Pie -- a window matching tool.
 * Copyright (C) 2002 Ross Burton <ross@burtonini.com>
 * Licensed under the GPL (2.0 or above)
 * 
 * Inspired by the Matched Windows feature in Sawfish, and implemented
 * as a seperate program so that Metacity can remain clean, lean, and
 * crack-free.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glib/glist.h>
#include <stdlib.h>
#include <libwnck/libwnck.h>
#include "flurb.h"
#include "config-parser.h"
#include "devilspie-matcher.h"
#include "devilspie-action.h"

/**
 * The list of flurbs to execute.
 */
GList *flurbs = NULL;

/**
 * If we apply to existing windows or not.
 */
gboolean apply_to_existing = FALSE;

/**
 * This callback is called whenever a window is opened on a screen.
 */
void window_opened_cb(WnckScreen *screen, WnckWindow *window) {
  g_list_foreach(flurbs, (GFunc)run_flurb, window);
  return;
}

static void init_screens(void) {
  GdkDisplay *display;
  int i, num_screens;

  display = gdk_display_get_default();
  g_assert (display != NULL);
  num_screens = gdk_display_get_n_screens (display);
  for (i = 0 ; i < num_screens; ++i) {
    WnckScreen *screen;
    screen = wnck_screen_get (i);
    /* Connect a callback to the window opened event in libwnck */
    g_signal_connect (screen, "window_opened", (GCallback)window_opened_cb, NULL);
    if (apply_to_existing) wnck_screen_force_update (screen);
  }
}

/*
 * Dedicated to Vicky.
 */
int main(int argc, char **argv) {
  const GOptionEntry options[] = {
    { "apply-to-existing", 'a', 0, G_OPTION_ARG_NONE, &apply_to_existing, N_("Apply to all existing windows instead of just new windows."), NULL },
    { NULL }
  };
  GError *error = NULL;
  GOptionContext *context;

  /* Initialise i18n */
  bindtextdomain (GETTEXT_PACKAGE, DEVILSPIE_LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Initialise GTK+ */
  gtk_init(&argc, &argv);

  /* Parse the arguments */
  context = g_option_context_new ("- Devil's Pie");
  g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, &error);

  /* And finally do the flurb type initialisation */
  flurb_init();

  /* If there are no more options, use $HOME/.devilspie.xml */
  if (argc == 1) {
    char *filename;
    filename = g_build_filename (g_get_home_dir(), ".devilspie.xml", NULL);
    load_configuration(filename);
    g_free(filename);
  } else if (argc == 2) {
    load_configuration(argv[1]);
  } else {
    g_printerr("Too many arguments");
    return 1;
  }

  if (g_list_length (flurbs) == 0) {
    g_printerr("No flurbs loaded, quiting\n");
    return 1;
  }

  init_screens();

  /* Go go go! */
  gtk_main();
  return 0;
}
