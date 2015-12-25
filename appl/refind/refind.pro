#-------------------------------------------------
#
# Project created by QtCreator 2015-04-02T23:48:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = refind
TEMPLATE = app

win32:RC_ICONS += refind.ico

INCLUDEPATH = ../.. /usr/include/c++/4.9

SOURCES += main.cpp\
		  mainwindow.cpp \
	 ../../base/ReException.cpp \
	 ../../base/ReQStringUtils.cpp \
	 ../../base/ReFileUtils.cpp \
	 ../../base/ReLogger.cpp \
	 filefinder.cpp \
	 textfinder.cpp \
	 aboutdialog.cpp \
	 ../../gui/ReStateStorage.cpp \
	 ../../gui/ReGuiValidator.cpp \
	 ../../gui/ReGuiQueue.cpp \
	 dialogglobalplaceholder.cpp \
	 dialogfileplaceholder.cpp \
	 utils.cpp \
	 dialogoptions.cpp \
	 filetablewidget.cpp


HEADERS  += mainwindow.hpp \
	 ../../base/rebase.hpp \
	 filefinder.hpp \
	 textfinder.hpp \
	 ../../gui/ReStateStorage.hpp \
	 aboutdialog.hpp \
	 ../../gui/ReGuiValidator.hpp \
	 ../../gui/regui.hpp \
	 dialogglobalplaceholder.hpp \
	 dialogfileplaceholder.hpp \
	 utils.hpp \
	 dialogoptions.hpp \
	 filetablewidget.hpp \
	../../gui/ReGuiQueue.hpp


FORMS    += mainwindow.ui \
	 aboutdialog.ui \
	 dialogglobalplaceholder.ui \
	 dialogfileplaceholder.ui \
	 dialogoptions.ui

TRANSLATIONS = refind_de.ts

CODECFORSRC = UTF-8

RESOURCES += \
	 refind.qrc
