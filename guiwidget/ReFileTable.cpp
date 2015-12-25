/*
 * ReFileTable.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#include "guiwidget/reguiwidget.hpp"

ReFileTable::ReFileTable(QWidget* parent) :
   QWidget(parent),
   mainLayout(new QVBoxLayout(this)),
   horizontalLayout(new QHBoxLayout(this)),
   comboBoxPath(new QComboBox(this)),
   comboBoxPatterns(new QComboBox(this)),
   pushButtonDevice(new QPushButton("...", this)),
   pushButtonUp(new QPushButton("^", this)),
   pushButtonRoot(new QPushButton("/", this)),
   tableWidget(new QTableWidget(this)),
   fileSystem(NULL),
   matcher("*"),
   m_dateFormat("yyyy.MM.dd hh:mm:ss"),
   announcer(NULL) {
   setLayout(mainLayout);
   mainLayout->setSpacing(0);
   mainLayout->addLayout(horizontalLayout);
   horizontalLayout->addWidget(pushButtonDevice);
   pushButtonDevice->setMaximumWidth(25);
   horizontalLayout->addWidget(comboBoxPath);
   comboBoxPath->setEditable(true);
   horizontalLayout->addWidget(pushButtonUp);
   pushButtonUp->setMaximumWidth(25);
   horizontalLayout->addWidget(comboBoxPatterns);
   horizontalLayout->setSpacing(0);
   comboBoxPatterns->setEditable(true);
   comboBoxPatterns->setMaximumWidth(150);
   horizontalLayout->addWidget(pushButtonRoot);
   pushButtonRoot->setMaximumWidth(25);
   mainLayout->addWidget(tableWidget);
   QStringList labels;
   labels.append(tr("Type"));
   labels.append(tr("Modified"));
   labels.append(tr("Size"));
   labels.append(tr("Name"));
   tableWidget->setColumnCount(COL_COUNT);
   tableWidget->setColumnWidth(TYPE, 60);
   tableWidget->setColumnWidth(SIZE, 125);
   tableWidget->setColumnWidth(MODIFIED, 175);
   tableWidget->setDragDropMode(QAbstractItemView::DragDrop);
   tableWidget->setDragEnabled(true);
   tableWidget->setDragDropOverwriteMode(true);
   tableWidget->setAcceptDrops(true);
   connect(pushButtonUp, SIGNAL(clicked()), SLOT(pushButtonUpClicked()));
   connect(pushButtonRoot, SIGNAL(clicked()), SLOT(pushButtonRootClicked()));
   connect(tableWidget, SIGNAL(cellDoubleClicked(int, int)), this,
           SLOT(tableDoubleClicked(int, int)));
   tableWidget->setHorizontalHeaderLabels(labels);
   tableWidget->horizontalHeader()->setStretchLastSection(true);
   tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

/**
 * Destructor.
 */
ReFileTable::~ReFileTable() {
}

/**
 * Gets the absolute path of the file in the given row.
 *
 * @param row       the row number
 * @param withNode  true: the node will be appended to the result
 * @param uriFormat true: example: "file:///u:/tmp/file.txt"<br>
 *                  false: example: "u:\\tmp\\file.txt"
 * @return          the absolute path of the file given by the row
 */
QString ReFileTable::buildAbsPath(int row, bool withNode, bool uriFormat) {
   QString rc;
   QString node = cellAsText(row, NAME);
   rc.reserve(7 + fileSystem->directory().length() + node.length());
   rc = "file://";
   rc.append(fileSystem->directory());
   if (withNode)
      rc.append(node);
   if (uriFormat) {
#if defined WIN32
      rc = rc.replace('\\', '/');
#endif
   }
   return rc;
}

/**
 * Gets the content of the given cell as string.
 *
 * @param row   the row number: 0..R-1
 * @param col   the column number: 0..C-1
 * @return      the text of the given cell
 */
QString ReFileTable::cellAsText(int row, int col) {
   QTableWidgetItem* widget = tableWidget->item(row, col);
   QString rc;
   if (widget != NULL)
      rc = widget->text();
   return rc;
}

/**
 * Processes the dragging operation of the selected files in the table widget.
 */
