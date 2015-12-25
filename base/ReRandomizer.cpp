/*
 * ReRandomizer.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
//#define WITH_TRACE
#include "retrace.hpp"
enum {
   LOC_READ_1 = LOC_FIRST_OF(LOC_RANDOMIZER), // 12201
   LOC_DECODE_CONTENT_1,		// 12202
   LOC_DECODE_CONTENT_2,		// 12203
   LOC_UPDATE_1,				// 22204
};

const int64_t ReRandomizer::m_primes64[] = {
   7919787109669756829L, // 6de8 bc6a 5895 5f9d
   8190338840038832831L, // 71a9edc7de997abf
   7636937573374813723L, // 69fbda42c0d6e61b
   8079658586706028951L, // 7020b6ad3f0c2997
   7563150737819611069L, // 68f5b5865678afbd
   9161865508182333647L, // 7f257c23571c98cf
   8903611583739124583L, // 7b8ffb8fe2d2db67
   7907489303743889767L, // 6dbd0ba046db0167
   8657655465221782259L, // 78262bc680437af3
   7821404662262819933L, // 6c8b3619cac26c5d
   8719144494851117963L, // 79009fb8d8e7538b
   7686128797078281971L, // 6aaa9d6b07c05ef3
   8559273017814845539L, // 76c8a575f2708863
   8128849810409497363L, // 70cf79d585f5a313
   8854420360035656279L, // 7ae138679be96257
   9038887448923662727L, // 7d70943ea5d4e987
   7673830991152414909L, // 6a7eeca0f60600bd
   7403279260783338683L, // 66bdbb437001e4bb
   9124972090404732371L, // 7ea269c521ed7dd3
   8596166435592446783L, // 774bb7d4279fa33f
   8879015971887390451L, // 7b3899fbbf5e1ef3
   8633059853370048097L, // 77ceca325ccebe61
   8952802807442592961L, // 7c3ebeb829bc54c1
   8620762047444181001L, // 77a319684b146009
   8485486182259643099L, // 75c280b9881252db
   7526257320042009869L, // 6872a3282149950d
   8694548882999383631L, // 78a93e24b572964f
   7464768290412674327L, // 67982f35c8a5bd17
   7489363902264408529L, // 67ef90c9ec1a79d1
   7870595885966288393L, // 6d39f94211abe609
   9100376478552998239L, // 7e4b0830fe78c15f
   8534677405963111397L, // 767143e1cefbcbe5
   7415577066709205983L, // 66e96c0d81bc43df
   7809106856336952853L, // 6c5f854fb9080e15
   8805229136332187873L, // 7a32753f54ffe8e1
   7932084915595623907L, // 6e146d346a4fbde3
   8915909389664991689L, // 7bbbac59f48d39c9
   8768335718554586333L, // 79af62e11fd0ccdd
   8399401540778573443L, // 7490ab330bf9be83
   7956680527447358099L, // 6e6bcec88dc47a93
   7772213438559351497L, // 6bdc72f183d8f2c9
   8448592764482041823L, // 753f6e5b52e337df
   7784511244485218609L, // 6c0823bb95935131
   9051185254849529807L, // 7d9c4508b78f47cf
   7698426603004149029L, // 6ad64e35197abd25
   8325614705223370843L, // 738a8676a19b885b
   8214934451890567001L, // 72014f5c020e3759
   8018169557076693449L, // 6f4642bae66851c9
   8780633524480453421L, // 79db13ab318b2b2d
   8940505001516725891L, // 7c130dee1801f683
   8497783988185510157L, // 75ee318399ccb10d
   7513959514116142799L, // 6846f25e0f8f36cf
   7624639767448946537L, // 69d02978af1c8769
   9223354537811669137L, // 7ffff015afc07091
   7390981454857471589L, // 66920a795e478665
   7993573945224959303L, // 6eeee126c2f39547
   7600044155597212391L, // 6978c7e48ba7cae7
   7833702468188687047L, // 6cb6e6e3dc7ccac7
   7550852931893743991L, // 68ca04bc44be5177
   8313316899297503759L, // 735ed5ac8fe12a0f
   8301019093371636691L, // 733324e27e26cbd3
   9198758925959934931L, // 7fa88e818c4bb3d3
   8546975211888978467L, // 769cf4abe0b62a23
   7723022214855883181L, // 6b2dafc93cef79ad
   9075780866701264103L, // 7df3a69cdb0404e7
   8227232257816434127L, // 722d002613c895cf
   7759915632633484433L, // 6bb0c227721e9491
   7895191497818022631L, // 6d915ad63520a2e7
   8473188376333776041L, // 7596cfef7657f4a9
   9026589642997795633L, // 7d44e374941a8b31
   8669953271147649367L, // 7851dc9091fdd957
   8104254198557763209L, // 707818416280e689
   8337912511149237901L, // 73b63740b355e68d
   8829824748183922019L, // 7a89d6d37874a563
   8989696225220194277L, // 7cc1d1165eeb6fe5
   7649235379300680787L, // 6a278b0cd2914453
   9088078672627131179L, // 7e1f5766ecbe632b
   8387103734852706359L, // 7464fa68fa3f6037
   8116552004483630269L, // 70a3c90b743b44bd
   8153445422261231567L, // 7126db69a96a5fcf
   7501661708190275741L, // 681b4193fdd4d89d
   8928207195590858807L, // 7be75d2406479837
   8522379600037244329L, // 76459317bd416da9
   8276423481519902413L, // 72dbc34e5ab20ecd
   9137269896330599473L, // 7ece1a8f33a7dc31
   7858298080040421193L, // 6d0e4877fff18749
   7538555125967876929L, // 689e53f23303f341
   7796809050411085667L, // 6c33d485a74daf63
   8141147616335364457L, // 70fb2a9f97b00169
   8792931330406320547L, // 7a06c475434589a3
   9149567702256466567L, // 7ef9cb5945623a87
   7747617826707617371L, // 6b85115d6064365b
   8165743228187098633L, // 71528c33bb24be09
   8239530063742301213L, // 7258b0f02582f41d
   8756037912628719257L, // 7983b2170e166e99
   8866718165961523363L, // 7b0ce931ada3c0a3
   8743740106702852129L, // 7958014cfc5c1021
   8682251077073516489L, // 787d8d5aa3b837c9
   8608464241518313933L, // 7777689e395a01cd
   8731442300776985039L, // 792c5082eaa1b1cf
   7477066096338541471L, // 67c3dfffda601b9f
   7587746349671345317L, // 694d171a79ed6ca5
   9001994031146061461L, // 7ced81e070a5ce95
   8251827869668168273L, // 728461ba373d5251
   8436294958556174707L, // 7513bd914128d973
   8891313777813257509L, // 7b644ac5d1187d25
   9174163314108200743L, // 7f512ced68d6f727
   8091956392631896121L, // 704c677750c68839
   7735320020781750283L, // 6b5960934ea9d80b
   7882893691892155529L, // 6d65aa0c23664489
   7575448543745478239L, // 6921665068330e5f
   8374805928926839249L, // 7439499ee88501d1
   8030467363002560527L, // 6f71f384f822b00f
   8202636645964699889L, // 71d59e91f053d8f1
   8571570823740712651L, // 76f45640042ae6cb
   7981276139299092239L, // 6ec3305cb139370f
   8817526942258054961L, // 7a5e260966ba4731
   8411699346704440571L, // 74bc5bfd1db41cfb
   9014291837071928527L, // 7d1932aa82602ccf
   8965100613368460023L, // 7c6a6f823b76b2f7
   8706846688925250799L, // 78d4eeeec72cf4ef
   9186461120034067873L, // 7f7cddb77a9155a1
   7846000274114554117L, // 6ce297adee372905
   8460890570407908911L, // 756b1f25649d962f
   9063483060775397003L, // 7dc7f5d2c949a68b
};
const int ReRandomizer::m_countPrimes = sizeof ReRandomizer::m_primes64
                                        / sizeof ReRandomizer::m_primes64[0];

/**
 * Returns the hash value as hex string.
 *
 * Note: After call of this method the hash is reset!
 *
 * @return	the hash value as hex string, stored in "little endian" byte order
 */
