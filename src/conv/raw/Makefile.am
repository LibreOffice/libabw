if BUILD_TOOLS

bin_PROGRAMS = abw2raw

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(WPD_CFLAGS) \
	$(WPD_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

abw2raw_DEPENDENCIES = @ABW2RAW_WIN32_RESOURCE@

abw2raw_LDADD = \
	../../lib/libabw-@ABW_MAJOR_VERSION@.@ABW_MINOR_VERSION@.la \
	$(WPD_LIBS) \
	$(WPD_STREAM_LIBS) \
	@ABW2RAW_WIN32_RESOURCE@

abw2raw_SOURCES = \
	RawDocumentGenerator.cpp \
	RawDocumentGenerator.h \
	abw2raw.cpp

if OS_WIN32

@ABW2RAW_WIN32_RESOURCE@ : abw2raw.rc $(abw2raw_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource && \
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource abw2raw.rc @ABW2RAW_WIN32_RESOURCE@
endif

# Include the abw2raw_SOURCES in case we build a tarball without stream
EXTRA_DIST = \
	$(abw2raw_SOURCES)	\
	abw2raw.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	abw2raw.rc	 
 
endif
