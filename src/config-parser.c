#include <stdlib.h>
#include <string.h>
#include "glib.h"
#include "glib-object.h"
#include "devilspie.h"
#include "devilspie-matcher.h"
#include "devilspie-action.h"
#include "tristate-dummy.h"
#include "flurb.h"
#define _(x) x

extern GList *flurbs;

/*
 * BIG TODO:
 * Use this GError thing we hear so much about... :)
 */

typedef enum {
  STATE_START,
  STATE_DEVILSPIE,
  STATE_FLURB,
  STATE_MATCHERS,
  STATE_MATCHER,
  STATE_MATCHER_PROPERTY,
  STATE_ACTIONS,
  STATE_ACTION,
  STATE_ACTION_PROPERTY
} ParseState;

typedef struct {
  /* The current state of the parse */
  GSList *states;
  /* A list of flurbs defined */
  GList *flurbs;
  /* A Flurb being worked on */
  Flurb *flurb;
  /* The DevilsPie::Matcher being worked on */
  DevilsPieMatcher *matcher;
  /* The DevilsPie::Action being worked on */
  DevilsPieAction *action;
} ParseInfo;

static void set_error (GError             **err,
                       GMarkupParseContext *context,
                       int                  error_domain,
                       int                  error_code,
                       const char          *format,
                       ...) G_GNUC_PRINTF (5, 6);

static void       push_state (ParseInfo  *info,
                              ParseState  state);
static void       pop_state  (ParseInfo  *info);
static ParseState peek_state (ParseInfo  *info);

static void start_element_handler (GMarkupParseContext  *context,
                                   const gchar          *element_name,
                                   const gchar         **attribute_names,
                                   const gchar         **attribute_values,
                                   gpointer              user_data,
                                   GError              **error);
static void end_element_handler   (GMarkupParseContext  *context,
                                   const gchar          *element_name,
                                   gpointer              user_data,
                                   GError              **error);
static void text_handler          (GMarkupParseContext  *context,
                                   const gchar          *text,
                                   gsize                 text_len,
                                   gpointer              user_data,
                                   GError              **error);

static GMarkupParser devilspie_config_parser = {
  start_element_handler,
  end_element_handler,
  text_handler,
  NULL,
  NULL
};

/* Stolen from Metacity */
static gboolean
all_whitespace (const char *text, int text_len) {
  const char *p, *end;
  p = text;
  end = text + text_len;
  while (p != end) {
    if (!g_ascii_isspace (*p)) return FALSE;
    p = g_utf8_next_char (p);
  }
  return TRUE;
}

typedef struct
{
  const char  *name;
  const char **retloc;
} LocateAttr;

static gboolean
locate_attributes (GMarkupParseContext *context,
                   const char  *element_name,
                   const char **attribute_names,
                   const char **attribute_values,
                   GError     **error,
                   const char  *first_attribute_name,
                   const char **first_attribute_retloc,
                   ...)
{
  va_list args;
  const char *name;
  const char **retloc;
  int n_attrs;
#define MAX_ATTRS 24
  LocateAttr attrs[MAX_ATTRS];
  gboolean retval;
  int i;

  g_return_val_if_fail (first_attribute_name != NULL, FALSE);
  g_return_val_if_fail (first_attribute_retloc != NULL, FALSE);

  retval = TRUE;

  n_attrs = 1;
  attrs[0].name = first_attribute_name;
  attrs[0].retloc = first_attribute_retloc;
  *first_attribute_retloc = NULL;
  
  va_start (args, first_attribute_retloc);

  name = va_arg (args, const char*);
  retloc = va_arg (args, const char**);

  while (name != NULL)
    {
      g_return_val_if_fail (retloc != NULL, FALSE);

      g_assert (n_attrs < MAX_ATTRS);
      
      attrs[n_attrs].name = name;
      attrs[n_attrs].retloc = retloc;
      n_attrs += 1;
      *retloc = NULL;      

      name = va_arg (args, const char*);
      retloc = va_arg (args, const char**);
    }

  va_end (args);

  if (!retval)
    return retval;

  i = 0;
  while (attribute_names[i])
    {
      int j;
      gboolean found;

      found = FALSE;
      j = 0;
      while (j < n_attrs)
        {
          if (strcmp (attrs[j].name, attribute_names[i]) == 0)
            {
              retloc = attrs[j].retloc;

              if (*retloc != NULL)
                {
                  set_error (error, context,
                             G_MARKUP_ERROR,
                             G_MARKUP_ERROR_PARSE,
                             _("Attribute \"%s\" repeated twice on the same <%s> element"),
                             attrs[j].name, element_name);
                  retval = FALSE;
                  goto out;
                }

              *retloc = attribute_values[i];
              found = TRUE;
            }

          ++j;
        }

      if (!found)
        {
          set_error (error, context,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_PARSE,
                     _("Attribute \"%s\" is invalid on <%s> element in this context"),
                     attribute_names[i], element_name);
          retval = FALSE;
          goto out;
        }

      ++i;
    }

 out:
  return retval;
}

