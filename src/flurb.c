#include "glib-object.h"
#include "glib.h"
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
#include "devilspie-action-debug.h"
#include "devilspie-action-setwintype.h"

/* Equally foul */
void flurb_init(void) {
  devilspie_matcher_always_get_type();
  devilspie_matcher_windowname_get_type();
  devilspie_action_hide_get_type();
  devilspie_action_decorate_get_type();
  devilspie_action_layer_get_type();
  devilspie_action_savegeometry_get_type();
  devilspie_action_setgeometry_get_type();
  devilspie_action_setworkspace_get_type();
  devilspie_action_resize_get_type();
  devilspie_action_debug_get_type();
  devilspie_action_setwintype_get_type();
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
