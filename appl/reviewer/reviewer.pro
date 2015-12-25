#-------------------------------------------------
#
# Project created by QtCreator 2015-04-02T23:48:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = refind
TEMPLATE = app

INCLUDEPATH = ../.. /usr/include/c++/4.9

SOURCES += main.cpp\
		  mainwindow.cpp \
	 ../../base/ReException.cpp \
	 ../../base/ReQStringUtil.cpp \
	 ../../base/ReLogger.cpp \
	 aboutdialog.cpp \
	 ../../gui/ReStateStorage.cpp \
	 ../../gui/ReGuiValidator.cpp \
	 ../../base/ReBigFile.cpp

HEADERS  += mainwindow.hpp \
	 ../../base/rebase.hpp \
	 ../../base/ReQStringUtil.hpp \
	 ../../gui/ReStateStorage.hpp \
	 aboutdialog.hpp \
	 ../../gui/ReGuiValidator.hpp \
	 ../../gui/regui.hpp \
	 ../../base/ReBigFile.hpp


FORMS    += mainwindow.ui \
	 aboutdialog.ui

TRANSLATIONS = reviewer_de.ts

CODECFORSRC = UTF-8

RESOURCES += \
	 reviewer.qrc
