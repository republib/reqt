/*
 * Prime.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef PRIME_HPP_
#define PRIME_HPP_

//typedef long long int int64_t;
class Prime {
public:
   Prime(int64_t from, int count);
   virtual ~Prime();
public:
   void storePrimes();
   void calculate();
   void dump();
   int64_t lastPrime() const {
      return m_primes[m_countPrimes - 1];
   }
public:
   static void run(int argc, char* argv[]);
private:
   int64_t m_from;
   int m_count;
   int64_t* m_primes;
   int m_sizePrimes;
   int m_countPrimes;
   int m_maxStoredPrimes;
};

#endif /* PRIME_HPP_ */
