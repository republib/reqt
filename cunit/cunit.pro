#-------------------------------------------------
#
# Project created by QtCreator 2015-05-10T13:56:15
#
#-------------------------------------------------

QT       += core network gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cunit
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH = ..

SOURCES += main.cpp \
	cuReLexer.cpp \
	cuReProgArgs.cpp \
	 cuReFileSystem.cpp \
	 cuReCryptFileSystem.cpp \
	 cuReRandomizer.cpp \
	 cuReQStringUtils.cpp \
	 cuReStringUtils.cpp \
	 cuReFile.cpp \
	 cuReFileUtils.cpp \
	 cuReByteStorage.cpp \
	 cuReException.cpp \
	../expr/ReSource.cpp \
	../expr/ReLexer.cpp \
	 ../base/ReByteStorage.cpp \
	 ../base/ReCharPtrMap.cpp \
	 ../base/ReConfig.cpp \
	 ../base/ReContainer.cpp \
	 ../base/ReException.cpp \
	 ../base/ReFile.cpp \
	 ../base/ReFileUtils.cpp \
	 ../base/ReQStringUtils.cpp \
	 ../base/ReLogger.cpp \
	 ../base/ReStringUtils.cpp \
	 ../base/ReTerminator.cpp \
	 ../base/ReMatcher.cpp \
	 ../base/ReTest.cpp \
	 ../base/ReWriter.cpp \
	 ../base/ReRandomizer.cpp \
	 ../gui/ReStateStorage.cpp \
	 ../gui/ReSettings.cpp \
	../gui/ReEdit.cpp \
	../os/ReFileSystem.cpp \
	../os/ReCryptFileSystem.cpp \
	 cuReConfig.cpp \
	 cuReContainer.cpp \
	 cuReWriter.cpp \
	 cuReCharPtrMap.cpp \
	cuReEdit.cpp \
	cuReStateStorage.cpp \
	cuReSettings.cpp \
	cuReMatcher.cpp \
	 allTests.cpp \
	../base/ReProcess.cpp \
	cuReProcess.cpp \
	../base/ReProgramArgs.cpp

HEADERS += \
	 ../base/ReFile.hpp \
	 ../base/rebase.hpp \
	../gui/ReEdit.hpp \
	../math/ReMatrix.hpp \
	../os/reos.hpp \
	../math/remath.hpp \
	../base/ReProcess.hpp

