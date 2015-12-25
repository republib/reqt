QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = reidoscl
CONFIG += console
CONFIG -= app_bundle

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
	reidoscl.cpp

HEADERS  +=  ../../base/rebase.hpp \
	 ../../gui/regui.hpp \
	 ../../gui/ReEdit.hpp \
	 ../../gui/ReStateStorage.hpp \
	 ../../gui/ReSettings.hpp \
	../../base/ReStringUtils.hpp \
	../../base/ReQStringUtils.hpp \
	../../base/ReException.hpp \
	../../os/reos.hpp
	reidoscl.hpp
