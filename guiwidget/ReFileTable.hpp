/*
 * ReFileTable.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REFILETABLE_HPP
#define REFILETABLE_HPP
#ifndef REGUIWIDGET_HPP
// needed for moc_ReFileTable.cpp
#include "guiwidget/reguiwidget.hpp"
#endif
class ReFileTable : public QWidget, public ReGuiValidator {
public:
   enum { TYPE, MODIFIED, SIZE, NAME, COL_COUNT };
   Q_OBJECT
public:
   explicit ReFileTable(QWidget* parent = 0);
   ~ReFileTable();
signals:

public slots:

public:
   void fillTable();
   void changePatterns(const QString& patterns);
   bool changeDirectory(QString directory);
public:
   virtual void dragEnterEvent(QDragEnterEvent* event);
   virtual void dropEvent(QDropEvent* event);
   virtual void fileDragging();
   virtual void keyPressEvent(QKeyEvent* event);
public slots:
   void tableDoubleClicked(int row, int column);
   void pushButtonUpClicked();
   void pushButtonRootClicked();
protected:
   QString buildAbsPath(int row, bool withNode = false, bool uriFormat = false);
   QString cellAsText(int row, int col);
   void copyFromClipboard(int currentRow);
   void copyToClipboard(int currentRow = -1,
                        const QString& full = ReQStringUtils::m_empty);
   void openEntry(int row);
protected:
   QVBoxLayout* mainLayout;
   QHBoxLayout* horizontalLayout;
   QString m_dateFormat;
   virtual bool say(ReLoggerLevel level, const QString& message);
public:
   QComboBox* comboBoxPath;
   QComboBox* comboBoxPatterns;
   QPushButton* pushButtonDevice;
   QPushButton* pushButtonUp;
   QPushButton* pushButtonRoot;
   QTableWidget* tableWidget;
   ReFileSystem* fileSystem;
   ReIncludeExcludeMatcher matcher;
   ReAnnouncer* announcer;
};

#endif // REFILETABLE_HPP
