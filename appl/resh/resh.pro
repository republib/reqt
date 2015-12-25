QT += core
#QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = resh
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH = ../..

SOURCES += main.cpp \
	../../base/ReStringUtils.cpp \
	../../base/ReQStringUtils.cpp \
	../../base/ReCharPtrMap.cpp \
	../../base/ReException.cpp \
	../../base/ReLogger.cpp \
	../../expr/ReSource.cpp \
	../../expr/ReLexer.cpp \
	ReShellTree.cpp
#	../../expr/ReExpression.cpp \

HEADERS += \
	resh.hpp

