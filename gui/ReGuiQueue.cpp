/*
 * ReGuiQueue.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "gui/regui.hpp"

/**
 * Constructor.
 */
ReGuiQueue::ReGuiQueue() :
   QVector<ReGuiQueueItem>(),
   m_locker() {
}

/**
 * Adds an entry at the end of the the queue.
 *
 * This method can be used by all threads.
 *
 * @param item	the item to add
 */
void ReGuiQueue::pushBack(const ReGuiQueueItem& item) {
   m_locker.lock();
   append(item);
   m_locker.unlock();
}

/**
 * Returns the number of elements.
 *
 * This method should be called only by the master thread.
 *
 * @return	the number of elements
 */
int ReGuiQueue::count() const {
   // no locking is necessary: removing is done only by the same thread
   return size();
}

/**
 * Returns the first element and delete it from the queue.
 *
 * This method should be called only by the master thread.
 *
 * @return the first elements
 */
ReGuiQueueItem ReGuiQueue::popFront() {
   m_locker.lock();
   ReGuiQueueItem rc = takeFirst();
   m_locker.unlock();
   return rc;
}

/**
 * Takes the info from the instance and put it into the widget.
 *
 * This method should only used by the master thread.
 *
 * @return	<code>true</code>: the info could be put into the widget<br>
 *			<code>false</code>: nothing is done
 */
bool ReGuiQueueItem::apply() const {
   bool rc = m_widget != NULL;
   if (rc) {
      switch(m_type) {
      case LabelText:
         reinterpret_cast<QLabel*>(m_widget)->setText(m_value);
         break;
      case NewTableRow: {
         QChar separator = m_value.at(0);
         QStringList list = m_value.mid(1).split(separator);
         QTableWidget* table = reinterpret_cast<QTableWidget*>(m_widget);
         int rowCount = table->rowCount();
         table->setRowCount(rowCount + 1);
         int cols = min(list.size(), table->columnCount());
         for (int ix = 0; ix < cols; ix++) {
            table->setItem(rowCount, ix, new QTableWidgetItem(list.at(ix)));
         }
         break;
      }
      default:
         rc = false;
         break;
      }
   }
   return rc;
}
