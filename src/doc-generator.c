#include <libxml/xmlwriter.h>
#include <glib-object.h>
#include <glib.h>

#include "flurb.h"
#include "devilspie-matcher.h"
#include "devilspie-action.h"

static void handle_properties(xmlTextWriterPtr writer, GType t) {
  GObjectClass *class;
  GParamSpec **props;
  int i;
  class = g_type_class_ref (t);
  props = g_object_class_list_properties (class, NULL);
  if (props != NULL && props[0] != NULL) {
    for (i = 0; props[i] != NULL; ++i) {
      xmlTextWriterStartElement (writer, "property");
      xmlTextWriterWriteAttribute (writer, "name", props[i]->name);
      xmlTextWriterWriteAttribute (writer, "nick", g_param_spec_get_nick (props[i]));
      /* TODO: transform type names to Tristate/Boolean/Integer/String/etc */
      xmlTextWriterWriteAttribute (writer, "type", g_type_name (props[i]->value_type));
      xmlTextWriterWriteCDATA (writer, g_param_spec_get_blurb (props[i]));
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
