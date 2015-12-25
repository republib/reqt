/*
 * regui.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REGUI_HPP
#define REGUI_HPP

#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include "gui/ReGuiQueue.hpp"
#include "gui/ReStateStorage.hpp"
#include "gui/ReGuiValidator.hpp"
#include "gui/ReEdit.hpp"
#include "gui/ReSettings.hpp"
#include "gui/ReFileTree.hpp"
/**
 * Tests whether a point is inside the rectangle (including border).
 * @param rect  rectangle to test
 * @param point point to test
 * @param what description of the caller
 * @return      <code>true</code>: the point lays inside the rectangle
 */
inline bool rectContains(const QRect& rect, const QPoint& point,
                         const char* what = "") {
#if 1
   ReUseParameter(what);
   return point.x() >= rect.x() && point.y() >= rect.y()
          && point.x() < rect.x() + rect.width()
          && point.y() < rect.y() + rect.height();
#else
   bool rc = point.x() >= rect.x();
   char reason = ' ';
   if (! rc)
      reason = 'x';
   else {
      rc = point.y() >= rect.y();
      if (! rc)
         reason = 'y';
      else {
         rc = point.x() < rect.x() + rect.width();
         if (! rc)
            reason = 'X';
         else {
            rc = point.y() < rect.y() + rect.height();
            if (! rc)
               reason = 'Y';
         }
      }
   }
   if (! rc)
      ReLogger::globalLogger()->logv(LOG_INFO, 1, "rectContains(%s) %c %d/%d %d-%d/%d-%d",
                                     what, reason, point.x(), point.y(), rect.x(), rect.x() + rect.width(),
                                     rect.y(), rect.y() + rect.height());
   return rc;
#endif
}

#endif /* REGUI_HPP */
