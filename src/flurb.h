#include "glib/glist.h"

/* A Flurb is a list of predicates, and a list of actions */

typedef struct _Flurb Flurb;
struct _Flurb {
  char *name;
  GList *matchers;
  GList *actions;
};

void flurb_init(void);
void flurb_free(Flurb *flurb);
