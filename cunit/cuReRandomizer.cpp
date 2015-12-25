/*
 * cuReRandomizer.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Unit test of the ReString tools.
 */

#include "../base/rebase.hpp"

class TestReRandomizer: public ReTest {
public:
   TestReRandomizer() :
      ReTest("ReRandomizer") {
      doIt();
   }

public:
   void testOne(ReRandomizer& rand) {
      QByteArray password("1");
      QByteArray seed1;
      QByteArray seed2;
      rand.saveSeed(seed1);
      char cc1 = rand.nextChar();
      rand.reset();
      rand.saveSeed(seed2);
      checkEqu(seed1, seed2);
      rand.textToSeed(password);
      rand.saveSeed(seed1);
      cc1 = rand.nextChar();
      rand.reset();
      rand.saveSeed(seed2);
      checkEqu(seed1, seed2);
      char cc2 = rand.nextChar();
      if (cc1 != cc1) {
         checkEqu(cc1, cc2);
      }
      rand.reset();
      int numbers[16];
      for (size_t ix = 0; ix < sizeof numbers / sizeof numbers[0]; ix++)
         numbers[ix] = rand.nextInt();
      rand.reset();
      rand.saveSeed(seed2);
      checkEqu(seed1, seed2);
      for (size_t ix = 0; ix < sizeof numbers / sizeof numbers[0]; ix++)
         checkEqu(numbers[ix], rand.nextInt());
      clock_t start = clock();
      for (int ix = 0; ix < 10 * 1000 * 1000; ix++) {
         rand.nextInt64();
      }
      double duration = double(clock() - start) / CLOCKS_PER_SEC;
      printf("%s (1E6): %.3f sec\n", rand.name().constData(), duration);
   }

   void testBasics() {
      ReKISSRandomizer rand3;
      testOne(rand3);
      ReCongruentialGenerator rand;
      testOne(rand);
      ReRotateRandomizer rand2;
      testOne(rand2);
      ReMultiCongruentialGenerator rand4(4);
      testOne(rand4);
      ReXorShift64Randomizer rand5;
      testOne(rand5);
   }
   void testOnePassw(const char* pw, ReRandomizer& rand) {
      QByteArray passw(pw);
      QByteArray seed;
      rand.textToSeed(passw);
      rand.saveSeed(seed);
      passw.append(":       ", 8 - passw.length());
      passw = rand.name() + ": " + passw;
      passw.append(seed.toHex());
      log(passw.constData());
   }
   void testPasswords(ReRandomizer& rand) {
      testOnePassw("a", rand);
      testOnePassw("b", rand);
      testOnePassw("aa", rand);
      testOnePassw("ab", rand);
      testOnePassw("aaa", rand);
      testOnePassw("aab", rand);
   }

   void testTextToSeed() {
      ReCongruentialGenerator rand;
      testPasswords(rand);
      ReXorShift64Randomizer rand2;
      testPasswords(rand2);
      ReMultiCongruentialGenerator rand3(2);
      testPasswords(rand3);
      ReKISSRandomizer rand4;
      testPasswords(rand4);
      log("ready");
   }
   void write1m(ReRandomizer& rand) {
      QByteArray fn = ReFileUtils::tempFile(rand.name().constData());
      fn.append(".data");
      QByteArray buffer;
      buffer.reserve(1000 * sizeof(int64_t));
      FILE* fp = fopen(fn.constData(), "w");
      checkNN(fp);
      if (fp != NULL) {
         for (int block = 0; block < 1000; block++) {
            buffer.clear();
            for (int ix = 0; ix < 1000; ix++) {
               int64_t x = rand.nextInt64();
               buffer.append((const char*) &x, sizeof x);
            }
            fwrite(buffer.constData(), 1, buffer.length(), fp);
         }
         fclose(fp);
      }
   }
   void checkModifySeed(ReRandomizer& rand) {
      rand.reset();
      QByteArray s1;
      rand.nextString(10, 10, s1);
      rand.reset();
      rand.modifySeed(22);
      QByteArray s2;
      rand.nextString(10, 10, s2);
      checkF(s1 == s2);
      rand.reset();
      rand.modifySeed(22);
      QByteArray s3;
      rand.nextString(10, 10, s3);
      checkEqu(s2, s3);
   }