void ReFileTable::fileDragging() {
   QDrag* drag = new QDrag(this);
   QMimeData* mimeData = new QMimeData;
   QList < QUrl > urls;
   QList < QTableWidgetSelectionRange > ranges =
      tableWidget->selectedRanges();
   QList <QTableWidgetSelectionRange>::iterator it;
   int files = 0;
   int dirs = 0;
   bool isDir = false;
   for (it = ranges.begin(); it != ranges.end(); ++it) {
      for (int row = (*it).topRow(); row <= (*it).bottomRow(); row++) {
         isDir = cellAsText(row, SIZE).isEmpty();
         QUrl url(buildAbsPath(row, true, true));
         urls.append(url);
         if (isDir)
            dirs++;
         else
            files++;
      }
   }
   if (urls.size() > 0) {
      mimeData->setUrls(urls);
      drag->setMimeData(mimeData);
      QPixmap image(200, 30);
      QPainter painter(&image);
      QString msg;
      if (urls.size() == 1)
         msg = tr("copy ") + ReFileUtils::nodeOf(urls.at(0).toString());
      else if (files > 0 && dirs > 0)
         msg = tr("copy %1 file(s) and %2 dir(s)").arg(files).arg(dirs);
      else if (files > 0)
         msg = tr("copy %1 file(s)").arg(files);
      else
         msg = tr("copy %1 dirs(s)").arg(dirs);
      painter.fillRect(image.rect(), Qt::white);
      painter.drawText(10, 20, msg);
      drag->setPixmap(image);
      Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
   }
}

/**
 * Copies the selected files into the clipboard.
 *
 * @param currentRow    the row where the context menu is called
 */
void ReFileTable::copyFromClipboard(int currentRow) {
   QClipboard* clipboard = QApplication::clipboard();
   const QMimeData* mimeData = clipboard->mimeData();
   ReFileSystem* source = NULL;
   if (! mimeData->hasUrls())
      say(LOG_INFO, tr("No files in clipboard"));
   else {
      QList<QUrl> urls = mimeData->urls();
      QList<QUrl>::const_iterator it;
      ReFileMetaData srcFile;
      QString url;
      for (it = urls.cbegin(); it != urls.cend(); ++it) {
         url = it->url();
         if (source == NULL) {
            if ( (source = ReFileSystem::buildFromUrl(url)) == NULL) {
               say(LOG_ERROR, tr("unknown filesystem: %1").arg(url));
               break;
            }
         }
         if (source->findByUrl(url, srcFile))
            fileSystem->copy(srcFile, *source);
      }
   }
}

/**
 * Copies the selected files into the clipboard.
 *
 * @param currentRow    the row where the context menu is called
 * @param full          the full name of the current row
 */
void ReFileTable::copyToClipboard(int currentRow, const QString& full) {
   QMimeData* mimeData = new QMimeData;
   QList < QUrl > urls;
   bool isInSelection = currentRow == -1;
   QList < QTableWidgetSelectionRange > ranges =
      tableWidget->selectedRanges();
   QList <QTableWidgetSelectionRange>::iterator it;
   QString textList;
   textList.reserve(tableWidget->rowCount() * 80);
   for (it = ranges.begin(); it != ranges.end(); ++it) {
      for (int row = (*it).topRow(); row <= (*it).bottomRow(); row++) {
         if (currentRow != -1)
            isInSelection = isInSelection || row == currentRow;
         QString name(buildAbsPath(row, true));
         QUrl url(name);
         textList += name + '\n';
         urls.append(url);
      }
   }
   if (!isInSelection) {
      urls.clear();
      urls.append(QUrl(full));
      textList = full;
   }
   mimeData->setUrls(urls);
   mimeData->setText(textList);
   QClipboard* clipboard = QApplication::clipboard();
   clipboard->setMimeData(mimeData);
   say(LOG_INFO,
       tr("%1 entry/entries copied to clipboard").arg(urls.length()));
}
/**
 * Handles the event "drag enter".
 *
 * @param event	the event data
 */
void ReFileTable::dragEnterEvent(QDragEnterEvent* event) {
   if (event->mimeData()->hasFormat("text/uri-list"))
      event->acceptProposedAction();
}

void ReFileTable::dropEvent(QDropEvent* event) {
   QList<QUrl> urls = event->mimeData()->urls();
   if (! urls.isEmpty()) {
      QString fileName = urls.first().toLocalFile();
      say(LOG_INFO, "copy " + fileName);
   }
}

/**
 * Fills the table with the file data of the filesystem.
 */
