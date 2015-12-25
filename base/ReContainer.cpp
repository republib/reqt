/*
 * ReContainer.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"

/** @file
 * @brief Implements a portable data container.
 */

/** @file base/ReContainer.hpp
 *
 * @brief Definition for a portable data container.
 */

/** @class ReContainer ReContainer.hpp "base/ReContainer.hpp"
 *
 * @brief Implements a portable data container.
 *
 * The container contains a list of "bags".
 * Each bag contains a sequence of items (with a simple data type).
 * The items are portable: transported to another architecture
 * the item is restored correct (independent of big/little endian).
 *
 * Format:
 * container ::= magic header_size_hex2 container_size_hex '[' list_count ']' bag_descr list_of_bags<br>
 * list_of_bag ::= bag1 bag2 ...<br>
 * bag_descr ::= bag_type1 bag_type2 ... ':'<br>
 * bag_types := i(nteger) ' ' | s(tring) | b(indata255) | B(indata64k) X(blob4G)<br>
 * item_list ::= item1 item2...<br>
 * The binary items (bBX) are byte sequences with a starting size element.
 * The size element can be a byte (255) or a word (64k) or a double word(4G).
 * The order of the size element is big endian.
 *
 * Example (additional blanks for readibility):
 * <pre>
 * Rpl&1 09 36[2]cis:!7b Nirwana &lt;0&gt; Y -ab34 A long string with an trailing '0' &lt;0&gt;
 * </pre>
 * magic header: Rpl&1 length: 09h data length: 46h bag count: 2 item types: char int string
 *
 */

enum {
   // 11000
   LOC_FILL_1 = LOC_CONTAINER * 1000,
   LOC_FILL_2,
   LOC_FILL_3,
   LOC_NEXT_BAG_1,
   LOC_NEXT_ITEM_1,
   LOC_NEXT_ITEM_2 = 11005,
   LOC_NEXT_INT_1,
   LOC_NEXT_ITEM_3,
   LOC_NEXT_BAG_2,
};

const char* ReContainer::MAGIC_1 = "Rpl&1";

/**
 * @brief Constructor.
 *
 * @param sizeHint      Probable length of the container
 */
ReContainer::ReContainer(size_t sizeHint) :
   m_data(""),
   m_countBags(0),
   m_typeList(""),
   m_ixItem(0),
   m_ixBag(0),
   m_readPosition(NULL) {
   if (sizeHint > 0)
      m_data.reserve(sizeHint);
}

/**
 * @brief Destructor.
 */
ReContainer::~ReContainer() {
}

/**
 * @brief Adds an type to the type list.
 *
 * @param tag   the type tag
 */
void ReContainer::addType(type_tag_t tag) {
   if (m_countBags == 0)
      startBag();
   if (m_countBags == 1)
      m_typeList.append((char) tag);
}

/**
 * @brief Starts a new bag.
 */
void ReContainer::startBag() {
   m_countBags++;
   m_ixBag = 0;
}
/**
 * @brief Adds a character to the current bag.
 *
 * @param value    value to insert
 */
void ReContainer::addChar(char value) {
   addType(TAG_CHAR);
   //if (m_typeList.at(m_ixBag) != TAG_INT)
   //   ReLogger::logAndThrow(LOG_ERROR, __FILE__, __LINE__, 1, "unexpected type: %c instead of c", m_typeList.at(m_ixBag));
   m_data.append(value);
}
/**
 * @brief Adds an integer to the current bag.
 *
 * @param value    value to add
 */
void ReContainer::addInt(int value) {
   addType(TAG_INT);
   char buffer[64];
   char* ptr = buffer;
   if (value < 0) {
      *ptr++ = '-';
      value = -value;
   }
   qsnprintf(ptr, sizeof buffer - 1, "%x ", value);
   m_data.append(buffer);
}
/**
 * @brief Adds an integer to the current bag.
 *
 * @param value    value to add
 */
void ReContainer::addInt(int64_t value) {
   addType(TAG_INT);
   char buffer[128];
   qsnprintf(buffer, sizeof buffer, "%llx ", value);
   m_data.append(buffer);
}

/**
 * @brief Adds a string to the current bag.n
 *
 * @param value    value to add
 */
void ReContainer::addString(const char* value) {
   addType(TAG_STRING);
   // store with trailing '\0'
   m_data.append(value, strlen(value) + 1);
}
/**
 * @brief Adds binary data to the current bag.
 *
 * @param value     binary data
 * @param size      size of the binary data in bytes
 */
void ReContainer::addData(uint8_t* value, size_t size) {
   if (size <= 255) {
      addType(TAG_DATA255);
      m_data.append((char) size);
   } else if (size <= 0xffff) {
      addType(TAG_DATA64K);
      m_data.append((char) (size / 256));
      m_data.append((char) (size % 256));
      m_data.append((const char*) value, size);
   } else {
      addType(TAG_DATA4G);
      m_data.append((char) (size / 256 / 256 / 256));
      m_data.append((char) (size / 256 / 256 % 256));
      m_data.append((char) (size / 256 % 256));
      m_data.append((char) (size % 256));
      m_data.append((const char*) value, size);
   }
   addType(TAG_DATA255);
}

