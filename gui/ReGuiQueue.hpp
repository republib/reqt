/*
 * ReGuiQueue.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REGUIQUEUE_HPP
#define REGUIQUEUE_HPP

class ReGuiQueueItem {
public:
   enum WidgetType {
      Undef, LabelText, NewTableRow, LogMessage, ReadyMessage,
      UserDefined1, UserDefined2
   };

public:
   /** Constructor.
    */
   ReGuiQueueItem():
      m_type(Undef),
      m_widget(NULL),
      m_value() {
   }

   /** Constructor.
    * @param type		widget type
    * @param widget	NULL or the widget
    * @param value		the value to set
    */
   ReGuiQueueItem(WidgetType type, QWidget* widget, const QString value) :
      m_type(type),
      m_widget(widget),
      m_value(value) {
   }
   /** Copy constructor.
    * @param source	the source to copy
    */
   ReGuiQueueItem(const ReGuiQueueItem& source) :
      m_type(source.m_type),
      m_widget(source.m_widget),
      m_value(source.m_value) {
   }
   /** Assign operator.
    * @param source	the source to copy
    * @return			the instance
    */
   ReGuiQueueItem& operator = (const ReGuiQueueItem& source) {
      m_type = source.m_type;
      m_widget = source.m_widget;
      m_value = source.m_value;
      return *this;
   }
public:
   bool apply() const;
public:
   WidgetType m_type;
   QWidget* m_widget;
   QString m_value;
};

/**
 * Queue for exchange gui data for the main thread.
 *
 * Qt allows manipulating GUI elements only in the main thread.
 * This queue allows the exchange of information from other threads.
 */
class ReGuiQueue : protected QVector<ReGuiQueueItem> {
public:
   ReGuiQueue();
public:
   int count() const;
   ReGuiQueueItem popFront();
   void pushBack(const ReGuiQueueItem& item);
protected:
   QMutex m_locker;
};

#endif // REGUIQUEUE_HPP
