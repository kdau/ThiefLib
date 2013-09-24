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
#   the Free Software Foundation; either version 3 of the License, or
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
bindir = ./bin
LGDIR = ./lg

HOST = x86_64-linux-gnu
TARGET = i686-w64-mingw32

bindir1n = $(bindir)/t1
bindir1d = $(bindir)/t1-d
bindir2n = $(bindir)/t2
bindir2d = $(bindir)/t2-d
bindir_osl = $(bindir)/osl

LIBRARY1N = libthief1.a
LIBRARY1D = libthief1-d.a
LIBRARY2N = libthief2.a
LIBRARY2D = libthief2-d.a
OSL_NAME = ThiefLib.osl



.PHONY: default dirs doc clean-doc all clean
.PRECIOUS: %.o
.INTERMEDIATE: ./Thief/Private.hh ./Thief/ParameterCache.hh ./Thief/OSL.hh
default: all

ifdef TARGET
CXX = $(TARGET)-g++
LD = $(TARGET)-g++
ifneq (,$(findstring linux-gnu,$(HOST)))
AR = $(TARGET)-ar
RC = $(TARGET)-windres
DLLTOOL = $(TARGET)-dlltool
else # !linux-gnu
AR = $(TARGET)-gcc-ar
RC = windres
DLLTOOL = dlltool
endif # linux-gnu
else # !TARGET
CXX = g++
LD = g++
AR = ar
RC = windres
DLLTOOL = dlltool
endif # TARGET

CXXFLAGS = -Wall -Wextra -Woverloaded-virtual -Wsign-promo -Wshadow -std=gnu++0x
CXXFLAGSN = -O2
CXXFLAGSD = -g -O0

DEFINES = -D_NEWDARK -DOSL_NAME=\"$(OSL_NAME)\"
DEFINESN = -DNDEBUG
DEFINESD = -DDEBUG
DEFINES1 = -D_DARKGAME=1
DEFINES2 = -D_DARKGAME=2

INCLUDES = -I. -I$(LGDIR)

ARFLAGS = rc
LDFLAGS = -mwindows -mdll -static-libgcc -static-libstdc++

LIBSN = $(LGDIR)/iids.o $(LGDIR)/lg.o $(LGDIR)/scrmsgs.o $(LGDIR)/refcnt.o
LIBSD = $(LGDIR)/iids.o $(LGDIR)/lg-d.o $(LGDIR)/scrmsgs-d.o $(LGDIR)/refcnt-d.o



dirs: $(bindir1n) $(bindir1d) $(bindir2n) $(bindir2d) $(bindir_osl)

$(bindir1n):
	mkdir -p $@

$(bindir1d):
	mkdir -p $@

$(bindir2n):
	mkdir -p $@

$(bindir2d):
	mkdir -p $@

$(bindir_osl):
	mkdir -p $@



COMMON_SOURCES = \
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
	Parameter.cc \
	Physics.cc \
	Player.cc \
	Property.cc \
	QuestVar.cc \
	Rendering.cc \
	Security.cc \
	Sound.cc \
	Tweq.cc \
	Types.cc



SOURCES = $(COMMON_SOURCES) \
	Script.cc

HEADERS = Thief/Thief.hh $(SOURCES:%.cc=Thief/%.hh)

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

# doesn't exist; in ./source/ instead
./Thief/Private.hh:

$(bindir1n)/Base.o: ./Thief/Base.inl
$(bindir1d)/Base.o: ./Thief/Base.inl
$(bindir2n)/Base.o: ./Thief/Base.inl
$(bindir2d)/Base.o: ./Thief/Base.inl
$(bindir_osl)/Base.o: ./Thief/Base.inl

$(bindir1n)/AI.o: ./Thief/AI.inl
$(bindir1d)/AI.o: ./Thief/AI.inl
$(bindir2n)/AI.o: ./Thief/AI.inl
$(bindir2d)/AI.o: ./Thief/AI.inl
$(bindir_osl)/AI.o: ./Thief/AI.inl

$(bindir1n)/HUD.o: ./Thief/HUD.inl
$(bindir1d)/HUD.o: ./Thief/HUD.inl
$(bindir2n)/HUD.o: ./Thief/HUD.inl
$(bindir2d)/HUD.o: ./Thief/HUD.inl
$(bindir_osl)/HUD.o: ./Thief/HUD.inl