QByteArray ReDigest::hexDigest() {
   QByteArray rc;
   QByteArray hash = digest();
   for (int ix = 0; ix < hash.length(); ix++) {
      rc.append(ReStringUtils::toNumber(hash.at(ix) & 0xff, "%02x"));
   }
   return rc;
}

/**
 * Contructor.
 *
 * @param factor
 * @param increment
 */
ReHmHash64::ReHmHash64(int64_t factor, int64_t increment) :
   m_factor(factor),
   m_increment(increment),
   m_hash(0),
   m_sumLength(0),
   m_rest() {
}

/**
 * Return the hash value as 64 bit integer.
 *
 * Note: After call of this method the hash is reset!
 *
 * @return the hash value as 64 bit integer
 */
int64_t ReHmHash64::digestAsInt() {
#	define CalcNextHash(data) m_hash ^= (data ^ 0x2004199111121989L) \
	* m_factor + (m_increment >> (data % 23))
   // ; printf("%016lx: -> %016lx\n", data, m_hash)
   int64_t rc;
   if (m_rest.length() > 0) {
      int64_converter_t data;
      data.m_int = 0;
      memcpy(data.m_bytes, m_rest.constData(), m_rest.length());
      CalcNextHash(data.m_int);
   }
   CalcNextHash(m_sumLength);
   rc = m_hash;
   reset();
   return rc;
}

/**
 * Returns the hash value as byte array.
 *
 * Note: After call of this method the hash is reset!
 *
 * @return	the hash value as byte array, stored in "little endian" byte order
 */
QByteArray ReHmHash64::digest() {
   QByteArray rc;
   rc.resize(sizeof m_hash);
   int64_converter_t data;
   data.m_int = digestAsInt();
   data.toBytes(reinterpret_cast<uint8_t*>(rc.data()));
   return rc;
}

/**
 * Resets the hash.
 *
 * The state is equal after the constructor.
 */
void ReHmHash64::reset() {
   m_hash = m_sumLength = 0;
   m_rest.clear();
}

/**
 * Adds the content of a block to the hash.
 *
 *
 * @param source	the data block which will be added to the hash
 * @param length	the length of the data (in bytes)
 */
void ReHmHash64::update(const void* source, size_t length) {
   const uint8_t* src = reinterpret_cast<const uint8_t*>(source);
   int64_converter_t data;
   m_sumLength += length;
   if (m_rest.length() > 0) {
      size_t needed = sizeof m_hash - m_rest.length();
      if (needed > length) {
         m_rest.append(reinterpret_cast<const char*>(src), length);
         length = 0;
      } else {
         m_rest.append(reinterpret_cast<const char*>(src), needed);
         length -= needed;
         src += needed;
         data.fromBytes(reinterpret_cast<const uint8_t*>(m_rest.constData()));
         CalcNextHash(data.m_int);
         m_rest.clear();
      }
   }
   for (int ix = length / sizeof m_hash - 1; ix >= 0; ix--) {
      data.fromBytes(src);
      src += sizeof m_hash;
      CalcNextHash(data.m_int);
   }
   int rest = length % sizeof m_hash;
   if (rest > 0) {
      m_rest.append(reinterpret_cast<const char*>(src), rest);
   }
}