static void
set_error (GError **err, GMarkupParseContext *context, int error_domain,
           int error_code, const char *format, ...) {
  int line, ch;
  va_list args;
  char *str;
  
  g_markup_parse_context_get_position (context, &line, &ch);
  
  va_start (args, format);
  str = g_strdup_vprintf (format, args);
  va_end (args);
  
  g_set_error (err, error_domain, error_code,_("Line %d character %d: %s"),
               line, ch, str);
  
  g_free (str);
}

static void
parse_info_init(ParseInfo *info) {
  info->states = g_slist_prepend (NULL, GINT_TO_POINTER (STATE_START));
  info->flurbs = NULL;
}

static void
parse_info_free (ParseInfo *info) {
  g_slist_free (info->states);
  /* We don't free the contents of the list as ownership has been
     passed on */
  /* g_list_free (info->flurbs); */
}

static void
push_state (ParseInfo *info, ParseState state) {
  info->states = g_slist_prepend (info->states, GINT_TO_POINTER (state));
}

static void
pop_state (ParseInfo *info) {
  g_return_if_fail (info->states != NULL);
  info->states = g_slist_remove (info->states, info->states->data);
}

static ParseState
peek_state (ParseInfo *info) {
  g_return_val_if_fail (info->states != NULL, STATE_START);
  return GPOINTER_TO_INT (info->states->data);
}

static void
set_property_with_data(GObject *object, const char *property_name, const char *data)
{
  GParamSpec *property_spec;
  GValue *value;

  property_spec = g_object_class_find_property(G_OBJECT_GET_CLASS(object), property_name);
  if (property_spec == NULL) {
    g_message("fuck");
    /* TODO: output something sane */
  }
  value = g_new0(GValue, 1);
  if (g_type_is_a(property_spec->value_type, G_TYPE_STRING)) {
    g_value_init (value, G_TYPE_STRING);
    g_value_set_string(value, data);
    g_object_set_property(object, property_name, value);
  } else if (g_type_is_a(property_spec->value_type, G_TYPE_INT)) {
    int i;
    g_value_init (value, G_TYPE_INT);
    i = strtol(data, (char**)NULL, 10); /* if I put 0 is it clever? */
    g_value_set_int(value, i);
    g_object_set_property(object, property_name, value);
  } else if (g_type_is_a(property_spec->value_type, G_TYPE_BOOLEAN)) {
    /*
     * TODO: decide whether we want a 0/1 representation of boolean,
     * or a 'true'/'false' representation.
     */
    int i;
    g_value_init (value, G_TYPE_BOOLEAN);
    i = strtol(data, (char**)NULL, 10); /* if I put 0 is it clever? */
    g_value_set_boolean(value, (gboolean)i);
    g_object_set_property(object, property_name, value);
  } else if (g_type_is_a(property_spec->value_type, DEVILSPIE_TYPE_TRISTATE)) {
    DevilsPieTriState tri;
    g_value_init (value, DEVILSPIE_TYPE_TRISTATE);
    if (!strcmp(data, "TRUE")) {
      tri = TRI_TRUE;
    } else if (!strcmp(data, "FALSE")) {
      tri = TRI_FALSE;
    } else {
      tri = TRI_UNSET;
    }
    g_value_set_enum(value, tri);
    g_object_set_property(object, property_name, value);
  } else {
    g_assert_not_reached();
  }
  g_free(value);
}

