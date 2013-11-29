#!/bin/sh

# Script to create the BUILDNUMBER used by compile-resource. This script
# needs the script createBuildNumber.pl to be in the same directory.

{ perl ./createBuildNumber.pl \
	src/lib/libabw-build.stamp \
	src/lib/libabw-stream-build.stamp \
	src/conv/html/abw2html-build.stamp \
	src/conv/raw/abw2raw-build.stamp \
	src/conv/text/abw2text-build.stamp
#Success
exit 0
}
#Unsucessful
exit 1
