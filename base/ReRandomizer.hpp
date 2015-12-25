/*
 * ReRandomizer.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RANDOMIZER_H_
#define RANDOMIZER_H_

/**
 * Abstract base class for hash generators.
 */
class ReDigest {
public:
   /** Returns the hash value as byte array.
    * Note: After call of this method the hash is reset!
    * @return	the hash value as byte array, stored in "little endian" byte order
    */
   virtual QByteArray digest() = 0;
   /** Resets the hash.
    * The state of the instance is the same as after the constructor.
    */
   virtual void reset() = 0;
   /** Adds the data to the hash.
    * @param source	the data block which will be added to the hash
    * @param length	the length of the data (in bytes)
    */
   virtual void update(const void* source, size_t length) = 0;
public:
   QByteArray hexDigest();
   /** Adds the data to the hash.
    * @param source	the data block which will be added to the hash
    */
   inline virtual void updateBlock(const QByteArray& source) {
      update(source.constData(), source.length());
   }
};

/**
 * Implements a very simple (and cheap) checksum.
 *
 * Features:
 * <ul><li>If one bit of the content is changed normally many bits of the sum
 * are changed</li>
 * <li>If a '\\0' is appended to a sequence of '\\0'' many bits of the sum
 * are change too</li>
 * <li>64 bit checksum: low likelihood of collision</li>
 * <li>Only 6 basic 64 bit operations (*, 2 ^, +, %, >>) per 8 byte content</li>
 * <li>Works for little/big endian architectures</li>
 * </ul>
 */
class ReHmHash64 : public ReDigest {
public:
   ReHmHash64(int64_t factor = 0x70cf79d585f5a313L,
              int64_t increment = 0x75c280b9881252dbL);
public:
   int64_t digestAsInt();
public:
   virtual QByteArray digest();
   virtual void reset();
   virtual void update(const void* source, size_t length);

private:
   int64_t m_factor;
   int64_t m_increment;
   int64_t m_hash;
   int64_t m_sumLength;
   QByteArray m_rest;
};
/**
 * This implements an abstract base class for random generators.
 */
class ReRandomizer {
public:
   enum {
      START_RANGE = ' ',
      CHARRANGE = 128 - START_RANGE
   };
   typedef int64_t seed_t;
public:
   ReRandomizer(const char* name);
   virtual ~ReRandomizer();
public:
   void codec(uint8_t* target, const uint8_t* source, int length);
   /** Encode/decode data in a buffer.
    * @param buffer	IN/OUT: data to encode/decode
    */
   inline void codec(QByteArray& buffer) {
      codec((uint8_t*)buffer.data(), (const uint8_t*) buffer.constData(),
            buffer.length());
   }
   /** Encode/decode data.
    * @param target	OUT: target for the de/encoded data
    * @param source	source of the de/encoded data
    * @param start		first index to encode/decode. The data lying in front
    *					will be copied
    */
   inline void codec(QByteArray& target, const QByteArray& source, int start = 0) {
      target.resize(source.length());
      if (start > 0)
         memcpy((uint8_t*)target.data(), (const uint8_t*) source.constData(),
                min(source.length(), start));
      if (start < source.length())
         codec((uint8_t*)target.data() + start,
               (const uint8_t*) source.constData() + start,
               target.length() - start);
   }
   const QByteArray& name() const;
   char nextChar();
   QByteArray& nextData(int minLength, int maxLength, QByteArray& buffer);
   int nextInt(int maxValue = INT_MAX, int minValue = 0);
   int64_t nextInt64(int64_t maxValue = LLONG_MAX, int64_t minValue = 0);
   const char* nextString(int minLength, int maxLength, QByteArray& buffer);
   void shuffle(void* array, size_t length, size_t elemSize,
                int exchanges = -1);
public:
   virtual void dump() {
   }
   /** @brief Modifies the current seed with a 64-bit value.
    * @param seed		the value to modify the current seed
    */
   virtual void modifySeed(int64_t seed) = 0;
   /** @brief Returns the next pseudo random number.
    * @return the next pseudo random number
    * */
   virtual seed_t nextSeed64() = 0;
   /** @brief Sets the instance to a defined start state.
    */
   virtual void reset() = 0;
   /**
    * Sets the current point of pseudo random with a seed saved by <code>saveSeed()</code>.
    *
    * @param seed	the current point of pseudo random
    */
   virtual void restoreSeed(const QByteArray& seed) = 0;
   /**
    * Stores the the current point of pseudo random in a buffer.
    *
    * Restoring is done by <code>restoreSeed()</code>
    *
    * @param seed	the current point of pseudo random
    */
   virtual void saveSeed(QByteArray& seed) const = 0;
   /**
    * Converts a text (e.g. password) into the generator specific seed.
    * @param text	the text to convert
    */
   virtual void textToSeed(const QByteArray& text) = 0;
public:
   static seed_t hash(const QByteArray& text);
   static void hash(const QByteArray& text, QByteArray& seed);
   static seed_t pseudoTrueRandom();
   static seed_t nearTrueRandom();
protected:
   QByteArray m_name;
   int m_counter;
public:
   // the first 125 prime numbers
   static const int64_t m_primes64[];
   static const int m_countPrimes;
};

/**
 * Base class for pseudo random generators with a state variable (seed) of 64 bit.
 */
class ReSingleSeedRandomizer: public ReRandomizer {
public:
   ReSingleSeedRandomizer(const char* name);
public:
   virtual void modifySeed(int64_t seed);
   virtual void reset();
   seed_t seed() const;
   void setSeed(seed_t m_seed);
   virtual void restoreSeed(const QByteArray& seed);
   virtual void saveSeed(QByteArray& seed) const;
   virtual void textToSeed(const QByteArray& text);
protected:
   seed_t m_seed;
   seed_t m_lastSetSeed;
};

