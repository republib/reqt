/*
 * ReEnigma.cpp
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
 * @brief Implements encryption and decryption engines.
 */
/** @file math/ReEnigma.hpp
 *
 * @brief Definitions for encryption and decryption engines.
 */

#include "base/rebase.hpp"
#include "math/remath.hpp"

/** @class ReEnigma::secret_t ReEnigma.hpp "math/ReEnigma.hpp"
 *
 * @brief Stores the internal structure of a <b>secret</b>.
 *
 * A secret can be a password, a salt, a certificate or simlar
 * which makes an encryption individually.
 */
/** @class ReEnigma ReEnigma.hpp "math/ReEnigma.hpp"
 *
 * @brief Implements a portable en/decryption engine.
 *
 * It is used a symetric encryption.
 * Therefore the encrypt methods can be used for decryption too.
 *
 * The encryption can be unique by using certificates.
 * A certificate is a series of bytes affecting the pseudo random sequence.
 *
 * More than one certificate can be used.
 *
 * The encryption is done with an pseudo random generator.
 *
 * The portability (over byte order) is guaranteed if the
 * pseudo random generator is portable (nomalizing big/little endian)
 * and a 64 bit integer arithmetic is available.
 *
 */
const char* ReEnigma::SET_DECIMALS = "0123456789";
const char* ReEnigma::SET_HEXDIGITS = "0123456789abcdef";
const char* ReEnigma::SET_ALPHANUM = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz_";
const char* ReEnigma::SET_FILENAME = " !^°$%&=+~#-.0123456789ABCDEFGHIJKLM"
                                     "NOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";
const char* ReEnigma::SET_32_127 = " !\"#$%&'()*+,-./0123456789:;<=>?@"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f";
const char* ReEnigma::SET_32_255 =
   " !\"#$%&'()*+,-./0123456789:;<=>?@"
   "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f"
   "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
   "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
   "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
   "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
const char* ReEnigma::SET_PRINTABLE_127 =
   "\t\r\n !\"#$%&'()*+,-./0123456789:;<=>?@"
   "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f";
const char* ReEnigma::SET_PRINTABLE_255 =
   "\t\r\n !\"#$%&'()*+,-./0123456789:;<=>?@"
   "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f"
   "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
   "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
   "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
   "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";

/**
 * @brief Constructor.
 *
 * @param random    pseudo random generator
 */
ReEnigma::ReEnigma(ReRandom* random) :
   m_random(random),
   m_ownsRandom(false),
   m_secrets(),
   m_randomCalls(0),
   m_randomSource("4711") {
   m_randomSource.reserve(8096);
   if (random == NULL) {
      m_random = new ReRandom();
      m_ownsRandom = true;
   }
}
/**
 * @brief Destructor.
 */
ReEnigma::~ReEnigma() {
   if (m_ownsRandom) {
      delete m_random;
      m_random = NULL;
   }
}

/**
 * @brief Reads a certificate.
 *
 * @param filename      the name of the certificate
 *
 * @return              empty string: error while reading<br>
 *                      otherwise: the certificate as byte array
 */
QByteArray ReEnigma::readCertificate(const char* filename) {
   QByteArray rc = "not implemented: readCertificate(): ";
   rc += filename;
   assert(rc.isEmpty());
   return rc;
}

inline void buildBooster(QByteArray& booster, const char* charSet) {
   int size = 257;
   booster.fill(0, size);
   int ix = 0;
   unsigned char cc;
   while ((cc = (unsigned char) *charSet++) != '\0') {
      booster[cc] = ++ix;
   }
   booster[0] = ix;
}

/**
 * @brief Encodes a string in place with a given character set.
 *
 * If a character of data is not included in the character set
 * it remains unchanged
 *
 * @param data      IN: data to encode<br>
 *                  OUT: data encoded
 * @param size      length of <code>data</code>. If 0
 * @param charSet   a string containing all valid characters
 * @param booster   a performance booster. Once built it can be shared
 *                  between all calls of encodeChar() and decodeChar()
 *                  with the same character set<br>
 *                  IN: "": not initialized otherwise: ready for work
 *                  OUT: ready for work
 */
void ReEnigma::encode(char* data, int size, const char* charSet,
                      QByteArray& booster) {
   if (booster.length() == 0) {
      buildBooster(booster, charSet);
   }
   int lengthOfCharSet = (int) (unsigned char) booster.at(0);
   for (int ii = 0; ii < size; ii++) {
      unsigned char cc = (unsigned char) data[ii];
      int ix = booster.at(cc);
      if (ix != 0) {
         int next = nextInt(lengthOfCharSet);
         int ix2 = (ix - 1 + next) % lengthOfCharSet;
         data[ii] = charSet[ix2];
      }
   }
}

