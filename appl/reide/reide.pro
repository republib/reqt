#-------------------------------------------------
#
# Project created by QtCreator 2015-06-03T20:56:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ReEditor
TEMPLATE = app
PRECOMPILED_HEADER = base/rebase.hpp
INCLUDEPATH += ../..

SOURCES += \
	 ../../gui/ReEdit.cpp \
	 ../../gui/ReStateStorage.cpp \
	 ../../gui/ReSettings.cpp \
	 ../../base/ReFile.cpp \
	 ../../gui/ReFileTree.cpp \
	mainwindow.cpp \
	../../base/ReLogger.cpp \
	../../base/ReQStringUtils.cpp \
	../../base/ReFileUtils.cpp \
	../../base/ReException.cpp \
	../../base/ReDiff.cpp \
	../../os/ReFileSystem.cpp \
	projectselection.cpp \
	workspace.cpp \
	project.cpp \
	filesearch.cpp \
	views/View.cpp \
	views/FileTreeView.cpp \
	views/EditorView.cpp \
	views/Perspective.cpp \
	views/ProjectPerspective.cpp \
	views/StartPerspective.cpp \
	views/StartView.cpp \
	main.cpp


HEADERS  += mainwindow.hpp \
	 ../../base/rebase.hpp \
	 ../../gui/regui.hpp \
	 ../../gui/ReEdit.hpp \
	 ../../gui/ReStateStorage.hpp \
	 ../../gui/ReSettings.hpp \
	../../base/ReStringUtils.hpp \
	../../base/ReQStringUtils.hpp \
	../../base/ReException.hpp \
	projectselection.hpp \
	workspace.hpp \
	project.hpp \
	reide.hpp \
	filesearch.hpp \
	views/StartView.hpp

FORMS    += mainwindow.ui \
	projectselection.ui \
	filesearch.ui \
	startwidget.ui \
	views/startview.ui

RESOURCES += \
	 editor.qrc
