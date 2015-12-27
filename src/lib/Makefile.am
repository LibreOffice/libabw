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

AM_CXXFLAGS = -I$(top_srcdir)/inc \
	$(REVENGE_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(ZLIB_CFLAGS) \
	$(DEBUG_CXXFLAGS) \
	-DLIBABW_BUILD=1 \
	-DBOOST_ERROR_CODE_HEADER_ONLY \
	-DBOOST_SYSTEM_NO_DEPRECATED

BUILT_SOURCES = tokens.h tokenhash.h

libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_LIBADD  = $(REVENGE_LIBS) $(LIBXML_LIBS) $(ZLIB_LIBS) @LIBABW_WIN32_RESOURCE@
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_DEPENDENCIES = @LIBABW_WIN32_RESOURCE@
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic $(no_undefined)
libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_SOURCES = \
	ABWCollector.cpp \
	ABWContentCollector.cpp \
	ABWOutputElements.cpp \
	ABWParser.cpp \
	ABWStylesCollector.cpp \
	ABWXMLHelper.cpp \
	ABWXMLTokenMap.cpp \
	ABWZlibStream.cpp \
	AbiDocument.cpp \
	libabw_internal.cpp \
	\
	ABWCollector.h \
	ABWContentCollector.h \
	ABWOutputElements.h \
	ABWParser.h \
	ABWStylesCollector.h \
	ABWXMLHelper.h \
	ABWXMLTokenMap.h \
	ABWZlibStream.h \
	libabw_internal.h

tokens.h : tokens.gperf

tokenhash.h : tokens.gperf
	$(GPERF) --compare-strncmp -C -m 20 tokens.gperf \
		| $(SED) -e 's/(char\*)0/(char\*)0, 0/g' -e 's/register //g' > tokenhash.h

tokens.gperf : $(top_srcdir)/src/lib/tokens.txt $(top_srcdir)/src/lib/gentoken.pl
	$(PERL) $(top_srcdir)/src/lib/gentoken.pl $(top_srcdir)/src/lib/tokens.txt \
		tokens.h tokens.gperf

if OS_WIN32

@LIBABW_WIN32_RESOURCE@ : libabw.rc $(libabw_@ABW_MAJOR_VERSION@_@ABW_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libabw.rc @LIBABW_WIN32_RESOURCE@
endif

MOSTLYCLEANFILES = \
	$(BUILT_SOURCES) \
	tokens.gperf

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	tokens.txt \
	gentoken.pl \
	libabw.rc \
	libabw.rc.in
