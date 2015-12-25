/*
 * ReCharPtrMap.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RECHARPTRMAP_HPP
#define RECHARPTRMAP_HPP

class ReKeyCharPtr {
   friend bool operator <(ReKeyCharPtr const& op1, ReKeyCharPtr const& op2);
public:
   ReKeyCharPtr(const char* ptr);
private:
   const char* m_ptr;
};
/**
 * @brief Compares two instances of the class <code>ReKeyCharPtr</code>.
 * @param op1   1st operand
 * @param op2   2nd operand
 * @return      true: op1 < op2<br>
 *              false: op1 >= op2
 */
inline bool operator <(ReKeyCharPtr const& op1, ReKeyCharPtr const& op2) {
   bool rc = strcmp(op1.m_ptr, op2.m_ptr) < 0;
   return rc;
}

template<class ValueType>
class ReCharPtrMap: public QMap<ReKeyCharPtr, ValueType> {
};

#endif // RECHARPTRMAP_HPP
