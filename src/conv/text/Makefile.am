if BUILD_TOOLS

bin_PROGRAMS = abw2text

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(REVENGE_CFLAGS) \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

abw2text_DEPENDENCIES = @ABW2TEXT_WIN32_RESOURCE@ 

abw2text_LDADD = \
	../../lib/libabw-@ABW_MAJOR_VERSION@.@ABW_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	@ABW2TEXT_WIN32_RESOURCE@
 
abw2text_SOURCES = \
	abw2text.cpp

if OS_WIN32

@ABW2TEXT_WIN32_RESOURCE@ : abw2text.rc $(abw2text_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource && \
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource abw2text.rc @ABW2TEXT_WIN32_RESOURCE@
endif

# Include the abw2text_SOURCES in case we build a tarball without stream
EXTRA_DIST = \
	$(abw2text_SOURCES) \
	abw2text.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	abw2text.rc	 
 
endif
