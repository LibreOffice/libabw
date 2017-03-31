noinst_PROGRAMS = abwfuzzer

AM_CXXFLAGS = -I$(top_srcdir)/inc \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

abwfuzzer_LDADD = \
	$(top_builddir)/src/lib/libabw-@ABW_MAJOR_VERSION@.@ABW_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	-lFuzzingEngine

abwfuzzer_SOURCES = \
	abwfuzzer.cpp