static void
parse_property (GMarkupParseContext  *context,
              const gchar          *element_name,
              const gchar         **attribute_names,
              const gchar         **attribute_values,
              ParseInfo            *info,
              GError              **error)
{
  const char *name;
  const char *value;

  if (!locate_attributes (context, element_name,
                          attribute_names, attribute_values, error,
                          "name", &name,
                          "value", &value,
                          NULL)) return;
  if (name == NULL) {
    set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
               _("No \"name\" attribute on element <%s>"), element_name);
    return;
  }
  if (value == NULL) {
    set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
               _("No \"value\" attribute on element <%s>"), element_name);
    return;
  }
  switch (peek_state(info)) {
  case STATE_MATCHER_PROPERTY:
    set_property_with_data (G_OBJECT (info->matcher), name, value);
    break;
  case STATE_ACTION_PROPERTY:
    set_property_with_data (G_OBJECT (info->action), name, value);
    break;
  default:
    g_assert_not_reached();
  }
}

static DevilsPieMatcher*
create_matcher (GMarkupParseContext  *context,
              const gchar          *element_name,
              const gchar         **attribute_names,
              const gchar         **attribute_values,
              ParseInfo            *info,
              GError              **error)
{
  const char *type_name;
  GType matcher_type;
  DevilsPieMatcher *matcher;

  if (!locate_attributes (context, element_name,
                          attribute_names, attribute_values, error,
                          "name", &type_name,
                          NULL)) return NULL;

  if (type_name == NULL) {
    set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
               _("No \"name\" attribute on element <%s>"), element_name);
    return NULL;
  }

  matcher_type = g_type_from_name (type_name);
  if (!g_type_is_a(matcher_type, devilspie_matcher_get_type())) {
    set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
               _("Requested type %s is not a known Matcher"), type_name);
    return NULL;
  }

  matcher = g_object_new (matcher_type, NULL);
  return matcher;
}

static DevilsPieAction*
create_action (GMarkupParseContext  *context,
              const gchar          *element_name,
              const gchar         **attribute_names,
              const gchar         **attribute_values,
              ParseInfo            *info,
              GError              **error)
{
  const char *type_name;
  GType action_type;
  DevilsPieAction *action;

  if (!locate_attributes (context, element_name,
                          attribute_names, attribute_values, error,
                          "name", &type_name,
                          NULL)) {
    if (!error)
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Could not parse attributes of <%s>"), element_name);
    return NULL;
  }

  if (type_name == NULL) {
    set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
               _("No \"name\" attribute on element <%s>"), element_name);
    return NULL;
  }

  action_type = g_type_from_name (type_name);
  if (!g_type_is_a(action_type, devilspie_action_get_type())) {
    set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
               _("Requested type %s is not a known Action"), type_name);
    return NULL;
  }

  action = g_object_new (action_type, NULL);
  return action;
}

static void
start_element_handler (GMarkupParseContext *context,
                       const gchar         *element_name,
                       const gchar        **attribute_names,
                       const gchar        **attribute_values,
                       gpointer             user_data,
                       GError             **error)
{
  ParseInfo *info;
  info = (ParseInfo*)user_data;

  switch (peek_state(info)) {
  case STATE_START:
    if (strcmp (element_name, "devilspie") == 0) {
      push_state(info, STATE_DEVILSPIE);
    } else {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Outermost element in config files must be <devilspie> not <%s>"),
                 element_name);
    }
    break;
  case STATE_DEVILSPIE:
    if (strcmp (element_name, "flurb") == 0) {
      const char* name;
      push_state(info, STATE_FLURB);
      if (!locate_attributes (context, element_name,
                              attribute_names, attribute_values, error,
                              "name", &name, NULL)) return;
      info->flurb = g_new0 (Flurb, 1);
      info->flurb->name = g_strdup(name);
      info->flurbs = g_list_append(info->flurbs, info->flurb);
    } else {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Only <flurb> elements are allowed inside <devilspie> not <%s>"),
                 element_name);
    }
    break;
  case STATE_FLURB:
    if (strcmp (element_name, "matchers") == 0) {
      push_state(info, STATE_MATCHERS);
    } else if (strcmp (element_name, "actions") == 0) {
      push_state(info, STATE_ACTIONS);
    } else {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Only <matchers> and <actions> elements are allowed inside <blurb> not <%s>"),
                 element_name);
    }
    break;
  case STATE_MATCHERS:
    if (strcmp (element_name, "matcher") == 0) {
      push_state(info, STATE_MATCHER);
      info->matcher = create_matcher (context, element_name, attribute_names,
                                      attribute_values, info, error);
    } else {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Only <matcher> elements are allowed inside <matchers> not <%s>"),
                 element_name);
    }
    break;
  case STATE_MATCHER:
    if (strcmp (element_name, "property") == 0) {
      push_state(info, STATE_MATCHER_PROPERTY);
      parse_property (context, element_name, attribute_names,
                      attribute_values, info, error);
    } else {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Only <property> elements are allowed inside <matcher> not <%s>"),
                 element_name);
    }
    break;
  case STATE_MATCHER_PROPERTY:
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Element <%s> is not allowed inside <property>"),
                 element_name);
    break;
  case STATE_ACTIONS:
    if (strcmp (element_name, "action") == 0) {
      push_state(info, STATE_ACTION);
      info->action = create_action (context, element_name, attribute_names,
                                      attribute_values, info, error);
    } else {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Only <action> elements are allowed inside <actions> not <%s>"),
                 element_name);
    }
    break;
  case STATE_ACTION:
    if (strcmp (element_name, "property") == 0) {
      push_state(info, STATE_ACTION_PROPERTY);
      parse_property (context, element_name, attribute_names,
                      attribute_values, info, error);
    } else {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Only <property> elements are allowed inside <action> not <%s>"),
                 element_name);
    }
    break;
  case STATE_ACTION_PROPERTY:
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Element <%s> is not allowed inside <property>"),
                 element_name);
    break;
  }
}

