SUBDIRS = po src

man_MANS = devilspie.1

EXTRA_DIST = \
	debian \
	devilspie.spec.in \
	devilspie.spec \
	autogen.sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	sample-config.xml \
	devilspie.dtd \
	devilspie.glade \
	devilspie.gladep \
	$(man_MANS)
#	$(schema_DATA) \
#	$(schema_in_files)

INTLTOOL_BUILT = \
	intltool-extract \
	intltool-merge \
	intltool-update

CLEANFILES = $(INTLTOOL_BUILT)
