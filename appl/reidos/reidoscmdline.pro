#-------------------------------------------------
#
# Project created by QtCreator 2015-09-14T18:00:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = reidoscmdline
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += . ../..

SOURCES += \
	 ../../gui/ReEdit.cpp \
	 ../../gui/ReStateStorage.cpp \
	 ../../gui/ReSettings.cpp \
	 ../../gui/ReFileTree.cpp \
	 ../../gui/ReGuiValidator.cpp \
	 ../../base/ReMatcher.cpp \
	 ../../base/ReFile.cpp \
	 ../../base/ReStringUtils.cpp \
	 ../../base/ReRandomizer.cpp \
	 ../../os/ReFileSystem.cpp \
	 ../../os/ReCryptFileSystem.cpp \
	../../base/ReLogger.cpp \
	../../base/ReQStringUtils.cpp \
	../../base/ReFileUtils.cpp \
	../../base/ReException.cpp \
	../../base/ReProgramArgs.cpp \
	maincmdline.cpp

HEADERS  +=  ../../base/rebase.hpp \
	 ../../gui/regui.hpp \
	 ../../gui/ReEdit.hpp \
	 ../../gui/ReStateStorage.hpp \
	 ../../gui/ReSettings.hpp \
	../../base/ReStringUtils.hpp \
	../../base/ReQStringUtils.hpp \
	../../base/ReException.hpp \
	../../os/reos.hpp
	reidoscmdline.hpp
