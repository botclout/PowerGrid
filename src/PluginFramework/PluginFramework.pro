#------------------------------------------------------------------------
# Copyright 2014 Patrick Kramer, Vincent Wassenaar
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

#-------------------------------------------------
#
# This subproject contains the functionality
# required to extend PowerGrid with additional
# features and tasks it can perform.
#
#-------------------------------------------------

QT       -= gui

TARGET = PluginFramework
TEMPLATE = lib
CONFIG += staticlib thread c++11

SOURCES += \
    taskdescriptor.cpp \
    pgpluginloader.cpp \
    pgplugin.cpp \
    skilltrainer.cpp

HEADERS += \
    taskdescriptor.h \
    pgpluginloader.h \
    pgplugin.h \
    skilltrainer.h

# Dependency for Bridge
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Bridge/release/ -lBridge
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Bridge/debug/ -lBridge
else:unix: LIBS += -L$$OUT_PWD/../Bridge/ -lBridge

INCLUDEPATH += $$PWD/../Bridge
DEPENDPATH += $$PWD/../Bridge

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Bridge/release/libBridge.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Bridge/debug/libBridge.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Bridge/release/Bridge.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Bridge/debug/Bridge.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Bridge/libBridge.a

# Dependency for the EntityFramework
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../EntityFramework/release/ -lEntityFramework
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../EntityFramework/debug/ -lEntityFramework
else:unix: LIBS += -L$$OUT_PWD/../EntityFramework/ -lEntityFramework

INCLUDEPATH += $$PWD/../EntityFramework
DEPENDPATH += $$PWD/../EntityFramework

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../EntityFramework/release/libEntityFramework.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../EntityFramework/debug/libEntityFramework.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../EntityFramework/release/EntityFramework.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../EntityFramework/debug/EntityFramework.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../EntityFramework/libEntityFramework.a
