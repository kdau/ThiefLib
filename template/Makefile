# Makefile
#
# Part of the template for ThiefLib script modules
# Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>

# CUSTOMIZE to the short name of your module.
MODULE_NAME = SampleModule

# CUSTOMIZE to the platform you are building from (Windows is i686-w64-mingw32).
HOST = x86_64-linux-gnu

# This should not change; it is the Dark Engine's platform.
TARGET = i686-w64-mingw32

# CUSTOMIZE to the location of the ThiefLib headers and archives.
THIEFLIBDIR = ../

# CUSTOMIZE to include only the game(s) you want to build for.
default: thief1 thief2

# CUSTOMIZE to list your scripts' headers.
SCRIPT_HEADERS = \
	SampleScript.hh

include $(THIEFLIBDIR)/module.mk