   void testModifySeed() {
      ReCongruentialGenerator rand;
      checkModifySeed(rand);
      ReXorShift64Randomizer rand2;
      checkModifySeed(rand2);
      ReMultiCongruentialGenerator rand3(2);
      checkModifySeed(rand3);
      ReKISSRandomizer rand4;
      checkModifySeed(rand4);
   }

   void testWrite1m() {
      ReCongruentialGenerator rand;
      write1m(rand);
      ReXorShift64Randomizer rand2;
      write1m(rand2);
      ReMultiCongruentialGenerator rand3(2);
      write1m(rand3);
      ReKISSRandomizer rand4;
      write1m(rand4);
   }

   void checkContent(const char* content, ReByteScrambler& scrambler) {
      QByteArray src(content);
      QByteArray trg;
      QByteArray trg2;
      int markerLength = 4;
      ReByteScrambler scrambler2(scrambler);
      ReKISSRandomizer rand;
      rand.nearTrueRandom();
      QByteArray info;
      rand.nextString(10, 20, info);
      int infoLength = info.length();
      scrambler.initHeader(0, markerLength, infoLength, 0, info);
      scrambler.contentRandom(true).codec(trg, src);
      QByteArray info2;
      QByteArray header2 = scrambler.header();
      info2.fill(' ', infoLength);
      scrambler2.initFromHeader(0, markerLength, infoLength, 0, &header2, info2);
      scrambler2.contentRandom(true).codec(trg2, trg);
      checkEqu(src, trg2);
      checkEqu(info, info2);
   }
   void testContentEncoding() {
      ReKISSRandomizer dataRandom;
      ReByteScrambler scrambler(dataRandom, &m_logger);
      checkContent("12345678abc", scrambler);
      checkContent("8765432112345678ab", scrambler);
      checkContent("8765432112345678", scrambler);
      checkContent("", scrambler);
      ReXorShift64Randomizer rand;
      QByteArray src;
      for (int ix = 0; ix < 1000; ix++) {
         rand.nextString(10, 50, src);
         checkContent(src, scrambler);
      }
      for (int ix = 0; ix < 1000; ix++) {
         src.resize(0);
         for (int ix2 = 0; ix2 < 50; ix2++) {
            int64_t nRand = rand.nextInt64();
            src.append((const char*) &nRand, sizeof nRand);
         }
         checkContent(src, scrambler);
      }
      src = "12345678abcdefghijklmn";
      QByteArray trg;
      scrambler.contentRandom(true).codec(trg, src, 8);
      checkEqu("12345678", trg.mid(0, 8));
      src = "1234XY78abcdefghijklmn";
      QByteArray trg2;
      scrambler.contentRandom(true).codec(trg2, src, 8);
      checkEqu("1234XY78", trg2.mid(0, 8));
      checkEqu(trg.mid(8), trg2.mid(8));
   }
   void testShuffle() {
      ReKISSRandomizer random;
      const int MAX = 177;
      int64_t alField[MAX];
      int anField[MAX];
      unsigned char acField[MAX];
      typedef struct info {
         int64_t m_i64[8];
      } big_t;
      big_t aoField[MAX];
      bool nHit[MAX];
      bool lHit[MAX];
      bool cHit[MAX];
      bool oHit[MAX];
      memset(nHit, 0, sizeof nHit);
      memset(lHit, 0, sizeof lHit);
      memset(cHit, 0, sizeof cHit);
      memset(oHit, 0, sizeof oHit);
      int ix, ix2;
      for (ix = 0; ix < MAX; ix++) {
         anField [ix] = ix + 1000;
         alField [ix] = ix + 10000;
         acField [ix] = ' ' + ix;
         for (ix2 = 0; ix2 < 8; ix2++)
            aoField[ix].m_i64[ix2] = ix2 * 100 + 1000 + ix;
      }
      random.shuffle(alField, MAX, sizeof alField[0]);
      random.shuffle(acField, MAX, sizeof acField[0]);
      random.shuffle(anField, MAX, sizeof anField[0]);
      random.shuffle(aoField, MAX, sizeof aoField[0]);
      for (ix = 0; ix < MAX; ix++) {
         if (anField[ix] >= 1000 && anField[ix] < 1000 + MAX)
            nHit[anField[ix] - 1000] = true;
         else
            checkT(false);
         if (alField[ix] >= 10000 && anField[ix] < 10000 + MAX)
            lHit[alField[ix] - 10000] = true;
         else
            checkT(false);
         if (acField[ix] >= ' ' && acField[ix] < ' ' + MAX)
            cHit[acField[ix] - ' '] = true;
         else
            checkT(false);
         if (aoField[ix].m_i64[0] >= 1000 && aoField[ix].m_i64[0] < 1000 + MAX) {
            oHit[aoField[ix].m_i64[0] - 1000] = true;
            int ix3 = aoField[ix].m_i64[0] - 1000;
            for (int ix2 = 0; ix2 < 8; ix2++) {
               checkEqu(aoField[ix].m_i64[ix2], (int64_t) ix2 * 100 + 1000 + ix3);
            }
         } else
            checkT(false);
      }
      for (ix = 0; ix < MAX; ix++) {
         checkT(nHit[ix]);
         checkT(lHit[ix]);
         checkT(cHit[ix]);
         checkT(oHit[ix]);
      }
   }
   void testRealRandom() {
      int64_t seeds[10];
      for (int ix = 0; ix < 10; ix++)
         seeds[ix] = ReRandomizer::pseudoTrueRandom();
      log("nearRealRandom:");
      for (int ix = 0; ix < 10; ix++)
         printf("%016llx\n", (long long) seeds[ix]);
   }
   void testNextString() {
      ReCongruentialGenerator rand;
      ReXorShift64Randomizer rand2;
      ReMultiCongruentialGenerator rand3(2);
      ReKISSRandomizer rand4;
      QByteArray buffer;
      printf("nextString():\n");
      printf("LCG: %s\n", rand.nextString(80, 80, buffer));
      printf("XOR: %s\n", rand2.nextString(80, 80, buffer));
      printf("MLC: %s\n", rand3.nextString(80, 80, buffer));
      printf("KIS: %s\n", rand4.nextString(80, 80, buffer));
      for (int ix = 0; ix < 500000; ix++) {
         rand.nextSeed64();
         rand2.nextSeed64();
         rand3.nextSeed64();
         rand4.nextSeed64();
      }
      printf("LCG: %s\n", rand.nextString(80, 80, buffer));
      printf("XOR: %s\n", rand2.nextString(80, 80, buffer));
      printf("MLC: %s\n", rand3.nextString(80, 80, buffer));
      printf("KIS: %s\n", rand4.nextString(80, 80, buffer));
   }
   void printHash(const char* source) {
      ReHmHash64 hash;
      hash.update((void*) source, strlen(source));
      printf("%-8s: %s\n", source, hash.hexDigest().constData());
   }
   void printBinary(int length) {
      QByteArray source;
      source.fill(0, length);
      ReHmHash64 hash;
      hash.update((void*) source.constData(), length);
      printf("0 x %2d: %s\n",
             length, hash.hexDigest().constData());
   }