/**
 * @brief Constructor.
 *
 * @param name	the name of the generator
 */
ReRandomizer::ReRandomizer(const char* name) :
   m_name(name),
   m_counter(0) {
}
/**
 * @brief Destructor.
 */
ReRandomizer::~ReRandomizer() {
}

#if defined __linux__
inline int abs(int x) {
   return x < 0 ? -x : x;
}
inline int64_t abs(int64_t x) {
   return x < 0 ? -x : x;
}

#endif

/**
 * Builds a number from a text by hasing.
 *
 * Conditions:
 * <ul><li>It should be difficult to restore the text from the hash.</li>
 * <li>Same text must return the same hash value.</li>
 * <li>All bits should be set from any text (except the empty text)</li>
 * </ul>
 *
 * @param text	the text to hash
 * @return		a number
 */
ReRandomizer::seed_t ReRandomizer::hash(const QByteArray& text) {
   seed_t rc = 0x200a110b190c580dLL;
   int ixPrimes = m_countPrimes / 2;
   rc *= text.at(0);
   for (int ix = text.length() - 1; ix > 0; ix--) {
      rc = rc * m_primes64[ixPrimes] * text.at(ix) * m_primes64[ixPrimes - 1];
      if ((ixPrimes -= 2) <= 0)
         ixPrimes = m_countPrimes;
   }
   return rc;
}

/**
 * Converts a text into a seed
 * @param text
 * @param seed
 */
void ReRandomizer::hash(const QByteArray& text, QByteArray& seed) {
   int maxTrg = seed.length() / sizeof(seed_t) - 1;
   seed_t* trg = reinterpret_cast<seed_t*>(seed.data());
   seed_t last = 0x1b20a811cc19f258LL;
   int textLength = text.length();
   for (int ix = maxTrg; ix >= 0; ix--) {
      last = trg[ix] = last * text.at(ix % textLength)
                       + m_primes64[ix % m_countPrimes] * text.at((ix + 1) % textLength);
   }
   // length password > length seed:
   for (int ix = maxTrg; ix < textLength; ix++) {
      last = trg[ix % maxTrg] ^= last * text.at(ix)
                                 + m_primes64[ix % m_countPrimes];
   }
   // length password < length seed:
   for (int ix = textLength; ix <= maxTrg; ix++) {
      last = trg[ix] = last * text.at(ix % textLength)
                       + m_primes64[ix % m_countPrimes];
   }
   // mix of all seed entries:
   for (int ix = 0; ix <= maxTrg; ix++)
      last = trg[ix] ^= last * trg[maxTrg - ix];
}

/**
 * Returns the name of the generator.
 *
 * @return	the name
 */
const QByteArray& ReRandomizer::name() const {
   return m_name;
}

/**
 * Produces a random number which is very difficult to find out.
 *
 * @return	a random value not easy to find out
 */
ReRandomizer::seed_t ReRandomizer::pseudoTrueRandom() {
   clock_t random = clock();
   time_t random2 = time(NULL);
   static int s_counter = 0;
   void* dummy2 = malloc(1);
   free(dummy2);
   seed_t rc = (((seed_t) random2) << 31) + (seed_t) random
               + ((seed_t) &s_counter << 9)
               + ((-random ^ 0x20111958) ^ (seed_t(dummy2)));
   rc = (rc * m_primes64[int(rc % m_countPrimes)]
         + (m_primes64[int((rc >> 13) % m_countPrimes)] >> 1))
        ^ m_primes64[++s_counter % m_countPrimes];
   rc = (rc << 56) | (uint64_t(rc) >> (64 - 56));
   return rc;
}

/**
 * @brief Returns a random number which is not predictable.
 *
 * @return a number which is not predictable
 */
ReRandomizer::seed_t ReRandomizer::nearTrueRandom() {
   seed_t rc = pseudoTrueRandom();
#if defined __linux__
   int fh = open("/dev/urandom", O_RDONLY);
   seed_t buffer;
   if (read(fh, reinterpret_cast<char*>(&buffer), sizeof buffer) > 0)
      rc ^= buffer;
   close(fh);
#endif
   return rc;
}

/**
 * @brief Returns the next random character.
 *
 * The character is in the range ' ' .. chr(127) (inclusive).
 *
 * @return The next random character.
 */
char ReRandomizer::nextChar() {
   char rc = nextInt(' ' + CHARRANGE - 1, ' ');
   return rc;
}

/**
 * @brief Returns binary data with random bytes.
 *
 * @param minLength	The minimum length of the result data
 * @param maxLength	The maximum length of the result data
 * @param buffer	OUT: The place for the data
 *
 * @return			<code>buffer</code> (for chaining)
 */
QByteArray& ReRandomizer::nextData(int minLength, int maxLength,
                                   QByteArray& buffer) {
   int len = nextInt(maxLength, minLength);
   buffer.resize(len);
   seed_t data;
   seed_t* ptr = reinterpret_cast<seed_t*>(buffer.data());
   for (int ix = len / sizeof data - 1; ix >= 0; ix--)
      *ptr = nextSeed64();
   int rest = len % sizeof data;
   if (rest != 0) {
      data = nextSeed64();
      uint8_t* ptr2 = reinterpret_cast<uint8_t*>(ptr);
      while(rest-- > 0) {
         *ptr2++ = data;
         data >>= 8;
      }
   }
   return buffer;
}

