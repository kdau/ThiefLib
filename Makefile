#******************************************************************************
#   Makefile
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


srcdir = ./source
bindir1n = ./bin-t1
bindir1d = ./bin-t1-d
bindir2n = ./bin-t2
bindir2d = ./bin-t2-d

HOST = x86_64-linux-gnu
TARGET = i686-w64-mingw32
LGDIR = ./lg
DH2DIR = ./DH2



.PHONY: default all clean
.PRECIOUS: %.o
.INTERMEDIATE: ./Thief/Private.hh ./Thief/ParameterCache.hh
default: all

ifdef TARGET
CXX = $(TARGET)-g++
ifneq (,$(findstring linux-gnu,$(HOST)))
AR = $(TARGET)-ar
else # !linux-gnu
AR = $(TARGET)-gcc-ar
endif # linux-gnu
else # !TARGET
CXX = g++
AR = ar
endif # TARGET

CXXFLAGS = -Wall -Wextra -Woverloaded-virtual -Wsign-promo -Wshadow -masm=intel -std=gnu++0x
CXXFLAGSN = -O2
CXXFLAGSD = -g -O0

DEFINES = -D_NEWDARK
DEFINESN = -DNDEBUG
DEFINESD = -DDEBUG
DEFINES1 = -D_DARKGAME=1
DEFINES2 = -D_DARKGAME=2
INCLUDES = -I. -I$(LGDIR) -I$(DH2DIR)

ARFLAGS = rc
LIBS = $(DH2DIR)/dh2lib.o $(LGDIR)/iids.o
LIBSN = $(LGDIR)/lg.o $(LGDIR)/scrmsgs.o $(LGDIR)/refcnt.o
LIBSD = $(LGDIR)/lg-d.o $(LGDIR)/scrmsgs-d.o $(LGDIR)/refcnt-d.o



$(bindir1n):
	mkdir -p $@

$(bindir1d):
	mkdir -p $@

$(bindir2n):
	mkdir -p $@

$(bindir2d):
	mkdir -p $@



SOURCES = \
	Base.cc \
	Private.cc \
	AI.cc \
	Combat.cc \
	Engine.cc \
	HUD.cc \
	Link.cc \
	Message.cc \
	Mission.cc \
	Object.cc \
	ParameterCache.cc \
	Parameter.cc \
	Physics.cc \
	Player.cc \
	Property.cc \
	QuestVar.cc \
	Rendering.cc \
	Script.cc \
	Security.cc \
	Sound.cc \
	Types.cc

OBJECTS1N = $(SOURCES:%.cc=$(bindir1n)/%.o)
OBJECTS1D = $(SOURCES:%.cc=$(bindir1d)/%.o)
OBJECTS2N = $(SOURCES:%.cc=$(bindir2n)/%.o)
OBJECTS2D = $(SOURCES:%.cc=$(bindir2d)/%.o)

$(bindir1n)/%.o: $(srcdir)/%.cc ./Thief/%.hh $(srcdir)/Private.hh
	$(CXX) $(CXXFLAGS) $(CXXFLAGSN) $(DEFINES) $(DEFINESN) $(DEFINES1) $(INCLUDES) -o $@ -c $<

$(bindir1d)/%.o: $(srcdir)/%.cc ./Thief/%.hh $(srcdir)/Private.hh
	$(CXX) $(CXXFLAGS) $(CXXFLAGSD) $(DEFINES) $(DEFINESD) $(DEFINES1) $(INCLUDES) -o $@ -c $<

$(bindir2n)/%.o: $(srcdir)/%.cc ./Thief/%.hh $(srcdir)/Private.hh
	$(CXX) $(CXXFLAGS) $(CXXFLAGSN) $(DEFINES) $(DEFINESN) $(DEFINES2) $(INCLUDES) -o $@ -c $<

$(bindir2d)/%.o: $(srcdir)/%.cc ./Thief/%.hh $(srcdir)/Private.hh
	$(CXX) $(CXXFLAGS) $(CXXFLAGSD) $(DEFINES) $(DEFINESD) $(DEFINES2) $(INCLUDES) -o $@ -c $<

# don't exist; in ./source/ instead
./Thief/Private.hh:
./Thief/ParameterCache.hh:

$(bindir1n)/Base.o: ./Thief/Base.inl
$(bindir1d)/Base.o: ./Thief/Base.inl
$(bindir2n)/Base.o: ./Thief/Base.inl
$(bindir2d)/Base.o: ./Thief/Base.inl

$(bindir1n)/AI.o: ./Thief/AI.inl
$(bindir1d)/AI.o: ./Thief/AI.inl
$(bindir2n)/AI.o: ./Thief/AI.inl
$(bindir2d)/AI.o: ./Thief/AI.inl

$(bindir1n)/Link.o: ./Thief/Link.inl
$(bindir1d)/Link.o: ./Thief/Link.inl
$(bindir2n)/Link.o: ./Thief/Link.inl
$(bindir2d)/Link.o: ./Thief/Link.inl

$(bindir1n)/ParameterCache.o: $(srcdir)/ParameterCache.hh
$(bindir1d)/ParameterCache.o: $(srcdir)/ParameterCache.hh
$(bindir2n)/ParameterCache.o: $(srcdir)/ParameterCache.hh
$(bindir2d)/ParameterCache.o: $(srcdir)/ParameterCache.hh

$(bindir1n)/Parameter.o: ./Thief/Parameter.inl $(srcdir)/ParameterCache.hh
$(bindir1d)/Parameter.o: ./Thief/Parameter.inl $(srcdir)/ParameterCache.hh
$(bindir2n)/Parameter.o: ./Thief/Parameter.inl $(srcdir)/ParameterCache.hh
$(bindir2d)/Parameter.o: ./Thief/Parameter.inl $(srcdir)/ParameterCache.hh

$(bindir1n)/Property.o: ./Thief/Property.inl
$(bindir1d)/Property.o: ./Thief/Property.inl
$(bindir2n)/Property.o: ./Thief/Property.inl
$(bindir2d)/Property.o: ./Thief/Property.inl

$(bindir1n)/Script.o: ./Thief/Script.inl
$(bindir1d)/Script.o: ./Thief/Script.inl
$(bindir2n)/Script.o: ./Thief/Script.inl
$(bindir2d)/Script.o: ./Thief/Script.inl



LIBRARY1N = libthief1.a
$(LIBRARY1N): $(OBJECTS1N) $(LIBS) $(LIBSN)
LIBRARY1D = libthief1-d.a
$(LIBRARY1D): $(OBJECTS1D) $(LIBS) $(LIBSD)
LIBRARY2N = libthief2.a
$(LIBRARY2N): $(OBJECTS2N) $(LIBS) $(LIBSN)
LIBRARY2D = libthief2-d.a
$(LIBRARY2D): $(OBJECTS2D) $(LIBS) $(LIBSD)

$(LGDIR)/%.o: ;
$(DH2DIR)/%.o: ;
%.a:
	$(AR) $(ARFLAGS) $@ $?



all: $(bindir1n) $(LIBRARY1N) $(bindir1d) $(LIBRARY1D) $(bindir2n) $(LIBRARY2N) $(bindir2d) $(LIBRARY2D)

clean:
	$(RM) $(bindir1n)/*.o $(bindir1d)/*.o $(bindir2n)/*.o $(bindir2d)/*.o *.a

