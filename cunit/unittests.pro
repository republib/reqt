#-------------------------------------------------
#
# Project created by QtCreator 2014-05-31T00:01:23
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = unittests
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH = ..

TEMPLATE = app

SOURCES += main.cpp \
	 ../base/ReLogger.cpp \
	 ../base/ReTest.cpp \
	 ../base/ReString.cpp \
	 ../base/ReException.cpp \
	 ../math/ReMatrix.cpp \
	 ../expr/ReSource.cpp \
	 ../expr/ReLexer.cpp \
	 ../expr/ReASTree.cpp \
	 ../expr/ReParser.cpp \
	 ../expr/ReMFParser.cpp \
	 ../base/ReQString.cpp \
	 ../base/ReFile.cpp \
	 ../expr/ReASClasses.cpp \
	 ../base/ReByteStorage.cpp \
	 ../expr/ReVM.cpp \
	 ../base/ReWriter.cpp \
	 rplmatrix_test.cpp \
	 rplexception_test.cpp \
	 rplstring_test.cpp \
	 rplsource_test.cpp \
	 rpllexer_test.cpp \
	 rplqstring_test.cpp \
	 rplastree_test.cpp \
	 rplmfparser_test.cpp \
	 rplvm_test.cpp \
	 rplbytestorage_test.cpp \
	 rplwriter_test.cpp \
	 rplbench.cpp \
	 rplcharptrmap_test.cpp \
	 ../base/rplcharptrmap.cpp