/**
 * @brief Returns the container byte buffer.
 *
 * @return the container as a byte array
 */
const QByteArray& ReContainer::getData() {
   if (m_typeList.length() != 0) {
      char buffer[128];
      // RPL&1 0a b5[2]cis: !12
      qsnprintf(buffer, sizeof buffer, "%x[%d]%s:",
                (unsigned int) m_data.length(), m_countBags, m_typeList.data());
      char header[128 + 8];
      qsnprintf(header, sizeof header, "%s%02x%s", MAGIC_1,
                (unsigned int) strlen(buffer), buffer);
      m_data.insert(0, header);
   }
   return m_data;
}

/**
 * @brief Fills the container with an byte array.
 *
 * @param data      the container as a byte array
 */
void ReContainer::fill(const QByteArray& data) {
   m_data = data;
   const char* ptr = m_data.data();
   if (strncmp(ptr, MAGIC_1, strlen(MAGIC_1)) != 0)
      throw RplInvalidDataException(LOG_ERROR, LOC_FILL_1,
                                    "container has no magic", data.data(), data.length());
   ptr += strlen(MAGIC_1);
   unsigned int headerSize = 0;
   if (sscanf(ptr, "%02x", &headerSize) != 1)
      throw RplInvalidDataException(LOG_ERROR, LOC_FILL_2,
                                    "container has no header size", ptr, 2);
   ptr += 2;
   unsigned int dataSize = 0;
   unsigned int countBags = 0;
   if (sscanf(ptr, "%x[%x]", &dataSize, &countBags) != 2)
      throw RplInvalidDataException(LOG_ERROR, LOC_FILL_2,
                                    "container has no data_size[bag_count]", ptr, 16);
   m_countBags = countBags;
   ptr = strchr(ptr, ']') + 1;
   const char* end = ptr + strspn(ptr, "cisdDX!");
   if (end == ptr || *end != ':') {
      throw RplInvalidDataException(LOG_ERROR, LOC_FILL_2,
                                    "container has no valid typelist", ptr, 16);
   }
   m_typeList.clear();
   m_typeList.append(ptr, end - ptr);
   m_ixBag = -1;
   m_readPosition = (uint8_t*) end + 1;
}
/**
 * @brief Returns the number of bags in the container.
 *
 * @return the number of bags
 */
int ReContainer::getCountBags() const {
   return m_countBags;
}
/**
 * @brief Sets the begin of the new bag.
 */
void ReContainer::nextBag() {
   if (m_ixItem < m_typeList.length() && m_ixItem != -1)
      throw ReException(LOG_ERROR, LOC_NEXT_BAG_1, NULL,
                        "end of bag not reached: remaining items: %s",
                        m_typeList.data() + m_ixItem);
   m_ixItem = 0;
   m_ixBag++;
   if (m_ixBag >= m_countBags)
      throw ReException(LOG_ERROR, LOC_NEXT_BAG_2, NULL, "no more bags: %d",
                        m_ixBag);
}
/**
 * @brief Sets the next item.
 *
 * @param expected      the expected data type
 */
void ReContainer::nextItem(type_tag_t expected) {
   if (m_ixBag < 0) {
      m_ixBag = 0;
      m_ixItem = 0;
   }
   if (m_ixItem >= m_typeList.length())
      throw ReException(LOG_ERROR, LOC_NEXT_ITEM_1, ReLogger::globalLogger(),
                        "no more items in the bag");
   type_tag_t current = (type_tag_t) m_typeList.at(m_ixItem);
   // Unify all data types:
   if (current == TAG_DATA4G || current == TAG_DATA64K)
      current = TAG_DATA255;
   if (current != expected)
      throw ReException(LOG_ERROR, LOC_NEXT_ITEM_2, NULL,
                        "current item is a %c, not a %c", (char) m_typeList.at(m_ixItem),
                        (char) expected);
   m_ixItem++;
   if (m_readPosition > (uint8_t*) (m_data.data() + m_data.length()))
      throw ReException(LOG_ERROR, LOC_NEXT_ITEM_3, NULL,
                        "container size too small. Bag: %d of %d Item: %d of %d",
                        1 + m_ixBag, m_countBags, 1 + m_ixItem, m_typeList.length());
}

/**
 * @brief Reads the next character from the current item in the current bag.
 *
 * @return  the next char from the container
 */
char ReContainer::nextChar() {
   nextItem(TAG_CHAR);
   char rc = *m_readPosition++;
   return rc;
}

/**
 * @brief Reads the next integer from the current item in the current bag.
 *
 * @return  the next integer from the container
 */
int ReContainer::nextInt() {
   nextItem(TAG_INT);
   bool isNegativ = *m_readPosition == '-';
   if (isNegativ)
      m_readPosition++;
   unsigned int value = 0;
   if (sscanf((const char*) m_readPosition, "%x ", &value) != 1)
      throw RplInvalidDataException(LOG_ERROR, LOC_NEXT_INT_1,
                                    "not a hex_number<blank>", m_readPosition, 16);
   m_readPosition = (uint8_t*) strchr((const char*) m_readPosition, ' ') + 1;
   if (isNegativ)
      value = -value;
   return value;
}
/**
 * @brief Reads the next integer from the current item in the current bag.
 *
 * @return  the next integer from the container
 */
