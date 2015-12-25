/*
 * ReRandom.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RERANDOM_HPP
#define RERANDOM_HPP

class ReRandom {
public:
   ReRandom();
   virtual ~ReRandom();
private:
   // No copy constructor: no implementation!
   ReRandom(const ReRandom& source);
   // Prohibits assignment operator: no implementation!
   ReRandom& operator =(const ReRandom& source);
public:
   virtual uint64_t nextInt64();
   virtual void setSeed(uint64_t seed);
   void xorSeed(uint64_t seed);
   uint8_t nextByte();
   int nextInt(int minValue, int maxValue);
   QByteArray nextString(int length = 8, char minChar = ' ',
                         char maxChar = 127);
   QByteArray nextString(int length, char* charSet);
protected:
   uint64_t m_seed;
};

#endif // RERANDOM_HPP
