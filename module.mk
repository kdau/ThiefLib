#******************************************************************************
#   module.mk: includable elements for script module Makfiles
#
#   This file is part of ThiefLib, a library for Thief 1/2 script modules.
#   Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
#   Adapted in part from Public Scripts and the Object Script Library
#   Copyright (C) 2005-2013 Tom N Harris <telliamed@whoopdedo.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#******************************************************************************

bindir1 = ./bin-t1
bindir2 = ./bin-t2

.PHONY: default all clean
.PRECIOUS: %.o
.INTERMEDIATE: $(bindir1)/exports.o $(bindir2)/exports.o module.hh
default: all

ifdef TARGET
CXX = $(TARGET)-g++
LD = $(TARGET)-g++
ifneq (,$(findstring linux-gnu,$(HOST)))
DLLTOOL = $(TARGET)-dlltool
RC = $(TARGET)-windres
else # !linux-gnu
DLLTOOL = dlltool
RC = windres
endif # linux-gnu
else # !TARGET
CXX = g++
LD = g++
DLLTOOL = dlltool
RC = windres
endif # TARGET

DEFINES = -D_NEWDARK
DEFINES1 = -D_DARKGAME=1
DEFINES2 = -D_DARKGAME=2

INCLUDES = -I. -I$(THIEFLIBDIR)
LIBDIRS = -L. -L$(THIEFLIBDIR)

CXXFLAGS = -Wall -Wextra -Woverloaded-virtual -Wsign-promo -Wshadow -masm=intel -std=gnu++0x
LDFLAGS = -mwindows -mdll -Wl,--enable-auto-image-base -static-libgcc -static-libstdc++

LIBS = -luuid

ifdef DEBUG
DEFINES := $(DEFINES) -DDEBUG
CXXFLAGS := $(CXXFLAGS) -g -O0
LDFLAGS := $(LDFLAGS) -g
LIBS1 = -lthief1-d
LIBS2 = -lthief2-d
else
DEFINES := $(DEFINES) -DNDEBUG
CXXFLAGS := $(CXXFLAGS) -O2
LDFLAGS := $(LDFLAGS) -Wl,--strip-all
LIBS1 = -lthief1
LIBS2 = -lthief2
endif



SCRIPT_OBJS1 = $(SCRIPT_HEADERS:%.hh=$(bindir1)/%.o)
SCRIPT_OBJS2 = $(SCRIPT_HEADERS:%.hh=$(bindir2)/%.o)



MODULE_OBJS1 = \
	$(bindir1)/module.o \
	$(bindir1)/version.o \
	$(bindir1)/exports.o
MODULE_OBJS2 = \
	$(bindir2)/module.o \
	$(bindir2)/version.o \
	$(bindir2)/exports.o

module.hh:
$(bindir1)/module.o: $(SCRIPT_HEADERS) version.rc
$(bindir2)/module.o: $(SCRIPT_HEADERS) version.rc

$(bindir1)/version.o: version.rc
	$(RC) $(DEFINES) $(DEFINES1) $(INCLUDES) -o $@ -i $<
$(bindir2)/version.o: version.rc
	$(RC) $(DEFINES) $(DEFINES2) $(INCLUDES) -o $@ -i $<

$(bindir1)/exports.o: exports.def
	$(DLLTOOL) --output-exp $@ --input-def $<
$(bindir2)/exports.o: exports.def
	$(DLLTOOL) --output-exp $@ --input-def $<



$(bindir1):
	mkdir -p $@

$(bindir2):
	mkdir -p $@

$(bindir1)/%.o: %.cc %.hh $(SCRIPT_INCLUDES)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(DEFINES1) $(INCLUDES) -o $@ -c $<

$(bindir2)/%.o: %.cc %.hh $(SCRIPT_INCLUDES)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(DEFINES2) $(INCLUDES) -o $@ -c $<

$(bindir1)/$(MODULE_NAME).osm: $(SCRIPT_OBJS1) $(MODULE_OBJS1)
	$(LD) $(LDFLAGS) $(LIBDIRS) -o $@ $^ $(LIBS1) $(LIBS)

$(bindir2)/$(MODULE_NAME).osm: $(SCRIPT_OBJS2) $(MODULE_OBJS2)
	$(LD) $(LDFLAGS) $(LIBDIRS) -o $@ $^ $(LIBS2) $(LIBS)



all: $(bindir1) $(bindir2) $(bindir1)/$(MODULE_NAME).osm $(bindir2)/$(MODULE_NAME).osm

clean:
	$(RM) $(bindir1)/*.o $(bindir2)/*.o $(bindir1)/*.osm $(bindir2)/*.osm