/**
 * @brief Returns the next random integer.
 *
 *
 * @param maxValue	The maximum of the result (including).
 * @param minValue	The minimum of the result.
 *
 * @return The next seed.
 */
int ReRandomizer::nextInt(int maxValue, int minValue) {
   int rc;
   if (minValue > maxValue) {
      rc = minValue;
      minValue = maxValue;
      maxValue = rc;
   }
   seed_t seed = nextSeed64();
   if (minValue >= 0)
      seed = abs(seed);
   if (minValue == maxValue)
      rc = minValue;
   else {
      // we calculate in 64 bit, no range overflow:
      int64_t range = (int64_t) maxValue - minValue + 1;
      int64_t offset = seed % range;
      rc = (int) (minValue + offset);
   }
   return rc;
}

/**
 * @brief Returns the next random integer.
 *
 *
 * @param maxValue	The maximum of the result (including).
 * @param minValue	The minimum of the result.
 *
 * @return The next seed.
 */
int64_t ReRandomizer::nextInt64(int64_t maxValue, int64_t minValue) {
   seed_t rc;
   if (minValue > maxValue) {
      rc = minValue;
      minValue = maxValue;
      maxValue = rc;
   }
   seed_t seed = nextSeed64();
   if (minValue == maxValue)
      rc = minValue;
   else if (minValue == 0 && maxValue == LLONG_MAX)
      rc = abs((int64_t) seed);
   else if (uint64_t(maxValue - minValue) < LLONG_MAX)
      // no signed int64 overflow:
      rc = minValue + seed % (maxValue - minValue + 1);
   else {
      // int64 overflow: we need a higher precision:
      double rc2 = (double) minValue
                   + fmod((double) seed, (double) (maxValue - minValue));
      rc = (int) rc2;
   }
   return rc;
}

/**
 * @brief Returns a string with random characters.
 *
 * All character will be inside the range ' ' .. chr(127).
 *
 * @param minLength	The minimum length of the result string.
 * @param maxLength	The maximum length of the result string.
 * @param buffer	Out: The place for the string.
 *
 * @result The buffer.
 */
const char* ReRandomizer::nextString(int minLength, int maxLength,
                                     QByteArray& buffer) {
   int len = nextInt(maxLength, minLength);
   buffer.resize(len);
   char* ptr = buffer.data();
   for (int ii = 0; ii < len; ii++) {
      ptr[ii] = nextChar();
   }
   return buffer.constData();
}

/**
 * Convert a clear text into scrambled data and vice versa.
 *
 * <ul><li>The algorithm is symetric: the same method can be used for encoding and
 * decoding.</li>
 * <li>the result is the same for little and big endian architectures.</li>
 * <li>Large data blocks can be de/encoded in parts if the parts have the length
 * N*8 (multiples of 8 bytes).</li>
 * </ul>
 *
 * @param target	OUT: the target buffer
 * @param source	the source to scramble. May be identical <code>target</code>
 * @param length	the length of <code>source</code> in byte
 */
void ReRandomizer::codec(uint8_t* target, const uint8_t* source , int length) {
   int64_converter_t data;
   for (int ii = length / sizeof data - 1; ii >= 0; ii--) {
      data.fromBytes(source);
      data.m_int ^= nextSeed64();
      data.toBytes(target);
      source += sizeof data;
      target += sizeof data;
   }
   int rest = length % sizeof(seed_t);
   if (rest > 0) {
      data.m_int = nextSeed64();
      for (int ii = 0; ii < rest; ii++) {
         *target++ = *source++ ^ data.m_bytes[ii];
      }
   }
}

/**
 * @brief Builds a random permutation of an array.
 *
 * The operation will be done in place:
 *
 * @param array		IN/OUT: the array to shuffle
 * @param length	the length of the array
 * @param elemSize	the size of one element of the array
 * @param exchanges	count of exchanges: two randomly selected entries will be
 *					exchanged. If <= 0 a default value will be taken: 1.5*length
 */
void ReRandomizer::shuffle(void* array, size_t length, size_t elemSize,
                           int exchanges) {
   int ii;
   char* cptr;
   int* iptr;
   int64_t* i64ptr;
   int count = exchanges <= 0 ? length * 3 / 2 : exchanges;
   int maxIx = length - 1;
   switch (elemSize) {
   case 1:
      cptr = (char*) array;
      for (ii = 0; ii < count; ii++) {
         int ix1 =nextInt(maxIx);
         int ix2 =nextInt(maxIx);
         char x = cptr[ix1];
         cptr[ix1] = cptr[ix2];
         cptr[ix2] = x;
      }
      break;
   case sizeof(int):
      iptr = (int*) array;
      for (ii = 0; ii < count; ii++) {
         int ix1 =nextInt(maxIx);
         int ix2 =nextInt(maxIx);
         int x = iptr[ix1];
         iptr[ix1] = iptr[ix2];
         iptr[ix2] = x;
      }
      break;
   case sizeof(int64_t):
      i64ptr = (int64_t*) array;
      for (ii = 0; ii < count; ii++) {
         int ix1 = nextInt(maxIx);
         int ix2 = nextInt(maxIx);
         if (i64ptr[ix1] <= 0 || i64ptr[ix2] <= 0)
            ix1 += 0;
         int64_t x = i64ptr[ix1];
         i64ptr[ix1] = i64ptr[ix2];
         i64ptr[ix2] = x;
      }
      break;
   default: {
      char buffer[0x10000];
      assert(elemSize < sizeof buffer);
      iptr = (int*) array;
      for (ii = 0; ii < count; ii++) {
         int ix1 =nextInt(maxIx);
         int ix2 =nextInt(maxIx);
         char* p1 = ((char*) array) + ix1 * elemSize;
         char* p2 = ((char*) array) + ix2 * elemSize;
         memcpy(buffer, p1, elemSize);
         memcpy(p1, p2, elemSize);
         memcpy(p2, buffer, elemSize);
      }
      break;
   }
   }
}
/**
 * @brief Sets the seed to the value given by the last <code>setSeed()</code>.
 *
 * Note: The constructor does the first <code>setSeed()</code>.
 */