/**
 * @brief Decodes a string in place with a given character set.
 *
 * If a character of data is not included in the character set
 * it remains unchanged
 *
 * @param data      IN: data to decode<br>
 *                  OUT: data decoded
 * @param size      length of <code>data</code>. If 0
 * @param charSet   a string containing all valid characters
 * @param booster   a performance booster. Once built it can be shared
 *                  between all calls of encodeChar() and decodeChar()
 *                  with the same character set<br>
 *                  IN: "": not initialized otherwise: ready for work
 *                  OUT: ready for work
 */
void ReEnigma::decode(char* data, int size, const char* charSet,
                      QByteArray& booster) {
   if (booster.length() == 0) {
      buildBooster(booster, charSet);
   }
   int lengthOfCharSet = (int) (unsigned char) booster.at(0);
   for (int ii = 0; ii < size; ii++) {
      unsigned char cc = (unsigned char) data[ii];
      int ix = booster.at(cc);
      if (ix != 0) {
         int next = nextInt(lengthOfCharSet);
         int ix2 = (lengthOfCharSet + ix - 1 - next) % lengthOfCharSet;
         data[ii] = charSet[ix2];
      }
   }
}
/**
 * @brief Encodes or decode a byte array.
 *
 * The encoding method is symetric. Therefore it can encode and decode.
 *
 * @param data      IN: data to encode/decoded<br>
 *                  OUT: data encoded/decoded
 */
void ReEnigma::change(QByteArray& data) {
   int randomLength = m_randomSource.length();
   for (int ix = data.length() - 1; ix >= 0; ix--) {
      char item = data.at(ix);
      item = (item ^ nextInt(0xff) ^ m_randomSource.at(ix % randomLength));
      data[ix] = item;
   }
}

/**
 * @brief Adds a random source given by an byte array.
 *
 * A random source can be the binary form of an certificate,
 * a salt, a password or similar.
 *
 * @param byteSecret  a byte sequence which influences the random generation
 */
void ReEnigma::addByteSecret(QByteArray byteSecret) {
   // we expand it to a multiple of 64 bit:
   int oldSize = byteSecret.length();
   int newSize = (oldSize + 7) / 8 * 8;
   int ix;
   if (newSize > oldSize) {
      byteSecret.resize(newSize);
      int sum = 0;
      int start = oldSize > 8 ? oldSize - 8 : 0;
      for (ix = start; ix < oldSize; ix++) {
         sum += ix + byteSecret.at(ix);
      }
      for (ix = oldSize; ix < newSize; ix++) {
         sum += ix + 7;
         byteSecret[ix] = (char) (sum + byteSecret.at(ix - 1));
      }
   }
   int count = newSize / 8;
   secret_t* secret = new secret_t();
   secret->m_count = count;
   secret->m_list = new uint64_t[count];
   m_secrets.append(secret);
   for (ix = 0; ix < count; ix++) {
      uint64_t value = 0;
      for (int ix2 = 0; ix2 < 8; ix2++)
         value = (value << 8) + byteSecret.at(ix * 8 + ix2);
      secret->m_list[ix] = value;
   }
   QByteArray value;
   QCryptographicHash hash(QCryptographicHash::Md5);
   ReRandom rand;
   hash.addData(m_randomSource.constData(), 4);
   for (ix = 0; ix < byteSecret.length(); ix++) {
      hash.addData(byteSecret.constData() + ix, 1);
      QByteArray current = hash.result();
      int ix2 = rand.nextInt(0, m_randomSource.length() - 1);
      m_randomSource[ix2] = (m_randomSource.at(ix2) ^ current.at(0));
      m_randomSource.insert(0, current);
   }
}

/**
 * @brief Returns the next random integer
 * @param maxValue
 * @return
 */
int ReEnigma::nextInt(int maxValue) {
   uint64_t seed = 0;
   QList<secret_t*>::const_iterator it;
   int ixSecret = m_randomCalls++;
   int ix = ixSecret;
   for (it = m_secrets.constBegin(); it != m_secrets.constEnd(); ++it) {
      secret_t* secret = *it;
      seed |= ((secret->m_list[ixSecret % secret->m_count]) >> (ix % 8));
   }
   m_random->xorSeed(seed);
   int rc = m_random->nextInt(0, maxValue);
   return rc;
}

/**
 * @brief Sets the pseudo random to a define state.
 *
 * @param seed  the initial state
 */
void ReEnigma::setSeed(uint64_t seed) {
   m_random->setSeed(seed);
   m_randomCalls = 0;
}