   void testReHmHash64() {
      ReHmHash64 hash;
      hash.update((void*) "12345678abcdefghABC", 8+8+3);
      int64_t value = hash.digestAsInt();
      hash.reset();
      hash.update((void*) "1234567", 7);
      hash.update((void*) "8abcdefgh", 9);
      hash.update((void*) "ABC", 3);
      checkEqu(value, hash.digestAsInt());
      ReKISSRandomizer random;
      for (int ii = 0; ii < 1000; ii++) {
         QByteArray string;
         random.nextString(1, 64, string);
         testOneHash(string, random);
      }
      for (int ii = 0; ii < 1000; ii++) {
         QByteArray data;
         random.nextData(1, 64, data);
         testOneHash(data, random);
      }
      printHash("a");
      printHash("b");
      printHash("aa");
      printHash("ab");
      printHash("ba");
      printHash("aaa");
      printHash("aab");
      printHash("aaaa");
      printHash("aaab");
      printHash("aaaaaaaa");
      printHash("aaaaaaab");
      printHash("aaaaaaba");
      printHash("aaaaabaa");
      printHash("aaaabaaa");
      printHash("aaabaaaa");
      printHash("aabaaaaa");
      printHash("abaaaaaa");
      printHash("baaaaaaa");
      for (int ix = 0; ix < 16; ix++)
         printBinary(ix);
      log("ready");
   }
   void testOneHash(QByteArray source, ReRandomizer& random) {
      ReHmHash64 hash;
      hash.update(source.constData(), source.length());
      int64_t value = hash.digestAsInt();
      while (source.length() > 8) {
         int count = random.nextInt(1, source.length() - 1);
         hash.updateBlock(source.mid(0, count));
         source.remove(0, count);
      }
      hash.updateBlock(source);
      checkEqu(value, hash.digestAsInt());
   }

