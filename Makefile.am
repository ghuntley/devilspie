SUBDIRS = po src

#
# Man page
#

man_MANS = devilspie.1

#
# The Matcher/Action reference documentation
#

noinst_DATA = devilspie-reference.html

docs.xml:
	$(top_builddir)/src/doc-generator $@
.INTERMEDIATE: docs.xml

devilspie-reference.html: docs2html.xsl docs.xml
	xsltproc -o $@ $^
CLEANFILES += docs.xml devilspie-reference.html

#
# The rest
#

EXTRA_DIST = \
	devilspie.spec.in \
	devilspie.spec \
	autogen.sh \
	$(addsuffix .in, $(INTLTOOL_BUILT)) \
	sample-config.xml \
	devilspie.dtd \
	docs2html.xsl \
	$(man_MANS)

INTLTOOL_BUILT = \
	intltool-extract \
	intltool-merge \
	intltool-update

CLEANFILES += $(INTLTOOL_BUILT)
