/*
 * Prime.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "math.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"
#include "unistd.h"
#include "time.h"
#include "assert.h"
#include "base/rebase.hpp"
#include "Prime.hpp"

Prime::Prime(int64_t from, int count) :
   m_from(from),
   m_count(count),
   m_primes(NULL),
   m_sizePrimes(0),
   m_countPrimes(0),
   m_maxStoredPrimes(0) {
   m_maxStoredPrimes = int (sqrt((double) m_from / 3 / log(sqrt(m_from)))) + 100;
   m_sizePrimes = m_maxStoredPrimes + 10;
   fprintf(stderr, "Storing %d primes\n", m_sizePrimes);
   m_primes = new int64_t[m_sizePrimes];
   m_primes[0] = 2;
   m_countPrimes = 1;
}

Prime::~Prime() {
}
void toFile(const char* prefix, int64_t* primes, int count) {
   QByteArray fn = QByteArray(prefix) + "."
                   + QDateTime::currentDateTime().toLocalTime()
                   .toString("yyyy.MM.dd_hh_mm_ss").toLatin1() + ".txt";
   FILE* fp = fopen(fn.constData(), "w");
   if (fp != NULL) {
      fprintf(stderr, "Result in %s\n", fn.constData());
      for (int ix = 0; ix < count; ix++) {
         fprintf(fp, "%lldL, // %llx\n",
                 (long long) primes[ix], (long long) primes[ix]);
      }
      fclose(fp);
   }
}

void Prime::calculate() {
   int64_t x = lastPrime() * lastPrime();
   int64_t* primes = new int64_t[m_count];
   assert(x % 2 == 1);
   int nPrinted = 0;
   while (nPrinted < m_count) {
      while (x > 0) {
         x -= 2;
         bool isPrime = true;
         for (int ix = 0; ix < m_countPrimes; ix++) {
            int64_t fac = m_primes[ix];
            if (x % fac == 0) {
               isPrime = false;
               break;
            }
         }
         if (isPrime) {
            printf("%lld, // %llx\n", (long long) x, (long long) x);
            fflush(stdout);
            primes[nPrinted++] = x;
            x -= m_from / m_count / 5;
            if (x % 2 == 0)
               x--;
            break;
         }
      }
   }
   toFile("primes.sorted", primes, m_count);
   ReKISSRandomizer random;
   random.nearTrueRandom();
   random.shuffle(primes, m_count, sizeof primes[0]);
   toFile("primes.shuffled", primes, m_count);
}

void Prime::storePrimes() {
   clock_t start = clock();
   int counter2 = 0;
   for (int64_t x = 3; true; x += 2) {
      if (x % (10*1000*1000) == 1) {
         fprintf(stderr, "%ld Mio: %8.3f\n", x / (1000*1000),
                 (clock() - start) / double(CLOCKS_PER_SEC));
         if (counter2++ % 5 == 0)
            dump();
         fflush(stderr);
      }
      bool isPrime = true;
      for (int ix = 0; ix < m_countPrimes; ix++) {
         int64_t fac = m_primes[ix];
         if (x % fac == 0) {
            isPrime = false;
            break;
         } else if (fac * fac > x)
            break;
      }
      if (isPrime) {
         m_primes[m_countPrimes++] = x;
         // hex(sqrt(2**63-1)) == 0xb504f333:
         if (m_countPrimes >= m_maxStoredPrimes
               || x * x > m_from || x > 0xb504f333)
            break;
      }
   }
   dump();
}

void Prime::dump() {
   int64_t last = lastPrime();
   fprintf(stderr, "count/primes: %d/%d last: %lx lp*lp: %lx %.2f%%\n",
           m_countPrimes, m_sizePrimes, last, last * last,
           (double) last * last * 100.0 / m_from);
}

void Prime::run(int argc, char* argv[]) {
   int64_t from = 0x7fffffffefffefffL;
   int count = 150;
   if (argc > 1) {
      int64_t lValue;
      if (sscanf(argv[1], "0x%lx", &lValue) == 1) {
         from = lValue;
      } else if (sscanf(argv[1], "%ld", &lValue) == 1)
         from = lValue;
      if (argc > 2) {
         int nValue;
         if (sscanf(argv[2], "%d", &nValue) != 1)
            printf("wrong argument 2: %s", argv[2]);
         else
            count = nValue;
      }
   }
   fprintf(stderr, "from: %ld/%lx count: %d\n", from, from, count);
   clock_t start = clock();
   Prime prime(from, count);
   prime.storePrimes();
   fprintf (stderr, "storePrime: %f sec\n",
            double (clock() - start) / CLOCKS_PER_SEC);
   prime.dump();
   prime.calculate();
   double duration = double (clock() - start) / CLOCKS_PER_SEC;
   fprintf (stderr, "duration: %f sec\n", duration);
}

