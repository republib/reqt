/*
 * ReContainer.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RECONTAINER_HPP
#define RECONTAINER_HPP

// the sources generated from QT include this file directly:
#ifndef RPLCORE_HPP
#include <QByteArray>
#include <QDataStream>
#endif
class ReContainer {
public:
   typedef enum {

      TAG_CHAR = 'c',     ///< one character
      TAG_INT = 'i',      ///< an integer number, up to 64 bit
      TAG_STRING = 's',   ///< a string ending with a '\\0'
      TAG_DATA255 = 'd',  ///< binary data, up to 255 bytes long
      TAG_DATA64K = 'D',  ///< binary data, up to 64 KiBytes long
      TAG_DATA4G = 'X',   ///< binary data, up to 4 GiBytes long
      TAG_CONTAINER = '!' ///< a container (recursion)
   } type_tag_t;
   static const char* MAGIC_1;
public:
   ReContainer(size_t sizeHint);
   virtual ~ReContainer();
private:
   // No copy constructor: no implementation!
   ReContainer(const ReContainer& source);
   // Prohibits assignment operator: no implementation!
   ReContainer& operator =(const ReContainer& source);
public:
   // Building the container:
   void addType(type_tag_t tag);
   void startBag();
   void addChar(char cc);
   void addInt(int value);
   void addInt(int64_t value);
   void addString(const char* value);
   void addData(uint8_t* value, size_t size);
   const QByteArray& getData();

   // Getting data from the container:
   void fill(const QByteArray& data);
   int getCountBags() const;
   const char* getTypeList() const;
   void nextBag();
   char nextChar();
   int nextInt();
   int64_t nextInt64();
   const char* nextString();
   size_t nextData(QByteArray& data, bool append = false);

   QByteArray dump(const char* title, int maxBags, int maxStringLength = 80,
                   int maxBlobLength = 16, char separatorItems = '\n');
private:
   void nextItem(type_tag_t expected);
private:
   // the complete data of the container
   QByteArray m_data;
   // the number of elements in the container
   int m_countBags;
   // a string with the data types of a bag
   QByteArray m_typeList;

   // Getting data from the container:

   // current read position in m_typeList
   int m_ixItem;
   // the index of the current current bag:
   int m_ixBag;
   // read position in m_data:
   const uint8_t* m_readPosition;
};

#endif // RECONTAINER_HPP