/**
 * Constructor.
 *
 * @param name	the name of the generator
 */
ReSingleSeedRandomizer::ReSingleSeedRandomizer(const char* name) :
   ReRandomizer(name),
   m_seed(0x1120120419198991ull),
   m_lastSetSeed(0x1120120419198991ull) {
}

/**
 * Modifies the current seed with a 64-bit value.
 *
 * @param seed		the value to modify the current seed
 */
void ReSingleSeedRandomizer::modifySeed(int64_t seed) {
   m_seed += seed;
}

/**
 * Sets the seed to the start point (defined with setSeed()).
 */
void ReSingleSeedRandomizer::reset() {
   m_seed = m_lastSetSeed;
   m_counter = 0;
}

/**
 * Sets the current point of pseudo random.
 *
 * @param seed	the current point of pseudo random
 */
void ReSingleSeedRandomizer::restoreSeed(const QByteArray& seed) {
   if (seed.size() >= (int) sizeof(seed_t)) {
      memcpy(&m_seed, seed.constData(), sizeof m_seed);
   } else {
      QByteArray buffer = seed;
      buffer.resize(sizeof m_seed);
      memset(buffer.data() + seed.size(), 0, sizeof(m_seed) - seed.size());
      memcpy(&m_seed, buffer.constData(), sizeof m_seed);
   }
   m_lastSetSeed = m_seed;
}

/**
 * Gets the current point of pseudo random.
 *
 * @param seed	the current point of pseudo random
 */
void ReSingleSeedRandomizer::saveSeed(QByteArray& seed) const {
   seed.resize(sizeof m_seed);
   memcpy(seed.data(), &m_seed, sizeof m_seed);
}

/**
 * Returns the current seed.
 *
 * @return The current seed.
 */
ReRandomizer::seed_t ReSingleSeedRandomizer::seed() const {
   return m_seed;
}

/** @brief Sets the current seed.
 *
 * @param seed	The seed to set.
 */
void ReSingleSeedRandomizer::setSeed(seed_t seed) {
   m_seed = m_lastSetSeed = seed;
   TRACE1("setSeed: %llx\n", (long long) seed);
}

/**
 * Converts a text (e.g. password) into the generator specific seed.
 *
 * @param text	the text to convert
 */
void ReSingleSeedRandomizer::textToSeed(const QByteArray& text) {
   setSeed(hash(text));
}

/**
 * Constructor.
 */
ReNullRandomizer::ReNullRandomizer() :
   ReSingleSeedRandomizer("dummy") {
}

/**
 * Modifies the seed.
 * @param seed	the modifier
 */
void ReNullRandomizer::modifySeed(int64_t seed) {
   ReUseParameter(seed);
   // nothing to do!
}

/**
 * Returns the next seed.
 * @return	the next seed: 0L
 */
ReRandomizer::seed_t ReNullRandomizer::nextSeed64() {
   return 0;
}
/**
 * @brief Constructor.
 */
ReCongruentialGeneratorBase::ReCongruentialGeneratorBase() :
   m_factor(0x79009fb8d8e7538bL),
   m_increment(7809106856336952853L) {
}

/**
 * @brief Returns the current factor.
 *
 * @return The current factor.
 */
ReRandomizer::seed_t ReCongruentialGeneratorBase::factor() const {
   return m_factor;
}

/**
 * @brief Returns the current increment.
 *
 * @return The current increment.
 */
ReRandomizer::seed_t ReCongruentialGeneratorBase::increment() const {
   return m_increment;
}

/**
 * @brief Calculates the next seed for the generator.
 *
 * @return The next seed.
 */
ReRandomizer::seed_t ReCongruentialGenerator::nextSeed64() {
   m_seed = m_seed * m_factor + m_increment;
   ++m_counter;
   return m_seed;
}

/**
 * Sets the factor.
 *
 * @param factor	The new factor.
 */
void ReCongruentialGeneratorBase::setFactor(ReRandomizer::seed_t factor) {
   this->m_factor = factor;
}

/**
 * Sets the increment.
 *
 * @param increment	The new increment.
 */
void ReCongruentialGeneratorBase::setIncrement(ReRandomizer::seed_t increment) {
   this->m_increment = increment;
}

/**
 * @brief Constructor.
 */
ReCongruentialGenerator::ReCongruentialGenerator() :
   ReSingleSeedRandomizer("LCG"),
   ReCongruentialGeneratorBase() {
}

/**
 * @brief Constructor.
 */
ReCongruentialGenerator::ReCongruentialGenerator(const char* name) :
   ReSingleSeedRandomizer(name),
   ReCongruentialGeneratorBase() {
}

/**
 * Constructor.
 */
