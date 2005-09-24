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

#include <libwnck/libwnck.h>
#include <glib.h>
#include <glib-object.h>

#include "flurb.h"

/* Not good in the slightest */
#include "devilspie-matcher-always.h"
#include "devilspie-matcher-windowname.h"
#include "devilspie-action-exec.h"
#include "devilspie-action-hide.h"
#include "devilspie-action-layer.h"
#include "devilspie-action-decorate.h"
#include "devilspie-action-savegeometry.h"
#include "devilspie-action-setgeometry.h"
#include "devilspie-action-setworkspace.h"
#include "devilspie-action-resize.h"
#include "devilspie-action-shade.h"
#include "devilspie-action-debug.h"
#include "devilspie-action-setwintype.h"
#include "devilspie-action-opacity.h"

/* Equally foul */
void flurb_init(void) {
  devilspie_matcher_always_get_type();
  devilspie_matcher_windowname_get_type();
  
  devilspie_action_debug_get_type();
  devilspie_action_decorate_get_type();
  devilspie_action_exec_get_type();
  devilspie_action_hide_get_type();
  devilspie_action_layer_get_type();
  devilspie_action_opacity_get_type();
  devilspie_action_resize_get_type();
  devilspie_action_shade_get_type();
  devilspie_action_savegeometry_get_type();
  devilspie_action_setgeometry_get_type();
  devilspie_action_setwintype_get_type();
  devilspie_action_setworkspace_get_type();
}

/**
 * Run a flurb. Called by the window_opened handler
 */
void run_flurb(Flurb * flurb, WnckWindow *window) {
  GList *l;

  /* If there are no matchers, abort the Flurb */
  if (flurb->matchers == NULL) return;
  /* First, run all matchers. If any return false, abort this Flurb */
  for (l = flurb->matchers; l != NULL; l = g_list_next(l)) {
    DevilsPieMatcher *m = l->data;
    if (!devilspie_matcher_test(m, window)) return;
  }
  /* If we got here, this is a matching Flurb. Run all actions. */
  for (l = flurb->actions; l != NULL; l = g_list_next(l)) {
    DevilsPieAction *a = (DevilsPieAction*)l->data;
    devilspie_action_run(a, window);
  }
}

/**
 * Free the memory taken by a particular Flurb. It unrefs the actions
 * and matchers, so you can be sick and share them.
 */
void flurb_free(Flurb *flurb) {
  /* Free the name */
  g_free(flurb->name);
  /* Free the matchers */
  g_list_foreach(flurb->matchers, (GFunc)g_object_unref, NULL);
  g_list_free(flurb->matchers);
  /* Free the actions */
  g_list_foreach(flurb->actions, (GFunc)g_object_unref, NULL);
  g_list_free(flurb->actions); 
  /* Finally, free the struct itself */
  g_free(flurb);
}