static void end_element_handler (GMarkupParseContext  *context,
                                 const gchar          *element_name,
                                 gpointer              user_data,
                                 GError              **error)
{
  ParseInfo *info = (ParseInfo*)user_data;
  switch(peek_state (info)) {
  case STATE_START:
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 _("Can not close element <%s> outside of the root element"),
                 element_name);
    break;
  case STATE_DEVILSPIE:
    pop_state (info);
    g_assert (peek_state (info) == STATE_START);
    break;
  case STATE_FLURB:
    pop_state (info);
    g_assert (peek_state (info) == STATE_DEVILSPIE);
    break;
  case STATE_MATCHERS:
    pop_state (info);
    g_assert (peek_state (info) == STATE_FLURB);
    break;
  case STATE_MATCHER:
    pop_state (info);
    info->flurb->matchers = g_list_append(info->flurb->matchers, info->matcher);
    g_assert (peek_state (info) == STATE_MATCHERS);
    break;
  case STATE_MATCHER_PROPERTY:
    pop_state (info);
    g_assert (peek_state (info) == STATE_MATCHER);
    break;
  case STATE_ACTIONS:
    pop_state (info);
    g_assert (peek_state (info) == STATE_FLURB);
    break;
  case STATE_ACTION:
    pop_state (info);
    info->flurb->actions = g_list_append(info->flurb->actions, info->action);
    g_assert (peek_state (info) == STATE_ACTIONS);
    break;
  case STATE_ACTION_PROPERTY:
    pop_state (info);
    g_assert (peek_state (info) == STATE_ACTION);
    break;
  }
}

static void text_handler (GMarkupParseContext  *context,
                          const gchar          *text,
                          gsize                 text_len,
                          gpointer              user_data,
                          GError              **error)
{
  /* TODO: decode the state enum into a name to output nicely */
  if (!all_whitespace(text, text_len)) {
    set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
               _("No text is allowed inside element <%d>"),
               peek_state(user_data));
  }
}

void load_configuration(const char *filename) {
  char *text;
  unsigned int length;
  GError *error;
  GMarkupParseContext *context;
  ParseInfo info;
  
  error = NULL;
  if (!g_file_get_contents(filename, &text, &length, &error)) {
    g_message("Could not load theme: %s", error->message);
    exit(1);
  }
  /* Sanity check */
  g_assert(text);

  parse_info_init (&info);
  context = g_markup_parse_context_new (&devilspie_config_parser, 0, &info, NULL);
  error = NULL;
  if (g_markup_parse_context_parse (context, text, length, &error))
    if (g_markup_parse_context_end_parse (context, &error))
      g_markup_parse_context_free (context);

  g_free (text);

  if (error) {
    g_message("Could not parse configuration: %s", error->message);
    exit(1);
  }

  flurbs = info.flurbs;

  parse_info_free (&info);
}
