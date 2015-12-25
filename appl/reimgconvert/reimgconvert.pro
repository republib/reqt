#-------------------------------------------------
#
# Project created by QtCreator 2014-06-07T23:57:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = reimgconvert
TEMPLATE = app

INCLUDEPATH = ../.. /usr/include/c++/4.9

SOURCES += main.cpp\
	 ../../base/ReException.cpp \
	 ../../base/ReQStringUtil.cpp \
	 ../../base/ReLogger.cpp \
	 ../../gui/ReStateStorage.cpp \
	 ../../gui/ReGuiValidator.cpp \
	 mainwindow.cpp \
	 converter.cpp \
	 aboutdialog.cpp

HEADERS  += mainwindow.hpp \
	 ../../base/rebase.hpp \
	 ../../base/ReQStringUtil.hpp \
	 ../../gui/ReStateStorage.hpp \
	 ../../gui/ReGuiValidator.hpp \
	 ../../gui/regui.hpp \
	 converter.hpp \
	 aboutdialog.hpp

FORMS    += mainwindow.ui \
	 aboutdialog.ui

RESOURCES += \
	 reimgconvert.qrc

TRANSLATIONS = reimgconvert_de.ts

CODECFORSRC = UTF-8

OTHER_FILES +=
