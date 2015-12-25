/*
 * ReDiff.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REDIFF_HPP
#define REDIFF_HPP


class ReCommonSlice {
public:
   //@ first common line in list1
   int m_from1;
   //@ first common line in list2
   int m_from2;
   int m_count;
};

class ReDiff {
public:
   ReDiff(const QStringList& list1, const QStringList& list2);
   ~ReDiff();
public:
   void build();
protected:
   ReCommonSlice longestMatchingSlice(int from1, int to1, int from2, int to2);
protected:
   const QStringList& m_list1;
   const QStringList& m_list2;

   QList<ReCommonSlice> m_slices;
};

#endif // REDIFF_HPP
