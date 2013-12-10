if PLATFORM_WIN32
no_undefined = -no-undefined
else
if PLATFORM_DARWIN
no_undefined = -no-undefined
else
no_undefined =
endif
endif

if OS_WIN32
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -avoid-version
else
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

lib_LTLIBRARIES = libabw-@ABW_MAJOR_VERSION@.@ABW_MINOR_VERSION@.la $(target_libabw_stream)
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_includedir = $(includedir)/libabw-@ABW_MAJOR_VERSION@.@ABW_MINOR_VERSION@/libabw
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_include_HEADERS = \
	$(top_srcdir)/inc/libabw/libabw.h \
	$(top_srcdir)/inc/libabw/AbiDocument.h

AM_CXXFLAGS = -I$(top_srcdir)/inc $(REVENGE_CFLAGS) $(LIBXML2_CFLAGS) $(DEBUG_CXXFLAGS)

generated_files = \
	$(top_builddir)/src/lib/tokens.h \
	$(top_builddir)/src/lib/tokenhash.h

libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_LIBADD  = $(REVENGE_LIBS) $(LIBXML2_LIBS) @LIBABW_WIN32_RESOURCE@
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_DEPENDENCIES = @LIBABW_WIN32_RESOURCE@
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic $(no_undefined)
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_SOURCES = \
	ABWContentCollector.cpp \
	ABWOutputElements.cpp \
	ABWParser.cpp \
	ABWStylesCollector.cpp \
	ABWXMLHelper.cpp \
	ABWXMLTokenMap.cpp \
	AbiDocument.cpp \
	\
	ABWCollector.h \
	ABWContentCollector.h \
	ABWOutputElements.h \
	ABWParser.h \
	ABWStylesCollector.h \
	ABWXMLHelper.h \
	ABWXMLTokenMap.h \
	libabw_internal.h \
	\
	$(generated_files)

ABWXMLTokenMap.lo : $(generated_files)
ABWParser.lo : $(generated_files)

$(top_builddir)/src/lib/tokens.h : $(top_builddir)/src/lib/tokens.gperf

$(top_builddir)/src/lib/tokenhash.h : $(top_builddir)/src/lib/tokens.gperf
	$(GPERF) --compare-strncmp -C -m 20 $(top_builddir)/src/lib/tokens.gperf \
		| $(SED) -e "s/(char\*)0/(char\*)0, 0/g" > $(top_builddir)/src/lib/tokenhash.h

$(top_builddir)/src/lib/tokens.gperf : $(top_srcdir)/src/lib/tokens.txt gentoken.pl
	perl $(top_srcdir)/src/lib/gentoken.pl $(top_srcdir)/src/lib/tokens.txt \
		$(top_builddir)/src/lib/tokens.h $(top_builddir)/src/lib/tokens.gperf

if OS_WIN32

@LIBABW_WIN32_RESOURCE@ : libabw.rc $(libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libabw.rc @LIBABW_WIN32_RESOURCE@
endif

distclean-local :
	rm -f $(generated_files) $(top_builddir)/src/lib/*.gperf

EXTRA_DIST = \
	tokens.txt \
    gentoken.pl \
	libabw.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	libabw.rc