int64_t ReContainer::nextInt64() {
   nextItem(TAG_INT);
   bool isNegativ = *m_readPosition == '-';
   if (isNegativ)
      m_readPosition++;
   uint64_t value = 0;
   if (sscanf((const char*) m_readPosition, "%llx ", &value) != 1)
      throw RplInvalidDataException(LOG_ERROR, LOC_NEXT_INT_1,
                                    "not a hex_number<blank>", m_readPosition, 16);
   m_readPosition = (uint8_t*) strchr((const char*) m_readPosition, ' ') + 1;
   if (isNegativ)
      value = -value;
   return (int64_t) value;
}

/**
 * @brief Reads the next string from the current item in the current bag.
 *
 * @return  the next '\0' delimited string from the container
 */
const char* ReContainer::nextString() {
   nextItem(TAG_STRING);
   const char* rc = (const char*) m_readPosition;
   m_readPosition += strlen(rc) + 1;
   return rc;
}

/**
 * @brief Reads the next string from the current item in the current bag.
 *
 * @param data      OUT: the next data item from the container
 * @param append    true: the item data will be appended to data<br>
 *                  false: data contains the item data only
 * @return          the size of the read data
 */
size_t ReContainer::nextData(QByteArray& data, bool append) {
   nextItem(TAG_DATA255);
   type_tag_t tag = (type_tag_t) m_typeList.at(m_ixItem - 1);
   size_t length = 0;
   switch (tag) {
   case TAG_DATA4G:
      for (int ix = 3; ix >= 0; ix--) {
         length = 256 * length + m_readPosition[ix];
      }
      m_readPosition += 4;
      break;
   case TAG_DATA64K:
      length = *m_readPosition++ * 256;
      length += *m_readPosition++;
      break;
   case TAG_DATA255:
      length = *m_readPosition++;
      break;
   default:
      break;
   }
   if (!append)
      data.clear();
   data.append((const char*) m_readPosition, length);
   m_readPosition += length;
   return length;
}

/**
 * @brief Dumps a container as a human readable string.
 *
 * @param title             will be used in the first line
 * @param maxBags           if there are more bags they will be ignored
 * @param maxStringLength   if strings are longer the will be cut
 * @param maxBlobLength     maximum count of bytes which will be dumped
 * @param separatorItems    separator between two items, e.g. '\\n' or '|'
 * @return                  a human readable string describing the container
 */
QByteArray ReContainer::dump(const char* title, int maxBags,
                             int maxStringLength, int maxBlobLength, char separatorItems) {
   QByteArray rc;
   rc.reserve(64000);
   rc.append("=== ").append(title).append('\n');
   rc.append("Bags: ").append(ReStringUtils::toNumber(m_countBags));
   rc.append(" Types: ").append(m_typeList).append('\n');
   // save the current state:
   int safeIxBag = m_ixBag;
   int safeIxItem = m_ixItem;
   m_ixBag = -1;
   m_ixItem = 0;
   int iValue;
   QByteArray sValue;
   if (maxBags > m_countBags)
      maxBags = m_countBags;
   for (int ixBag = 0; ixBag < maxBags; ixBag++) {
      rc.append("--- bag ").append(ReStringUtils::toNumber(ixBag)).append(
         ":\n");
      nextBag();
      QByteArray item;
      int maxLength;
      for (int ixItem = 0; ixItem < m_typeList.length(); ixItem++) {
         type_tag_t currentType = (type_tag_t) m_typeList.at(ixItem);
         switch (currentType) {
         case TAG_CHAR:
            rc.append(" c: ").append(nextChar()).append(separatorItems);
            break;
         case TAG_INT:
            iValue = nextInt();
            rc.append(" i: ").append(ReStringUtils::toNumber(iValue)).append(
               " / ");
            rc.append(ReStringUtils::toNumber(iValue, "%x")).append(
               separatorItems);
            break;
         case TAG_STRING:
            sValue = nextString();
            if (sValue.length() > maxStringLength)
               sValue = sValue.left(maxStringLength);
            rc.append(" s: ").append(sValue).append(separatorItems);
            break;
         case TAG_DATA255:
         case TAG_DATA64K:
         case TAG_DATA4G:
            nextData(item, false);
            rc.append(' ').append((char) currentType).append(": [");
            rc.append(ReStringUtils::toNumber(item.length())).append("] ");
            maxLength =
               item.length() < maxBlobLength ?
               item.length() : maxBlobLength;
            rc.append(ReStringUtils::hexDump(item.data(), maxLength, 16))
            .append(separatorItems);
            break;
         default:
            break;
         }
      }
   }
   // restore the current state:
   m_ixBag = safeIxBag;
   m_ixItem = safeIxItem;
   return rc;
}