void ReFileTable::fillTable() {
   if (fileSystem != NULL) {
      ReFileMetaDataList list;
      fileSystem->listInfos(matcher, list);
      ReFileMetaDataList::const_iterator it;
      tableWidget->setRowCount(list.length());
      int row = 0;
      for (it = list.cbegin(); it != list.cend(); ++it) {
         bool isDir = S_ISDIR(it->m_mode);
         QString modified = it->m_modified.toString(m_dateFormat);
         QString size = isDir ? ReQStringUtils::m_empty : QString::number(it->m_size);
         QString ext = isDir ? tr("<dir>") : ReFileUtils::extensionOf(it->m_node);
         if (tableWidget->item(row, 0) == NULL) {
            QTableWidgetItem* item = new QTableWidgetItem(it->m_node);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            tableWidget->setItem(row, NAME, item);
            item = new QTableWidgetItem(modified);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            tableWidget->setItem(row, MODIFIED, item);
            item = new QTableWidgetItem(ext);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            tableWidget->setItem(row, TYPE, item);
            item = new QTableWidgetItem(size);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            tableWidget->setItem(row, SIZE, item);
         } else {
            tableWidget->item(row, NAME)->setText(it->m_node);
            tableWidget->item(row, MODIFIED)->setText(modified);
            tableWidget->item(row, TYPE)->setText(ext);
            tableWidget->item(row, SIZE)->setText(size);
         }
         row++;
      }
   }
}

/**
 * Change the include/exclude patterns of the file table.
 *
 * @param patterns	a comma separated list of patterns
 */
void ReFileTable::changePatterns(const QString& patterns) {
   // update the history:
   comboText(comboBoxPatterns);
   matcher.setPatterns(patterns, ',', '-');
   fillTable();
}

/**
 * Changes the current directory.
 *
 * @param directory	full path of the new directory
 * @return			<code>true</code>: success<br>
 *					<code>false</code>: error
 */
bool ReFileTable::changeDirectory(QString directory) {
   bool rc = true;
   directory = ReFileUtils::cleanPath(directory);
   ReQStringUtils::ensureLastChar(directory, OS_SEPARATOR);
   if (directory != fileSystem->directory()) {
      if (directory.indexOf('*') < 0) {
         ReFileSystem::ErrorCode rc2 = fileSystem->setDirectory(directory);
         if (rc2 != ReFileSystem::EC_SUCCESS) {
            rc = say(LOG_ERROR, fileSystem->errorMessage(rc2) + " " + directory);
         } else {
            comboBoxPath->setCurrentText(directory);
            // update the history:
            comboText(comboBoxPath);
            fillTable();
         }
      } else {
      }
   }
   return rc;
}

/**
 * Handles the key press event.
 *
 * @param event	event data
 */
void ReFileTable::keyPressEvent(QKeyEvent* event) {
   Qt::KeyboardModifiers modifiers = event->modifiers();
   QWidget* sender = QApplication::focusWidget();
   int key = event->key();
   if (sender == comboBoxPath) {
      if (key == Qt::Key_Return) {
         if (modifiers == Qt::NoModifier)
            changeDirectory(comboBoxPath->currentText());
      }
   } else if (sender == comboBoxPatterns) {
      if (key == Qt::Key_Return && modifiers == Qt::NoModifier)
         changePatterns(comboBoxPatterns->currentText());
   } else if (sender == tableWidget) {
      if (key == Qt::Key_Return && modifiers == Qt::NoModifier)
         openEntry(tableWidget->currentRow());
      else if (key == Qt::Key_C && modifiers == Qt::ControlModifier)
         copyToClipboard();
   }
}

/**
 * Starts the standard program for the given table entry.
 *
 * @param row	the row containing the entry to open
 */
void ReFileTable::openEntry(int row) {
   bool isDir = tableWidget->item(row, TYPE)->text().startsWith('<');
   QString node = tableWidget->item(row, NAME)->text();
   if (isDir) {
      changeDirectory(fileSystem->directory() + node);
   } else {
   }
}

/**
 * Handles the event push button "up" clicked()
 */
void ReFileTable::pushButtonUpClicked() {
   QString path(fileSystem->directory());
   path.resize(max(0, path.length() - 1));
   if (! path.isEmpty())
      changeDirectory(ReFileUtils::parentOf(path));
}

/**
 * Handles the event push button "root" clicked()
 */
void ReFileTable::pushButtonRootClicked() {
   changeDirectory("/");
}

/**
 * Handles user messages including errors/warnings.
 *
 * @param level		type of the message: LOG_ERROR...
 * @param message	message for the user
 * @return			<code>false</code>: level == LOG_ERROR or LOG_WARNING<br>
 *					<code>true</code>: level >= LOG_INFO
 */
bool ReFileTable::say(ReLoggerLevel level, const QString& message) {
   if (announcer != NULL)
      announcer->say(level, message);
   return level >= LOG_INFO;
}

/**
 * Handles the double click of a table cell.
 *
 * @param row		the row of the cell has been clicked
 * @param column	the column of the cell has been clicked
 */
void ReFileTable::tableDoubleClicked(int row, int column) {
   openEntry(row);
}
