SUBDIRS = po src

man_MANS = devilspie.1

EXTRA_DIST = \
	autogen.sh \
	devilspie.spec.in devilspie.spec \
	$(addsuffix .in, $(INTLTOOL_BUILT)) \
	$(man_MANS)

INTLTOOL_BUILT = \
	intltool-extract \
	intltool-merge \
	intltool-update
CLEANFILES = $(INTLTOOL_BUILT)
