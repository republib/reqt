/*
 * ReByteStorage.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** \@file
 *
 * @brief A very efficient storage for bytes and C strings.
 */
/** @file base/ReByteStorage.hpp
 *
 * @brief Definitions for a very efficient storage for bytes and C strings.
 */

#include "base/rebase.hpp"
/** @class ReByteStorage ReByteStorage.hpp "base/ReByteStorage.hpp"
 *
 * @brief Implements a very efficient byte storage.
 *
 * Efficiency: Allocation of one block needs mostly only 1 comparison
 * and 2 assignments.
 *
 * Restriction: The blocks can be returned (freed) only all together, not block by block.
 * This can be an advantage!
 *
 * Process:
 * The storage manages large buffers. Allocation can be done only in the
 * last buffer. If the buffer has too little space for the new block a new
 * buffer will be allocated and linked into the buffer list.
 * One buffer can store dozens or hundreds of blocks. Therefore allocation and
 * freeing is much cheeper than allocation by <code>new()</code>.
 */

/**
 * @brief Constructor.
 *
 * @param bufferSize    the size of one buffer
 */
ReByteStorage::ReByteStorage(int bufferSize) :
   m_bufferSize(bufferSize),
   m_buffer(NULL),
   m_rest(0),
   m_freePosition(NULL),
   m_summarySize(0),
   m_buffers(0) {
}

/**
 * @brief Destructor.
 */
ReByteStorage::~ReByteStorage() {
   const uint8_t* ptr = m_buffer;
   while (ptr != NULL) {
      const uint8_t* old = ptr;
      ptr = *(const uint8_t**) (ptr);
      delete[] old;
      m_buffers--;
   }
   assert(m_buffers == 0);
}

/**
 * @brief Allocates a block in a new allocated buffer.
 *
 * This method will be called if the buffer has too little space.
 * A new buffer will be allocated and the block will be allocated
 * in this new block.
 *
 * @note The block address is returned, but the allocation must be done outside!
 *
 * @param   size of the new block (inclusive the trailing '\0')
 * @return  a new block with the <code>size</code> bytes
 */
char* ReByteStorage::allocBuffer(int size) {
   m_rest =
      size + sizeof(uint8_t*) <= (size_t) m_bufferSize ?
      m_bufferSize : size + sizeof(uint8_t*);
   m_summarySize += m_rest;
   m_buffers++;
   uint8_t* rc = new uint8_t[m_rest];
   *(uint8_t**) rc = m_buffer;
   m_buffer = rc;
   rc += sizeof(uint8_t*);
   // the block allocation will be done outside!
   m_freePosition = rc;
   m_rest -= sizeof(uint8_t*);
   return reinterpret_cast<char*>(rc);
}

/**
 * @brief Duplicates a string into a new allocated block.
 *
 * @param source    the source string
 * @param size      the length of the string.
 *                  If < 0 <code>strlen(source)</code> will be used
 * @return          a copy of the source string. The copy ends always with '\0'
 */
const char* ReByteStorage::allocateChars(const char* source, int size) {
   if (size < 0)
      size = strlen(source);
   const char* rc = allocateChars(size + 1);
   memcpy((void*) rc, source, size);
   ((char*) rc)[size] = '\0';
   return rc;
}

/**
 * @brief Duplicates a string into a new allocated block.
 *
 * The unicode string will be converted into an UTF-8 string.
 *
 * @param source    the source string
 * @return          a copy of the source string. The copy ends always with '\0'
 */
const char* ReByteStorage::allocUtf8(const ReString& source) {
   const char* rc = allocateChars(I18N::s2b(source).constData());
   return rc;
}

/**
 * @brief Allocates a byte block without initialization.
 *
 * @param size  the size of the block to allocate
 *
 * @return  a byte block (without a trailing '\0')
 */
uint8_t* ReByteStorage::allocateBytes(int size) {
   uint8_t* rc =
      size <= m_rest ?
      m_freePosition : reinterpret_cast<uint8_t*>(allocBuffer(size));
   m_freePosition += size;
   m_rest -= size;
   return rc;
}

/**
 * @brief Allocates a byte block initialized by '\0'.
 *
 * @param size  the size of the block to allocate
 *
 * @return  a byte block (without a trailing '\0')
 */
uint8_t* ReByteStorage::allocateZeros(int size) {
   uint8_t* rc = allocateBytes(size);
   memset(rc, 0, size);
   return rc;
}

/**
 * @brief Copy a byte block to a new allocated byte block.
 *
 * @param source    the source to copy
 * @param size      the size of the block to allocate
 * @return          a byte block (without a trailing '\0')
 */
uint8_t* ReByteStorage::allocateBytes(void* source, int size) {
   uint8_t* rc = allocateBytes(size);
   memcpy(rc, source, size);
   return rc;
}
