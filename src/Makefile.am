# We only install devilspie at the moment
bin_PROGRAMS = devilspie

# Build but don't install the doc generator
noinst_PROGRAMS = doc-generator

# Build the parser test if we are running tests
check_PROGRAMS = parser-test
TESTS = parser-test

devilspie_SOURCES = \
	${BUILT_SOURCES} \
	config-parser.c \
	flurb.h \
	flurb.c \
	devilspie.h \
	devilspie.c \
	xutils.h \
	xutils.c

doc_generator_SOURCES = \
	${BUILT_SOURCES} \
	flurb.h \
	flurb.c \
	xutils.h \
	xutils.c \
	doc-generator.c

parser_test_SOURCES = \
	${BUILT_SOURCES} \
	config-parser.c \
	flurb.h \
	flurb.c \
	xutils.h \
	xutils.c \
	devilspie.h \
	parser-test.c

INCLUDES = \
	$(WNCK_CFLAGS) \
	-DDEVILSPIE_LOCALEDIR=\""$(datadir)/locale"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

AM_CFLAGS = -Wall

LDADD = ${WNCK_LIBS}

# The long list of sources which are generated from .gob files.
BUILT_SOURCES = \
	tristate-dummy.c \
	tristate-dummy.h \
	devilspie-matcher.c \
	devilspie-matcher.h \
	devilspie-matcher-windowname.c \
	devilspie-matcher-windowname.h \
	devilspie-matcher-windowname-private.h \
	devilspie-matcher-always.c \
	devilspie-matcher-always.h \
	devilspie-action.c \
	devilspie-action.h \
	devilspie-action-debug.c \
	devilspie-action-debug.h \
	devilspie-action-decorate.c \
	devilspie-action-decorate.h \
	devilspie-action-exec.c \
	devilspie-action-exec.h \
	devilspie-action-hide.c \
	devilspie-action-hide.h \
	devilspie-action-layer.c \
	devilspie-action-layer.h \
	devilspie-action-opacity.c \
	devilspie-action-opacity.h \
	devilspie-action-resize.c \
	devilspie-action-resize.h \
	devilspie-action-savegeometry.c \
	devilspie-action-savegeometry.h \
	devilspie-action-setgeometry.c \
	devilspie-action-setgeometry.h \
	devilspie-action-setwintype.c \
	devilspie-action-setwintype.h \
	devilspie-action-setworkspace.c \
	devilspie-action-setworkspace.h

# Leaving this list in as a reminder that some day most of this file
# should involve $(addprefix) functions, if only automake suported
# them.
devilspie_GOBS = \
	tristate-dummy \
	devilspie-matcher \
	devilspie-matcher-windowname \
	devilspie-matcher-always \
	devilspie-action \
	devilspie-action-debug \
	devilspie-action-decorate \
	devilspie-action-exec \
	devilspie-action-hide \
	devilspie-action-layer \
	devilspie-action-opacity \
	devilspie-action-resize \
	devilspie-action-savegeometry \
	devilspie-action-setgeometry \
	devilspie-action-setwintype \
	devilspie-action-setworkspace

# Must distribute the gob files!
EXTRA_DIST = $(addsuffix .gob, ${devilspie_GOBS})

CLEANFILES = ${BUILT_SOURCES}

SUFFIXES = .gob

%.c %.h %-private.h: %.gob
	@GOB2@ $<
