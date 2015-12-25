/*
 * ReDiff.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"


/**
 * Constructor.
 *
 * @param list1	first line list
 * @param list2 2nd line list
 */
ReDiff::ReDiff(const QStringList& list1, const QStringList& list2) :
   m_list1(list1), m_list2(list2) {
}

ReCommonSlice ReDiff::longestMatchingSlice(int from1, int to1, int from2, int to2) {
   ReCommonSlice rc;
   rc.m_from1 = from1;
   rc.m_from2 = from2;
   rc.m_count = 0;
   int nMax = max(to1 - from1, to2 - from2);
   int* runs = new int[nMax];
   int* runs2 = new int[nMax];
   memset(runs, 0, nMax * sizeof runs[0]);
   for (int ix1 = from1; ix1 < to1; ix1++) {
      memset(runs2, 0, nMax * sizeof runs[0]);
      for (int ix2 = from2; ix2 < to2; ix2++) {
         if (m_list1.at(ix1) == m_list2.at(ix2)) {
            int count = runs2[ix2] = runs[ix2 - 1] + 1;
            if (count > rc.m_count) {
               rc.m_from1 = ix1 - count + 1;
               rc.m_from2 = ix2 - count + 1;
               rc.m_count = count;
            }
         }
      }
      runs = runs2;
   }
   delete runs;
   delete runs2;
   return rc;
}

/**
 * Calculates the difference lists.
 */
void ReDiff::build() {
   /*
    * def longest_matching_slice(a, a0, a1, b, b0, b1):
   sa, sb, n = a0, b0, 0

   runs = {}
   for i in range(a0, a1):
   	new_runs = {}
   	for j in range(b0, b1):
   		if a[i] == b[j]:
   			k = new_runs[j] = runs.get(j-1, 0) + 1
   			if k > n:
   				sa, sb, n = i-k+1, j-k+1, k
   	runs = new_runs

   assert a[sa:sa+n] == b[sb:sb+n]
   return sa, sb, n

    */
}