ReRotateRandomizer::ReRotateRandomizer() :
   ReCongruentialGenerator("Rotating LCG") {
}

/** @brief Returns the next 64 bit pseudo random number.
 *
 * A congruential generator produces good random in the most significant
 * bits. Therefore we exchange the bits of the result.
 * Then x % m returns better results.
 *
 * @return a pseudo random number
 */

ReRandomizer::seed_t ReRotateRandomizer::nextSeed64() {
   seed_t rc = ReCongruentialGenerator::nextSeed64();
   rc = ((rc << 33) | (uint64_t(rc) >> 31));
   ++m_counter;
   return rc;
}

/**
 * Constructor.
 */

ReMultiSeedRandomizer::ReMultiSeedRandomizer(int countSeeds, const char* name) :
   ReRandomizer(name),
   m_countSeeds(min(256, max(countSeeds, 2))),
   m_seedBuffer(),
   m_startSeed(),
   m_seeds() {
   m_seedBuffer.resize(m_countSeeds * sizeof m_seeds[0]);
   m_seeds = reinterpret_cast<seed_t*>(m_seedBuffer.data());
   for (int ix = 0; ix < m_countSeeds; ix++) {
      int ixPrimes = ix % m_countPrimes;
      m_seeds[ix] = m_primes64[ixPrimes] * (2 * ix + 1)
                    + (seed_t(m_primes64[m_countPrimes - 1 - ixPrimes]) << 32);
   }
   // assignment does not work: copy on write
   m_startSeed.resize(m_seedBuffer.length());
   memcpy(m_startSeed.data(), m_seedBuffer.constData(), m_startSeed.length());
}

/** @brief Returns the next 64 bit pseudo random number.
 *
 * @return a pseudo random number
 */
ReRandomizer::seed_t ReMultiCongruentialGenerator::nextSeed64() {
   m_currentSeed = (m_currentSeed + 1) % m_countSeeds;
   int ixFactor = int((m_seeds[m_currentSeed] >> 7) % (m_countPrimes - 1));
   seed_t rc = m_seeds[m_currentSeed] * m_primes64[ixFactor]
               + (m_primes64[ixFactor + 1] >> 1);
   m_seeds[m_currentSeed] = rc;
   rc = (rc << 33) | (uint64_t(rc) >> (64-33));
   ++m_counter;
   return rc;
}

/**
 * Modifies the current seed with a 64-bit value.
 *
 * @param seed		the value to modify the current seed
 */
void ReMultiSeedRandomizer::modifySeed(int64_t seed) {
   for (int ix = 0; ix < m_countSeeds; ix++) {
      m_seeds[ix] += seed;
      seed = ((seed << 1) | (seed < 0 ? 1 : 0));
   }
}
/**
 * Sets the instance to a defined start state.
 */
void ReMultiSeedRandomizer::reset() {
   // assignment does not work: copy on write
   memcpy(m_seedBuffer.data(), m_startSeed.constData(), m_seedBuffer.length());
   m_counter = 0;
}

/**
 * Sets the current point of pseudo random with a seed saved by <code>saveSeed()</code>.
 *
 * @param seed	the current point of pseudo random
 */
void ReMultiSeedRandomizer::restoreSeed(const QByteArray& seed) {
   size_t length = min(seed.length(), m_seedBuffer.length());
   memcpy(m_seedBuffer.data(), seed.constData(), length);
}

/**
 * Stores the the current point of pseudo random in a buffer.
 *
 * Restoring is done by <code>restoreSeed()</code>
 *
 * @param seed	the current point of pseudo random
 */
void ReMultiSeedRandomizer::saveSeed(QByteArray& seed) const {
   size_t size = m_seedBuffer.length();
   seed.resize(size);
   memcpy(seed.data(), m_seedBuffer.constData(), size);
}

/**
 * Converts a text (e.g. password) into the generator specific seed.
 *
 * @param text	the text to convert
 */
void ReMultiSeedRandomizer::textToSeed(const QByteArray& text) {
   hash(text, m_seedBuffer);
   // assignment does not work: copy on write
   memcpy(m_startSeed.data(), m_seedBuffer.constData(), m_seedBuffer.length());
}

/**
 * Constructor.
 *
 * @param countSeeds	number of seeds
 */
ReMultiCongruentialGenerator::ReMultiCongruentialGenerator(int countSeeds) :
   ReMultiSeedRandomizer(countSeeds, "Multi LCG"),
   ReCongruentialGeneratorBase(),
   m_currentSeed(-1) {
}

/**
 * Sets the seed to the start point (defined with setSeed()).
 */
void ReMultiCongruentialGenerator::reset() {
   ReMultiSeedRandomizer::reset();
   m_currentSeed = -1;
   m_counter = 0;
}

/**
 * Constructor.
 *
 * @return
 */
ReXorShift64Randomizer::ReXorShift64Randomizer() :
   ReSingleSeedRandomizer("ShiftXor64") {
}

/**
 * Calculates the next pseudo random value.
 *
 * @return the next pseudo random value
 */
ReRandomizer::seed_t ReXorShift64Randomizer::nextSeed64() {
   m_seed ^= m_seed << 13;
   m_seed ^= m_seed >> 7;
   m_seed ^= m_seed << 17;
   ++m_counter;
   return m_seed;
}

/**
 * Constructor.
 */
ReKISSRandomizer::ReKISSRandomizer() :
   ReRandomizer("KISS"),
   m_params(),
   m_startParams() {
   m_params.m_x = 0x1234567887654321ULL;
   m_params.m_y = 0x3624363624363636ULL;
   m_params.m_z = 0x2011195811081965ULL;
   m_params.m_c = 0x2004199111121989ULL;
   m_startParams = m_params;
}