$(bindir1n)/Link.o: ./Thief/Link.inl
$(bindir1d)/Link.o: ./Thief/Link.inl
$(bindir2n)/Link.o: ./Thief/Link.inl
$(bindir2d)/Link.o: ./Thief/Link.inl
$(bindir_osl)/Link.o: ./Thief/Link.inl

$(bindir1n)/Parameter.o: ./Thief/Parameter.inl
$(bindir1d)/Parameter.o: ./Thief/Parameter.inl
$(bindir2n)/Parameter.o: ./Thief/Parameter.inl
$(bindir2d)/Parameter.o: ./Thief/Parameter.inl
$(bindir_osl)/Parameter.o: ./Thief/Parameter.inl

$(bindir1n)/Property.o: ./Thief/Property.inl
$(bindir1d)/Property.o: ./Thief/Property.inl
$(bindir2n)/Property.o: ./Thief/Property.inl
$(bindir2d)/Property.o: ./Thief/Property.inl
$(bindir_osl)/Property.o: ./Thief/Property.inl

$(bindir1n)/QuestVar.o: ./Thief/QuestVar.inl
$(bindir1d)/QuestVar.o: ./Thief/QuestVar.inl
$(bindir2n)/QuestVar.o: ./Thief/QuestVar.inl
$(bindir2d)/QuestVar.o: ./Thief/QuestVar.inl

$(bindir1n)/Script.o: ./Thief/Script.inl
$(bindir1d)/Script.o: ./Thief/Script.inl
$(bindir2n)/Script.o: ./Thief/Script.inl
$(bindir2d)/Script.o: ./Thief/Script.inl



$(LIBRARY1N): $(OBJECTS1N) $(LIBSN)
$(LIBRARY1D): $(OBJECTS1D) $(LIBSD)
$(LIBRARY2N): $(OBJECTS2N) $(LIBSN)
$(LIBRARY2D): $(OBJECTS2D) $(LIBSD)

$(LGDIR)/%.o: ;
%.a:
	$(AR) $(ARFLAGS) $@ $?



OSL_SOURCES = $(COMMON_SOURCES) \
	ParameterCache.cc \
	OSL.cc

OSL_OBJECTS = \
	$(OSL_SOURCES:%.cc=$(bindir_osl)/%.o) \
	$(bindir_osl)/OSL_version.o \
	$(bindir_osl)/OSL_exports.o

OSL_DEFINES = -DIS_OSL

$(bindir_osl)/%.o: $(srcdir)/%.cc ./Thief/%.hh $(srcdir)/Private.hh
	$(CXX) $(CXXFLAGS) $(CXXFLAGSN) $(DEFINES) $(OSL_DEFINES) $(INCLUDES) -o $@ -c $<

./Thief/ParameterCache.hh:
$(bindir_osl)/ParameterCache.o: $(srcdir)/ParameterCache.hh

./Thief/OSL.hh:
$(bindir_osl)/OSL.o: $(srcdir)/OSL.hh

$(bindir_osl)/OSL_version.o: $(srcdir)/OSL.rc
	$(RC) $(DEFINES) $(OSL_DEFINES) $(INCLUDES) -o $@ -i $<

$(bindir_osl)/OSL_exports.o: $(srcdir)/OSL.def
	$(DLLTOOL) --output-exp $@ --input-def $<

$(OSL_NAME): $(OSL_OBJECTS) $(LIBSN)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x14200000 -o $@ $^ -luuid



doc/index.html: Doxyfile $(HEADERS)
	doxygen Doxyfile

doc: doc/index.html

clean-doc:
	$(RM) -r doc/search
	$(RM) doc/*



all: dirs $(LIBRARY1N) $(LIBRARY1D) $(LIBRARY2N) $(LIBRARY2D) $(OSL_NAME) doc

clean: clean-doc
	$(RM) $(bindir1n)/*.o $(bindir1d)/*.o $(bindir2n)/*.o $(bindir2d)/*.o *.a $(bindir_osl)/*.o $(OSL_NAME)

