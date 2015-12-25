/*
 * ReConfig.cpp
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
 *
 * @brief Reading/writing configuration files.
 */
/** @file base/ReConfig.hpp
 *
 * @brief Definitions for reading/writing configuration files.
 */

/** @class ReConfig ReConfig.hpp "base/ReConfig.hpp"
 *
 * @brief Imports and exports a configuration file into a QHash instance.
 *
 * The format of the file:<br>
 * DEFS or COMMENTS
 *
 * DEFS ::= KEY=VALUE
 *
 * KEY is a string starting with an alphanumeric character and does not contain a '='
 *
 * VALUE is a string
 */

enum Locations {
   LOC_WRITE_1 = LOC_FIRST_OF(LOC_CONFIG),   // 10201
   LOC_WRITE_2,
   LOC_READ_1,
   LOC_READ_2,
};

/**
 * Constructor.
 *
 * Initializes the logger and reads the configuration file
 *
 * @param file          name of the configuration file. May be NULL
 * @param readOnly      true: the configuration must not be written
 * @param logger        NULL or a logger
 */
ReConfig::ReConfig(const char* file, bool readOnly, ReLogger* logger) :
   m_file(file),
   m_lineList(),
   m_readOnly(readOnly),
   m_logger(logger),
   m_ownLogger(logger == NULL) {
   if (logger == NULL) {
      initLogger();
   }
   if (file != NULL)
      read(file);
}

/**
 * Destructor.
 *
 * Frees the resources.
 */
ReConfig::~ReConfig() {
   if (m_ownLogger)
      delete m_logger;
   m_logger = NULL;
}

/**
 * Inititializes a logger.
 */
void ReConfig::initLogger() {
   m_logger = new ReLogger();
   ReMemoryAppender* appender = new ReMemoryAppender();
   appender->setAutoDelete(true);
   m_logger->addAppender(appender);
   ReStreamAppender* appender2 = new ReStreamAppender(stdout);
   appender2->setAutoDelete(true);
   m_logger->addAppender(appender2);
}

/**
 * Returns  configuration value as an integer.
 *
 * @param key           key of the wanted value
 * @param defaultValue  if the key does not exist this is the result
 * @return              defaultValue: key does not exist
 *                      otherwise: the value assigned to key
 */
int ReConfig::asInt(const char* key, int defaultValue) const {
   int rc = defaultValue;
   if (contains(key)) {
      rc = atoi((*this)[key]);
   }
   return rc;
}

/**
 * Returns the configuration value as a boolean.
 *
 * @param key           key of the wanted value
 * @param defaultValue  if the key does not exist this is the result
 * @return              defaultValue: key does not exist
 *                      otherwise: the value assigned to key
 */
bool ReConfig::asBool(const char* key, bool defaultValue) const {
   bool rc = defaultValue;
   if (contains(key)) {
      QByteArray value = (*this)[key].toLower();
      rc = value == "1" || value == "y" || value == "yes" || value == "t"
           || value == "true";
   }
   return rc;
}

/**
 * Returns a configuration value.
 *
 * @param key           key of the wanted value
 * @param defaultValue  if the key does not exist this is the result
 * @return              defaultValue: key does not exist
 */
QByteArray ReConfig::asString(const char* key, const char* defaultValue) {
   QByteArray rc = defaultValue;
   if (contains(key)) {
      rc = (*this)[key];
   }
   return rc;
}

/**
 * Reads a configuration file.
 *
 * @param file  file to read.
 * @return      true: OK<br>
 *              false: error occurred
 */
bool ReConfig::read(const char* file) {
   bool rc = true;
   m_lineList.reserve(1024);
   FILE* fp = fopen(file, "r");
   if (fp == NULL) {
      m_logger->logv(LOG_ERROR, LOC_READ_1, "cannot read: %s", file);
      rc = false;
   } else {
      char line[64000];
      char* separator;
      int lineNo = 0;
      while (fgets(line, sizeof line, fp) != NULL) {
         lineNo++;
         m_lineList.append(line);
         if (isalnum(line[0]) && (separator = strchr(line, '=')) != NULL) {
            QByteArray key(line, separator - line);
            QByteArray value(separator + 1);
            key = key.trimmed();
            value = value.trimmed();
            if (contains(key))
               m_logger->logv(LOG_WARNING, LOC_READ_2,
                              "defined more than once: %s-%d: %s", file, lineNo,
                              line);
            else
               insert(key, value);
         }
      }
   }
   return rc;
}

/**
 * Writes a configuration file.
 *
 * @param file  file to write.
 * @return      true: OK<br>
 *              false: error occurred
 */
bool ReConfig::write(const char* file) {
   bool rc = false;
   if (m_readOnly)
      m_logger->log(LOG_ERROR, LOC_WRITE_1, "cannot write: (readonly");
   else {
      m_logger->logv(LOG_ERROR, LOC_WRITE_2, "not implemented: write(%s)",
                     file);
   }
   return rc;
}