/**
 * Dumps the state of the generator.
 */
void ReKISSRandomizer::dump() {
   printf("%s\n", state().constData());
   printf(
      "    f: %016llx i: %016llx: c: %016llx x: %016llx y: %016llx z: %016llx\n",
      (long long) m_factor, (long long) m_increment,
      (long long) m_startParams.m_c, (long long) m_startParams.m_x,
      (long long) m_startParams.m_y, (long long) m_startParams.m_z);
}

/**
 * Calculates the next pseudo random value.
 * @return
 */
ReRandomizer::seed_t ReKISSRandomizer::nextSeed64() {
   seed_t t;
   // Linear congruence generator
   m_params.m_z = m_factor * m_params.m_z + m_increment;
   // Xorshift
   m_params.m_y ^= (m_params.m_y << 13);
   m_params.m_y ^= (m_params.m_y >> 17);
   m_params.m_y ^= (m_params.m_y << 43);
   // Multiply-with-carry
   t = (m_params.m_x << 58) + m_params.m_c;
   m_params.m_c = (m_params.m_x >> 6);
   m_params.m_x += t;
   m_params.m_c += (m_params.m_x < t);
   ++m_counter;
   seed_t rc = m_params.m_x + m_params.m_y + m_params.m_z;
   TRACE2("%3d: %llx\n", m_counter, (long long) rc);
   return rc;
}
/**
 * Modifies the current seed with a 64-bit value.
 *
 * @param seed		the value to modify the current seed
 */
void ReKISSRandomizer::modifySeed(int64_t seed) {
   m_params.m_x ^= seed;
   TRACE1("modifySeed() %s\n", state().constData());
}
/**
 * Sets the seed to the start point (defined with setSeed()).
 */
void ReKISSRandomizer::reset() {
   m_params = m_startParams;
   m_counter = 0;
   TRACE1("reset() %s\n", state().constData());
}

/**
 * Sets the current point of pseudo random with a seed saved by <code>saveSeed()</code>.
 *
 * @param seed	the current point of pseudo random
 */
void ReKISSRandomizer::restoreSeed(const QByteArray& seed) {
   restore(seed, m_params);
}

/**
 * Stores the the current point of pseudo random in a buffer.
 *
 * Restoring is done by <code>restoreSeed()</code>
 *
 * @param seed	the current point of pseudo random
 */
void ReKISSRandomizer::saveSeed(QByteArray& seed) const {
   save(m_params, seed);
}

/**
 * Returns the internal state as string.
 *
 * @return	the internal state
 */
QByteArray ReKISSRandomizer::state() const {
   char buffer[512];
   snprintf(buffer, sizeof buffer,
            "%2d: f: %016llx i: %016llx: c: %016llx x: %016llx y: %016llx z: %016llx",
            m_counter,
            (long long) m_factor, (long long) m_increment,
            (long long) m_params.m_c, (long long) m_params.m_x,
            (long long)  m_params.m_y, (long long) m_params.m_z);
   return QByteArray(buffer);
}

/**
 * Converts a text (e.g. password) into the generator specific seed.
 *
 * @param text	the text to convert
 */
void ReKISSRandomizer::textToSeed(const QByteArray& text) {
   QByteArray seeds;
   seeds.resize(sizeof m_params);
   hash(text, seeds);
   restore(seeds, m_params);
   m_startParams = m_params;
}

/**
 * Constructor.
 *
 * @param contentRandom	a pseudo random generator
 * @param logger		the logger
 */
ReByteScrambler::ReByteScrambler(ReRandomizer& contentRandom, ReLogger* logger) :
   m_contentRandom(contentRandom),
   m_contentSeed(),
   m_buffer(),
   m_header(),
   m_logger(logger),
   m_salt() {
   m_salt.m_int = 0;
   m_contentRandom.saveSeed(m_contentSeed);
   m_buffer.reserve(256);
}

/**
 * Copy constructor.
 *
 * @param source	the source to copy
 */
ReByteScrambler::ReByteScrambler(const ReByteScrambler& source) :
   m_contentRandom(source.m_contentRandom),
   m_contentSeed(source.m_contentSeed),
   m_buffer(source.m_buffer),
   m_header(source.m_header),
   m_logger(source.m_logger),
   m_salt(source.m_salt) {
}

/**
 * Asignment operator.
 *
 * @param source	source to copy
 * @return			the instance
 */
ReByteScrambler& ReByteScrambler::operator =(const ReByteScrambler& source) {
   m_contentRandom = source.m_contentRandom;
   m_contentSeed = source.m_contentSeed;
   m_buffer = source.m_buffer;
   m_header = source.m_header;
   m_logger = source.m_logger;
   m_salt = source.m_salt;
   return *this;
}

/**
 * Returns the pseudo random generator used for data encryption.
 *
 * @param doReset	<code>true</code>: the random generator is reset
 * @return			the random generator for data encryption
 */
ReRandomizer& ReByteScrambler::contentRandom(bool doReset) {
   if (doReset) {
      randomReset();
   }
   return m_contentRandom;
}

/**
 * Initializes the scrambler from a header.
 *
 * @param reservedLength	length of an area in front of the marker (not encrypted)
 * @param markerLength		length of a mark to ensure header's integrity
 * @param infoLength		the length of the "reserved area"
 * @param encryptedFrom		the first index from which data are encrypted.
 *							the first possible index is behind the marker
 * @param header			OUT: the header with the initializing info. If NULL the
 *							internal header will be used.
 *							Otherwise the parts of the header will be decrypted
 * @param info				OUT: the reserved area in the header
 * @return					<code>true</code>: success
 */
