/*
 * ReByteStorage.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RECHARSTORAGE_HPP
#define RECHARSTORAGE_HPP

class ReByteStorage {
public:
   ReByteStorage(int blockSize);
   ~ReByteStorage();
public:
   char* allocBuffer(int size);
   /**
    * @brief Allocates a char block.
    *
    * @return a new block
    */
   inline char* allocateChars(int size) {
      char* rc = size <= m_rest ? (char*) m_freePosition : allocBuffer(size);
      m_freePosition += size;
      m_rest -= size;
      return rc;
   }
   const char* allocateChars(const char* source, int size = -1);
   const char* allocUtf8(const ReString& source);
   uint8_t* allocateBytes(int size);
   uint8_t* allocateZeros(int size);
   uint8_t* allocateBytes(void* source, int size);
private:
   int m_bufferSize;
   uint8_t* m_buffer;
   int m_rest;
   uint8_t* m_freePosition;
   int64_t m_summarySize;
   int m_buffers;
};

#endif // RECHARSTORAGE_HPP
