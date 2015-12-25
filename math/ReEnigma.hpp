/*
 * ReEnigma.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RPLENIGMA_HPP
#define RPLENIGMA_HPP

class ReEnigma {
public:
   ///> '0'..'9'
   static const char* SET_DECIMALS;
   ///> '0'..'9' 'a'..'f'
   static const char* SET_HEXDIGITS;
   ///> '0'..'9' 'A'..'Z' a'..'z' '_'
   static const char* SET_ALPHANUM;
   ///> '0'..'9' 'A'..'Z' z'..'z' '_'
   static const char* SET_FILENAME;
   ///> ' ' .. chr(127)
   static const char* SET_32_127;
   ///> ' ' .. chr(255)
   static const char* SET_32_255;
   ///> TAB, CR, NL, ' '..chr(127)
   static const char* SET_PRINTABLE_127;
   ///> TAB, CR, NL, ' '..chr(255)
   static const char* SET_PRINTABLE_255;

protected:
   typedef struct {
      int m_count;
      uint64_t* m_list;
   } secret_t;
public:
   ReEnigma(ReRandom* random = NULL);
   virtual ~ReEnigma();
private:
   // No copy constructor: no implementation!
   ReEnigma(const ReEnigma& source);
   // Prohibits assignment operator: no implementation!
   ReEnigma& operator =(const ReEnigma& source);
public:
   void encode(char* data, int size, const char* charSet, QByteArray& cache);
   void decode(char* data, int size, const char* charSet, QByteArray& cache);
   void change(QByteArray& data);
   void addByteSecret(QByteArray byteSeed);
   void setSeed(uint64_t seed);
   QByteArray readCertificate(const char* filename);
protected:
   int nextInt(int maxValue);

protected:
   ///> a pseudo random generator
   ReRandom* m_random;
   ///> true: m_random must be destroyed (in the destructor).
   bool m_ownsRandom;
   ///> This values will be mixed with <code>m_random</code>' seed
   QList<secret_t*> m_secrets;
   ///> each call of setSeed sets this value to 0.
   int m_randomCalls;
   ///> a byte sequence derived from the secrets
   QByteArray m_randomSource;
};

#endif // RPLENIGMA_HPP
