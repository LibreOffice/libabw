if BUILD_TOOLS

bin_PROGRAMS = abw2html

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(WPD_CFLAGS) \
	$(WPD_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

abw2html_DEPENDENCIES = @ABW2HTML_WIN32_RESOURCE@

abw2html_LDADD = \
	../../lib/libabw-@ABW_MAJOR_VERSION@.@ABW_MINOR_VERSION@.la \
	$(WPD_LIBS) \
	$(WPD_STREAM_LIBS) \
	@ABW2HTML_WIN32_RESOURCE@

abw2html_SOURCES = \
	HtmlDocumentGenerator.cpp \
	HtmlDocumentGenerator.h \
	abw2html.cpp

if OS_WIN32

@ABW2HTML_WIN32_RESOURCE@ : abw2html.rc $(abw2html_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource && \
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource abw2html.rc @ABW2HTML_WIN32_RESOURCE@
endif

# Include the abw2html_SOURCES in case we build a tarball without stream
EXTRA_DIST = \
	$(abw2html_SOURCES)	\
	abw2html.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	abw2html.rc	 

endif
