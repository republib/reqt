/*
 * ReRandom.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 *
 * @brief Implements pseudo random generators.
 */
/** @file math/ReRandom.hpp
 *
 * @brief Definitions for pseudo random generators.
 */
#include "base/rebase.hpp"
#include "math/remath.hpp"

/** @class ReRandom ReRandom.hpp "math/ReRandom.hpp"
 *
 * @brief Implements a portable pseudo random generator.
 *
 */

/**
 * @brief Constructor.
 */
ReRandom::ReRandom() :
   m_seed(0) {
}

/**
 * @brief Destructor.
 */
ReRandom::~ReRandom() {
}

/**
 * @brief Returns the next random number as 64 bit unsigned integer.
 *
 * @return the next random number.
 */
uint64_t ReRandom::nextInt64() {
   // Donald Knuth recommands (for 64-Bit):
   m_seed = m_seed * 6364136223846793005L + 1442695040888963407L;
   return m_seed;
}
/**
 * @brief Sets the random seed.
 *
 * @param seed      the new seed.
 */
void ReRandom::setSeed(uint64_t seed) {
   m_seed = seed;
}
/**
 * @brief Modifies the seed.
 *
 * @param seed      the XOR operand.
 */
void ReRandom::xorSeed(uint64_t seed) {
   m_seed ^= seed;
}

/**
 * @brief nextByte  returns the next random byte.
 *
 * @return          a pseudo random value 0..255
 */
uint8_t ReRandom::nextByte() {
   uint64_t value = nextInt64();
   // forget the last 3 bits:
   uint8_t rc = (uint8_t) (value >> 3) % 256;
   return rc;
}

/**
 * @brief Returns the next pseudo random integer.
 *
 * @param minValue  the minimal result (including)
 * @param maxValue  the maximal result (includeing)
 * @return the next integer
 */
int ReRandom::nextInt(int minValue, int maxValue) {
   uint64_t value = nextInt64();
   uint64_t diff = maxValue - minValue;
   int rc;
   if (diff <= 0)
      rc = minValue;
   else
      rc = (int) (minValue + value % diff);
   return rc;
}

/**
 * @brief Returns a random string.
 *
 * @param length    the length of the result
 * @param minChar   all characters of the result are greater or equal than this value
 * @param maxChar   all characters of the result are lower or equal than this value
 * @return          a random string
 */
QByteArray ReRandom::nextString(int length, char minChar, char maxChar) {
   QByteArray rc;
   rc.resize(length);
   for (int ii = 0; ii < length; ii++) {
      rc[ii] = nextInt(minChar, maxChar);
   }
   return rc;
}

/**
 * @brief Returns a random string.
 *
 * @param length    the length of the result
 * @param charSet   a string with all allowed characters
 * @return          a random string with characters from the given set
 */
QByteArray ReRandom::nextString(int length, char* charSet) {
   QByteArray rc;
   rc.resize(length);
   int ubound = strlen(charSet) - 1;
   for (int ii = 0; ii < length; ii++) {
      rc[ii] = charSet[nextInt(0, ubound)];
   }
   return rc;
}