/**
 * A pseudo random generator returning constantly 0.
 *
 * This is useful for testing.
 */
class ReNullRandomizer: public ReSingleSeedRandomizer {
public:
   ReNullRandomizer();
public:

   // ReRandomizer interface
public:
   virtual void modifySeed(int64_t seed);
   virtual seed_t nextSeed64();
};

class ReCongruentialGeneratorBase {
public:
   ReCongruentialGeneratorBase();
public:
   ReRandomizer::seed_t factor() const;
   ReRandomizer::seed_t increment() const;
   void setFactor(ReRandomizer::seed_t factor);
   void setIncrement(ReRandomizer::seed_t increment);
protected:
   ReRandomizer::seed_t m_factor;
   ReRandomizer::seed_t m_increment;
};

/**
 * Implements a simple pseudo random generator.
 * A linear congruential generator produces the next value using this formula:
 * seed = (seed * factor + increment) % modulus
 * In this implementation modulus is 2**64.
 */
class ReCongruentialGenerator: public ReSingleSeedRandomizer,
   public ReCongruentialGeneratorBase {
public:
   ReCongruentialGenerator();
public:
   virtual seed_t nextSeed64();
protected:
   ReCongruentialGenerator(const char* name);
protected:
   friend class ReShiftRandom;
};

/**
 * A simple pseudo random generator simular to <code>ReCongruentialGenerator</code>.
 *
 * A congruential generator produces good random in the most significant
 * bits. Therefore we exchange the bits of the result.
 * Then x % m returns better results.
 */
class ReRotateRandomizer: public ReCongruentialGenerator {
public:
   ReRotateRandomizer();
protected:
   virtual seed_t nextSeed64();
};

/**
 * Abstract base class for pseudo random generators with a state variable larger than 64 bit.
 */
class ReMultiSeedRandomizer: public ReRandomizer {
public:
   ReMultiSeedRandomizer(int countSeeds, const char* name);
public:
   virtual void modifySeed(int64_t seed);
   virtual void reset();
   virtual void restoreSeed(const QByteArray& seed);
   virtual void saveSeed(QByteArray& seed) const;
   virtual void textToSeed(const QByteArray& text);
protected:
   int m_countSeeds;
   QByteArray m_seedBuffer;
   QByteArray m_startSeed;
   seed_t* m_seeds;
};

/**
 * A simple pseudo random generator simular to <code>ReCongruentialGenerator</code>.
 *
 * Difference: An array of seeds is used (round robin).
 */
class ReMultiCongruentialGenerator: public ReMultiSeedRandomizer,
   public ReCongruentialGeneratorBase {
public:
   ReMultiCongruentialGenerator(int countSeeds);
public:
   virtual void reset();
   virtual seed_t nextSeed64();
protected:
   int m_currentSeed;
};

class ReXorShift64Randomizer: public ReSingleSeedRandomizer {
public:
   ReXorShift64Randomizer();
public:
   virtual seed_t nextSeed64();
};

/**
 * A simple pseudo random generator but it fulfills some statistically tests.
 *
 * Combines a congruent generator, a xor-shift generator and the multiply
 * with carry method.
 */
class ReKISSRandomizer: public ReRandomizer, public ReCongruentialGeneratorBase {
public:
   typedef struct {
      seed_t m_x;
      seed_t m_y;
      seed_t m_z;
      seed_t m_c;
   } params_t;
public:
   ReKISSRandomizer();
public:
   virtual void dump();
   virtual seed_t nextSeed64();
public:
   virtual void modifySeed(int64_t seed);
   virtual void reset();
   virtual void restoreSeed(const QByteArray& seed);
   virtual void saveSeed(QByteArray& seed) const;
   virtual QByteArray state() const;
   virtual void textToSeed(const QByteArray& text);
private:
   /** Stores a parameter set into a buffer.
    * @param params	parameter set of the KISS generator
    * @param buffer	OUT: buffer for saving
    */
   inline void save(const params_t& params, QByteArray& buffer) const {
      buffer.resize(sizeof params);
      memcpy(buffer.data(), &params, sizeof params);
   }
   /** Restores a parameter set from a buffer
    * @param buffer	the stored parameters
    * @param params	OUT: the parameter to restore
    */
   inline void restore(const QByteArray& buffer, params_t& params) const {
      size_t size = min(buffer.length(), sizeof params);
      memcpy(&params, buffer.constData(), size);
   }

private:
   params_t m_params;
   params_t m_startParams;
};

/**
 * A processing unit for encoding/decoding of binary data.
 *
 * The scrambler administrates a header containing a 64 bit random value (salt).
 * Encryption is done with this salt.
 * Furthermore an optional marker is set (with a given length). This is was
 * distinguishes a valid header from wrong data.
 * Encryption of the rest of the header is optional.
 *
 */
class ReByteScrambler {
public:
   ReByteScrambler(ReRandomizer& contentRandom, ReLogger* logger);
   ReByteScrambler(const ReByteScrambler& source);
   ReByteScrambler& operator =(const ReByteScrambler& source);
public:
   ReRandomizer& contentRandom(bool doReset);
   bool initFromHeader(int reservedLength, int markerLength,
                       int infoLength, int encryptedFrom, QByteArray* header,
                       QByteArray& info);
   void initHeader(int reservedLength, int markerLength, int infoLength,
                   int encryptFrom, const QByteArray& info = ReStringUtils::m_empty);
   void randomReset();
   QByteArray& header();

protected:
   ReRandomizer& m_contentRandom;
   QByteArray m_contentSeed;
   QByteArray m_buffer;
   QByteArray m_header;
   ReLogger* m_logger;
   int64_converter_t m_salt;
};

#endif /* RANDOMIZER_H_ */
