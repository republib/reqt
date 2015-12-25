QT       += core network gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = reprime

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH = ../..

SOURCES += main.cpp \
	../../base/ReLogger.cpp \
	../../base/ReRandomizer.cpp \
	../../base/ReStringUtils.cpp \
	../../base/ReException.cpp \
	Prime.cpp


