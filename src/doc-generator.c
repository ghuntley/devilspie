#include <libxml/xmlwriter.h>
#include <glib-object.h>
#include <glib.h>

#include "flurb.h"
#include "devilspie-matcher.h"
#include "devilspie-action.h"

         
static void output_range(xmlTextWriterPtr writer, GParamSpec *param) {
  g_return_if_fail (writer != NULL);
  g_return_if_fail (param != NULL);
  GType type = G_PARAM_SPEC_TYPE (param);
#define OUTPUT_RANGE(t, T, format) \
       if (type == G_TYPE_PARAM_##T) { \
         GParamSpec##t *p = G_PARAM_SPEC_##T (param); \
         if (p->minimum != G_MIN##T) xmlTextWriterWriteAttribute (writer, "minimum", g_strdup_printf(format, p->minimum)); \
         if (p->maximum != G_MAX##T) xmlTextWriterWriteAttribute (writer, "maximum", g_strdup_printf(format, p->maximum)); \
       }
  OUTPUT_RANGE(Float, FLOAT, "%.2f");
  OUTPUT_RANGE(Double, DOUBLE, "%.2f");
  //OUTPUT_RANGE(Char, CHAR, "%d");
  //OUTPUT_RANGE(UChar, UCHAR, "%u");
  OUTPUT_RANGE(Int, INT, "%d");
  //OUTPUT_RANGE(UInt, UINT, "%u");
  OUTPUT_RANGE(Long, LONG, "%ld");
  //OUTPUT_RANGE(ULong, ULONG, "%lu");
  OUTPUT_RANGE(Int64, INT64, "%lld");
  //OUTPUT_RANGE(UInt64, UINT64, "%llu");
}

static void handle_properties(xmlTextWriterPtr writer, GType t) {
  GObjectClass *class;
  GParamSpec **props;
  int i;
  g_return_if_fail (writer != NULL);

  class = g_type_class_ref (t);
  props = g_object_class_list_properties (class, NULL);
  if (props != NULL && props[0] != NULL) {
    for (i = 0; props[i] != NULL; ++i) {
      GParamSpec *param = props[i];
      xmlTextWriterStartElement (writer, "property");
      xmlTextWriterWriteAttribute (writer, "name", param->name);
      xmlTextWriterWriteAttribute (writer, "nick", g_param_spec_get_nick (param));
      /* TODO: transform type names to Tristate/Boolean/Integer/String/etc */
      xmlTextWriterWriteAttribute (writer, "type", g_type_name (param->value_type));
      output_range (writer, param);
      xmlTextWriterWriteCDATA (writer, g_param_spec_get_blurb (param));
      xmlTextWriterEndElement (writer);
    }
  }
  g_free (props);
  g_type_class_unref (class);
}

static void handle_objects(xmlTextWriterPtr writer, const char* name, GType t) {
  int i;
  GType *children;
  children = g_type_children (t, NULL);
  for (i = 0; children[i] != 0; ++i) {
    xmlTextWriterStartElement (writer, name);
    xmlTextWriterWriteAttribute (writer, "name", g_type_name (children[i]));
    handle_properties (writer, children[i]);
    xmlTextWriterEndElement (writer);
  }
  g_free (children);
}

int main(int argc, char **argv) {
  xmlTextWriterPtr writer;

  if (argc != 2) {
    g_print ("ERROR: doc-generator <filename.xml>\n");
    return 1;
  }

  g_type_init ();
  flurb_init ();
  writer = xmlNewTextWriterFilename (argv[1], 0);
  g_assert (writer != NULL);

  xmlTextWriterStartDocument (writer, "1.0", "UTF-8", "yes");

  xmlTextWriterStartElement (writer, "devilspie");

  xmlTextWriterStartElement (writer, "matchers");
  handle_objects (writer, "matcher", devilspie_matcher_get_type());
  xmlTextWriterEndElement (writer);

  xmlTextWriterStartElement (writer, "actions");
  handle_objects (writer, "action", devilspie_action_get_type());
  xmlTextWriterEndElement (writer);

  xmlTextWriterEndElement (writer);
  xmlTextWriterEndDocument (writer);
  
  xmlFreeTextWriter(writer);
  return 0;
}
