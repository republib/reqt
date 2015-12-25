/*
 * mainwindow.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP
#ifndef REBASE_HPP
#include "base/rebase.hpp"
#endif
#include <QHeaderView>
#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QDir>
#if ! defined TEXTFINDER_HPP
#include "textfinder.hpp"
#endif
#if ! defined GUI_REGUI_HPP_
#include "gui/regui.hpp"
#endif
#if ! defined UTILS_HPP
#include "utils.hpp"
#endif
namespace Ui {
class MainWindow;
}
enum TableColumns {
   TC_NODE, TC_EXT, TC_SIZE, TC_MODIFIED, TC_TYPE, TC_PATH
};
class FileFinder;

class MainWindow: public QMainWindow, public ReGuiValidator, protected ReObserver {

   Q_OBJECT

public:
   explicit MainWindow(const QString& startDir, const QString& homeDir,
                       QWidget* parent = 0);
   ~MainWindow();

public:
   void fileDragging();
protected:
   void findFiles();
   void populateFinder(FileFinder& finder);
private slots:
   void about();
   void absPathToClipboard();
   void baseDirToClipboard();
   void closing();
   void clear();
   void exportFiles();
   void filePlaceholder();
   void footerPlaceholder();
   void fullNameToClipboard();
   void guiTimerUpdate();
   void handleTableContextMenu(const QPoint& position);
   void headerClicked(int col);
   void headerPlaceholder();
   virtual ReturnCode notify(const char* message);
   void options();
   void preview();
   void resetParameters();
   virtual bool say(ReLoggerLevel level, const QString& message);
   void saveState();
   void search();
   void selectDirectory();
   void selectExportFile();
   void stop();
   void up();

private:
   void prepareContextMenu();
   QString buildAbsPath(int row, bool withNode = false, bool uriFormat = false);
   QDir::Filters buildFileTypes();
   void buildGlobalPlaceholders(QMap <QString, QString>& hash);
   QString cellAsText(int row, int col);
   void exportToStream(QTextStream& stream, int maxRow = -1);
   void handleCopyToClipboard(int currentRow, const QString& full);
   void handleExternalCommand(ContextHandler* handler, const QString& parent,
                              const QString& full, const QString& node);
   void handlePlaceholder(QComboBox* target);
   void initializeHome();
   void prepareTextFind();
   QString replaceGlobalPlaceholders(QComboBox* combo,
                                     QMap <QString, QString>& placeholders);
   void restoreState();
   void startStop(bool start);
private:
   Ui::MainWindow* ui;
   QLabel* m_statusMessage;
   QPalette::ColorRole* m_stdLabelBackgroundRole;
   TextFinder m_textFinder;
   // the directory of the last search
   QDir m_lastBaseDir;
   QHeaderView* m_horizontalHeader;
   Qt::SortOrder m_lastOrder;
   Statistics m_statistics;
   QString m_homeDir;
   QString m_storageFile;
   QAction* m_actionEditor;
   QAction* m_actionStartShell;
   ContextHandlerList m_contextHandlers;
   ReLogger* m_logger;
   FileFinder* m_finder;
   ReGuiQueue m_guiQueue;
   QTimer* m_guiTimer;
};

#endif // MAINWINDOW_HPP