bool ReByteScrambler::initFromHeader(int reservedLength, int markerLength,
                                     int infoLength, int encryptedFrom, QByteArray* header,
                                     QByteArray& info) {
   TRACE("initFromHeader():\n");
   encryptedFrom = max(encryptedFrom, (int) sizeof(int64_t)
                       + reservedLength + markerLength);
   bool rc = true;
   if (header == NULL)
      header = &m_header;
   int headerLength = sizeof(int64_t) + reservedLength + markerLength + infoLength;
   if (header->length() < headerLength ) {
      m_logger->logv(LOG_ERROR, LOC_DECODE_CONTENT_1, "header length too small: %d/%d",
                     header->length(), headerLength);
      rc = false;
   } else {
      m_salt.fromBytes(reinterpret_cast<const uint8_t*>(header->constData()));
      randomReset();
      TRACE2("salt: %08lx / %s\n", m_salt.m_int, hexBytes(m_salt.m_bytes, 8).constData());
      QByteArray expectedMarker;
      if (markerLength > 0)
         m_contentRandom.nextString(markerLength, markerLength, expectedMarker);
      if (encryptedFrom < header->length()) {
         randomReset();
         uint8_t* start = reinterpret_cast<uint8_t*>(header->data()) + encryptedFrom;
         TRACE1("info before decoding: %s\n", hexBytes(start, 16).constData());
         m_contentRandom.codec(start, start, header->length() - encryptedFrom);
         TRACE1("info after decoding: %s\n", hexBytes(start, 16).constData());
      }
      if (markerLength > 0) {
         QByteArray marker;
         marker = header->mid(sizeof(int64_t) + reservedLength,
                              markerLength);
         TRACE1("marker: %s\n", marker.constData());
         if (marker != expectedMarker) {
            m_logger->logv(LOG_ERROR, LOC_DECODE_CONTENT_2,
                           "invalid marker: %s / %s",
                           ReStringUtils::hexDump(marker, markerLength,
                                                  markerLength).constData(),
                           ReStringUtils::hexDump(expectedMarker,
                                                  markerLength, markerLength).constData());
            rc = false;
         }
      }
      info.resize(0);
      if (infoLength > 0)
         info = header->mid(sizeof(int64_t) + reservedLength + markerLength,
                            infoLength);
   }
   IF_TRACE(m_contentRandom.dump());
   return rc;
}

/**
 * Initializes the scrambler header.
 *
 * Format of the header:
 * <pre>random (8 bytes)
 * unencrypted area (unencryptedSpace bytes)
 * marker (m_markerLength bytes)
 * reserved (reservedLength bytes)
 * </pre>
 * @param reservedLength	the length of the not encrypted area behind the random
 * @param markerLength		length of a mark to ensure header's integrity
 * @param infoLength		the length of the "reserved area"
 * @param encryptedFrom		the first index from which data are encrypted.
 *							the first possible index is behind the marker
 * @param info				the content of the "reserved area". If too few space
 *							the content will be cut
 */
void ReByteScrambler::initHeader(int reservedLength, int markerLength,
                                 int infoLength, int encryptedFrom, const QByteArray& info) {
   TRACE("initHeader():\n");
   encryptedFrom = max(encryptedFrom,
                       (int) sizeof(int64_t) + reservedLength + markerLength);
   m_salt.m_int = ReRandomizer::nearTrueRandom();
   m_contentRandom.reset();
   m_contentRandom.modifySeed(m_salt.m_int);
   int headerLength = sizeof(int64_t) + reservedLength + markerLength + infoLength;
   m_header.fill(' ', headerLength);
   m_salt.toBytes(reinterpret_cast<uint8_t*>(m_header.data()));
   TRACE2("salt: %08lx %s\n", m_salt.m_int, hexBytes(m_header.constData(), 8).constData());
   if (markerLength > 0) {
      m_buffer.resize(0);
      m_contentRandom.nextString(markerLength, markerLength, m_buffer);
      memcpy(m_header.data() + sizeof(int64_t) + reservedLength,
             m_buffer, markerLength);
      TRACE1("marker: %s\n", m_buffer.constData());
   }
   char* trg;
   if (info.length() > 0) {
      int offset = sizeof(int64_t) + reservedLength + markerLength;
      trg = reinterpret_cast<char*>(m_header.data() + offset);
      memcpy(trg, info, min(m_header.length() - offset, info.length()));
   }
   if (encryptedFrom < m_header.length()) {
      randomReset();
      uint8_t* start = reinterpret_cast<uint8_t*>(m_header.data() + encryptedFrom);
      TRACE1("info before encoding: %s\n", hexBytes(start, 16).constData());
      m_contentRandom.codec(start, start, m_header.length() - encryptedFrom);
      TRACE1("info after encoding: %s\n", hexBytes(start, 16).constData());
   }
   TRACE1("header: %s\n", hexBytes(m_header.constData(), 16).constData());
}

/**
 * Resets the pseudo random generator to the scrambler specific state (salt).
 */
void ReByteScrambler::randomReset() {
   m_contentRandom.reset();
   m_contentRandom.modifySeed(m_salt.m_int);
}

/**
 * Returns the internal stored header.
 * @return	the internal header
 */
QByteArray& ReByteScrambler::header() {
   return m_header;
}