   void hashPerformance() {
      ReHmHash64 hash;
      QByteArray data;
      data.fill('x', 1024*1024);
      clock_t start = clock();
      int count = 100;
      for (int ix = 0; ix < 100; ix++) {
         hash.update((void*) data.constData(), data.length());
      }
      double duration = double (clock() - start) / CLOCKS_PER_SEC;
      printf("ReHmHash64: %.3f sec %.3f MByte/sec\n", duration,
             count / duration);
   }

   void special() {
      log("ready");
   }
   void checkCodec(ReRandomizer& random, const QByteArray src, int offset) {
      QByteArray trg, trg2;
      random.reset();
      random.codec(trg, src, offset);
      random.reset();
      random.codec(trg2, trg, offset);
      checkEqu(trg2, src);
   }

   void testCodec() {
      ReKISSRandomizer random;
      QByteArray src("12345678");
      checkCodec(random, src, 0);
      QByteArray trg, trg2;
      ReKISSRandomizer random2;
      int sumLength = 0;
      for (int ii = 0; ii < 1000; ii++) {
         random2.nextData(8, 80, src);
         sumLength += src.length();
         int offset = 8 * random2.nextInt(src.length() / 8);
         checkCodec(random, src, offset);
      }
      logv("sum length: %d", sumLength);
      src.fill(0, 1024*1024);
      clock_t start = clock();
      int mbytes = 100;
      for (int ii = 0; ii < mbytes; ii++) {
         random.codec(src);
      }
      double duration = double (clock() - start) / CLOCKS_PER_SEC;
      logv("codec: %d MiByte %.3f sec %.1f MiBytes/sec", mbytes, duration,
           mbytes / duration);
   }
   void testScrambler() {
      ReKISSRandomizer random;
      QByteArray info("abcd12345678abcd1234");
      ReByteScrambler scrambler(random, &m_logger);
      scrambler.initHeader(8, 4, 4 + 16, 8*2+4, info);
      QByteArray info2;
      ReByteScrambler scrambler2(random, &m_logger);
      checkT(scrambler2.initFromHeader(8, 4, 4+16, 8*2+4,
                                       &scrambler.header(), info2));
      checkEqu(info, info2);
   }

   virtual void runTests(void) {
      testContentEncoding();
      testScrambler();
      testCodec();
      special();
      testReHmHash64();
      hashPerformance();
      testNextString();
      testRealRandom();
      testShuffle();
      special();
      testModifySeed();
      testTextToSeed();
      testWrite1m();
      testBasics();
   }
};
void testReRandomizer() {
   TestReRandomizer test;
}

