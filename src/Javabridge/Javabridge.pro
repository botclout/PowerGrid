#------------------------------------------------------------------------
# Copyright 2012-2013 Patrick Kramer, Vincent Wassenaar
#
# This file is part of PowerGrid.
#
# PowerGrid is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PowerGrid is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PowerGrid.  If not, see <http://www.gnu.org/licenses/>.
#------------------------------------------------------------------------

#------------------------------------------------
#
# .pro file for the Java bridge
#
# This module manages the connection with the
# running Java Virtual Machine and is therefore
# essential for starting Runescape. The Java bridge
# then acts as a gateway between the rest
# of PowerGrid and Runescape.
#
#------------------------------------------------

#------------------------------------------------
# The bridge only needs the QtCore, not the QtGUI.
#------------------------------------------------

QT = core

include (../cpp11enabler.pro) # enables C++11 and compiler-specific functionality

TARGET      = Javabridge
TEMPLATE    = lib
CONFIG     += static

# Temporary files are placed in the "tmp" folder
OBJECTS_DIR = tmp
MOC_DIR     = tmp

# this define is required to prevent the static VM loader
# from trying to bind to a statically linked jvm.lib
# We don't want this since we bind to the jvm library dynamically.
DEFINES    += JACE_WANT_DYNAMIC_LOAD

#------------------------------------------------
# All headers and source files to include
#------------------------------------------------

HEADERS    += javaenvironment.h
SOURCES    += javaenvironment.cpp

#depends for JACE
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../JACE/release/ -lJACE
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../JACE/debug/ -lJACE
else:unix: LIBS += -L$$OUT_PWD/../JACE/ -lJACE

INCLUDEPATH += $$PWD/../JACE
DEPENDPATH += $$PWD/../JACE
