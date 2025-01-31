#
# Makefile - Copyright (c) 2022 - Olivier Poncet
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# ----------------------------------------------------------------------------
# global environment
# ----------------------------------------------------------------------------

CC       = gcc
CFLAGS   = -g -O2 -Wall -std=c99
CXX      = g++
CXXFLAGS = -g -O2 -Wall -std=c++14
CPPFLAGS = -I.
LD       = g++
LDFLAGS  = -L.
CP       = cp
CPFLAGS  = -f
RM       = rm
RMFLAGS  = -f

# ----------------------------------------------------------------------------
# default rules
# ----------------------------------------------------------------------------

.c.o :
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o :
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $<

# ----------------------------------------------------------------------------
# global targets
# ----------------------------------------------------------------------------

all : build

build : build_chat
	@echo "=== $@ ok ==="

clean : clean_chat
	@echo "=== $@ ok ==="

# ----------------------------------------------------------------------------
# Chat server
# ----------------------------------------------------------------------------

CHAT_PROGRAM = \
	chat.bin \
	$(NULL)

CHAT_OBJECTS = \
	chat.o \
	$(NULL)

CHAT_LIBS = \
	-lpthread -lm \
	$(NULL)

build_chat : $(CHAT_PROGRAM)

clean_chat :
	$(RM) $(RMFLAGS) $(CHAT_OBJECTS) $(CHAT_PROGRAM)

$(CHAT_PROGRAM) : $(CHAT_OBJECTS)
	$(LD) $(LDFLAGS) -o $(CHAT_PROGRAM) $(CHAT_OBJECTS) $(CHAT_LIBS)

# ----------------------------------------------------------------------------
# dependencies
# ----------------------------------------------------------------------------

chat.o : chat.cc chat.h

# ----------------------------------------------------------------------------
# End-Of-File
# ----------------------------------------------------------------------------
