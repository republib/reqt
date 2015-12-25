/*
 * FileCommander.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#include "reidos.hpp"

/**
 * Constructor.
 */
FileCommander::FileCommander(IDosMain* main) :
   m_main(main),
   m_topFS(NULL),
   m_bottomFS(NULL),
   m_topMatcher(),
   m_bottomMatcher(),
   m_logger(new ReMemoryLogger()),
   m_topIsActive(true),
   m_dateFormat("yyyy.MM.dd hh:mm:ss") {
   m_topFS = new ReLocalFileSystem(QDir::homePath(), m_logger);
   m_bottomFS = new ReLocalFileSystem(QDir::homePath(), m_logger);
}

/**
 * Sets the active directory.
 *
 * @param path	the path to set
 * @return		<code>true</code>: directory has changed
 */
bool FileCommander::changeDirectory(const QString& path) {
   bool rc = false;
   ReFileSystem* current = m_topIsActive ? m_topFS : m_bottomFS;
   QString currentDir = current->directory();
   if (current->setDirectory(path) == ReFileSystem::EC_SUCCESS) {
      rc = currentDir != current->directory();
   }
   return rc;
}

/**
 * Fills the table with the file data of the active filesystem.
 *
 * @param table	OUT: table to fill
 */
void FileCommander::fillTable(QTableWidget* table) {
   ReFileMetaDataList list;
   ReFileSystem* fs = m_topIsActive ? m_topFS : m_bottomFS;
   fs->listInfos(m_topIsActive ? m_topMatcher : m_bottomMatcher, list);
   ReFileMetaDataList::const_iterator it;
   table->setRowCount(list.length());
   int row = 0;
   for (it = list.cbegin(); it != list.cend(); ++it) {
      // TC_EXT, TC_SIZE, TC_MODIFIED, TC_NODE
      QString modified = it->m_modified.toString(m_dateFormat);
      QString size = QString::number(it->m_size);
      QString ext = ReFileUtils::extensionOf(it->m_node);
      if (table->item(row, 0) == NULL) {
         table->setItem(row, TC_NODE, new QTableWidgetItem(it->m_node));
         table->setItem(row, TC_MODIFIED, new QTableWidgetItem(modified));
         table->setItem(row, TC_EXT, new QTableWidgetItem(ext));
         QTableWidgetItem* item = new QTableWidgetItem(size);
         item->setTextAlignment(Qt::AlignRight);
         table->setItem(row, TC_SIZE, item);
      } else {
         table->item(row, TC_NODE)->setText(it->m_node);
         table->item(row, TC_MODIFIED)->setText(modified);
         table->item(row, TC_EXT)->setText(ext);
         table->item(row, TC_SIZE)->setText(size);
      }
      row++;
   }
}

/**
 * Sets a filesystem to a filesystem given by its url.
 *
 * @param url			the Uniform Resource Locator, e.g. "file:/home/jonny"
 * @param filesystem	IN/OUT: the filesystem to change
 */
void FileCommander::buildFs(const QString& url, ReFileSystem& filesystem) {
   if (url.startsWith("file:", Qt::CaseInsensitive)) {
   } else {
      m_main->say(LOG_ERROR, QObject::tr("unknown protocol in url: ") + url);
   }
}

