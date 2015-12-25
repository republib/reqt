/*
 * ReMatcher.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Processors for pattern matching.
 *
 * Patterns can contain wildcards: '*' for any string
 */
/** @file base/ReMatcher.hpp
 *
 * @brief Efficient pattern matching.
 */
#include "base/rebase.hpp"

QStringList* ReListMatcher::m_allMatchingList = NULL;
ReListMatcher* ReListMatcher::m_allMatcher = NULL;
ReIncludeExcludeMatcher* ReIncludeExcludeMatcher::m_allMatcher = NULL;
/**
 * Constructor.
 *
 * @param pattern			a pattern with wildcards '*' (any string)
 * @param caseSensivity		caseSensitive or caseInsensitive
 * @param anchored			<code>true</code>: the pattern starts at the strings
 *							start<br>
 *							<code>false</code>: the pattern can start anywhere in
 *							the string
 */

ReMatcher::ReMatcher(const QString& pattern, Qt::CaseSensitivity caseSensivity,
                     bool anchored) :
   m_pattern(),
   m_needles(),
   m_restLengths(),
   m_anchored(anchored),
   m_caseSensivitiy(caseSensivity) {
   setPattern(pattern, anchored);
}

/**
 * Tests whether the given string matches the pattern.
 *
 * @param text	the text to test
 * @return		<code>true</code>: the pattern matches
 */
bool ReMatcher::matches(const QString& text) {
   bool found = m_allMatching || m_needles.size() == 0;
   int endIx = m_needles.size() - 1;
   if (!found && m_anchored) {
      found = m_needles.at(0).size() == 0
              || text.startsWith(m_needles.at(0), m_caseSensivitiy);
      if (found && (endIx > 0 || text.length() != m_pattern.length())) {
         found = m_needles.at(endIx).size() == 0
                 || text.endsWith(m_needles.at(endIx), m_caseSensivitiy);
      }
   }
   if (!found || (m_anchored && endIx > 1)) {
      int startIx = 0;
      int textIndex = 0;
      if (!m_anchored)
         found = true;
      else {
         startIx++;
         endIx--;
         textIndex = m_needles.at(0).length();
      }
      for (int ix = startIx; found && ix <= endIx; ix++) {
         found = text.size() - textIndex >= m_restLengths.at(ix);
         if (found)
            found = (textIndex = text.indexOf(m_needles.at(ix), textIndex,
                                              m_caseSensivitiy)) >= 0;
      }
   }
   return found;
}

/**
 * Sets the search pattern.
 *
 * @param pattern	the search pattern, with wildcards '*' (any string)
 * @param anchored	<code>true</code>: the pattern must match at the begin<br>
 *					<code>false</code>: the pattern can match anywhere
 */
void ReMatcher::setPattern(const QString& pattern, bool anchored) {
   m_anchored = anchored;
   m_allMatching = false;
   m_pattern = pattern;
   m_needles.clear();
   if (pattern.isEmpty())
      m_needles.clear();
   else {
      m_needles = pattern.split('*');
      // Eliminate empty entries but not first and next:
      for (int ix = m_needles.size() - 2; ix > 0; ix--) {
         if (m_needles.at(ix).length() == 0)
            m_needles.removeAt(ix);
         else
            m_restLengths.append(0);
      }
      int sum = 0;
      for (int ix = m_needles.size() - 1; ix >= 0; ix--) {
         int size = m_needles.at(ix).size();
         m_restLengths.append(size + sum);
         sum += size;
      }
   }
}

bool ReMatcher::anchored() const {
   return m_anchored;
}

void ReMatcher::setAnchored(bool anchored) {
   m_anchored = anchored;
}
/**
 * Returns the current pattern.
 *
 * @return	the current pattern
 */
const QString& ReMatcher::pattern() const {
   return m_pattern;
}

/**
 * Returns whether the matcher accepts all strings (pattern "*").
 *
 * @return <code>true</code>: the matcher accepts all strings
 */
bool ReMatcher::allMatching() const {
   return m_allMatching;
}

/**
 * Returns the case sensitivity of the pattern matching.
 *
 * @return	<code>true</code>: the character case is relevant
 */
Qt::CaseSensitivity ReMatcher::caseSensivitiy() const {
   return m_caseSensivitiy;
}

/**
 * Sets the case sensisitivy of the pattern matching.
 *
 * @param caseSensivitiy	<code>true</code>: the character case is relevant
 */
void ReMatcher::setCaseSensivitiy(const Qt::CaseSensitivity& caseSensivitiy) {
   m_caseSensivitiy = caseSensivitiy;
}

/**
 * Constructor.
 *
 * @param patterns			a list of patterns with wildcards '*' (any string)
 * @param caseSensivity		caseSensitive or caseInsensitive
 * @param anchored			<code>true</code>: the pattern starts at the strings
 *							start<br>
 *							<code>false</code>: the pattern can start anywhere in
 *							the string
 */
ReListMatcher::ReListMatcher(const QStringList& patterns,
                             Qt::CaseSensitivity caseSensivity, bool anchored) :
   m_patterns(patterns),
   m_list(),
   m_empty(false),
   m_allMatching(false) {
   setPatterns(patterns, caseSensivity, anchored);
}
/**
 * Destructor.
 */
ReListMatcher::~ReListMatcher() {
   destroy();
}

/**
 * Returns whether the search finds all (pattern "*").
 *
 * @return <code>true</code>: the matcher accept all strings
 */
bool ReListMatcher::allMatching() const {
   return m_allMatching;
}

/**
 * Returns a string list with one entry "*".
 *
 * @return a
 */
const QStringList& ReListMatcher::allMatchingList() {
   if (m_allMatchingList == NULL) {
      m_allMatchingList = new QStringList();
      m_allMatchingList->append("*");
   }
   return *m_allMatchingList;
}

/**
 * Returns the anchor flag of the pattern matching.
 *
 * @return	<code>true</code>: the character case is relevant
 */
bool ReListMatcher::anchored() const {
   return m_list.length() <= 0 ? true : m_list.at(0)->anchored();
}

/**
 * Returns the case sensitivity of the pattern matching.
 *
 * @return	<code>true</code>: the character case is relevant
 */
Qt::CaseSensitivity ReListMatcher::caseSensivitiy() const {
   return m_list.length() <= 0 ? Qt::CaseSensitive
          : m_list.at(0)->caseSensivitiy();
}

/**
 * Frees the resources.
 */
void ReListMatcher::destroy() {
   QList<ReMatcher*>::const_iterator it;
   for (it = m_list.constBegin(); it != m_list.cend(); ++it) {
      delete *it;
   }
   m_list.clear();
}

/**
 * Returns a pattern match processor accepting all strings.
 *
 * @return a matcher accepting all strings
 */
const ReListMatcher& ReListMatcher::allMatcher() {
   if (m_allMatcher == NULL) {
      m_allMatcher = new ReListMatcher(allMatchingList());
   }
   return *m_allMatcher;
}

/**
 * Returns the current pattern list.
 *
 * @return the pattern list
 */
const QStringList& ReListMatcher::patterns() const {
   return m_patterns;
}

/**
 * Returns whether the pattern list is empty.
 *
 * @return	<code>true</code>: the pattern list is empty
 */
bool ReListMatcher::empty() const {
   return m_empty;
}

/**
 * Tests whether at least one pattern of the list matches the given text
 *
 * @param text	text to test
 * @return		<code>true</code>: empty list or one of the stored patterns
 *				matches the text<br>
 *				<code>false</code>: none of the patterns matches
 */
bool ReListMatcher::matches(const QString& text) const {
   QList<ReMatcher*>::const_iterator it;
   bool rc = m_list.size() == 0;
   for (it = m_list.cbegin(); !rc && it != m_list.cend(); ++it) {
      rc = (*it)->matches(text);
   }
   return rc;
}

/**
 * Sets the case sensisitivy of the pattern matching.
 *
 * @param caseSensivitiy	<code>true</code>: the character case is relevant
 */
void ReListMatcher::setCaseSensivitiy(
   const Qt::CaseSensitivity& caseSensivitiy) {
   QList<ReMatcher*>::const_iterator it;
   for (it = m_list.begin(); it != m_list.end(); ++it) {
      (*it)->setCaseSensivitiy(caseSensivitiy);
   }
}

/**
 * Sets a new pattern list.
 *
 * @param patterns		the patterns to search
 * @param caseSensivity	CaseSensitive or CaseInsensitive
 * @param anchored		<code>true</code>: the pattern must match the string's start<br>
 *						<code>false</code>: the pattern can match anywhere
 */
void ReListMatcher::setPatterns(const QStringList& patterns,
                                Qt::CaseSensitivity caseSensivity, bool anchored) {
   destroy();
   m_patterns = patterns;
   m_empty = true;
   m_allMatching = false;
   QStringList::const_iterator it;
   for (it = patterns.constBegin(); it != patterns.cend(); ++it) {
      ReMatcher* matcher = new ReMatcher(*it, caseSensivity, anchored);
      m_list.append(matcher);
      if (matcher->allMatching())
         m_allMatching = true;
      m_empty = false;
   }
}

/**
 * Constructor.
 *
 * @param patterns			a comma delimited pattern list, exclude patterns
 *							marked with the prefix '-'. Example: "*.c,*.h,-test*'
 * @param caseSensivity		CaseSensitive or CaseInsensitive
 * @param anchored			<code>true</code>: the pattern starts at the strings
 *							start<br>
 *							<code>false</code>: the pattern can start anywhere in
 *							the string
 */
ReIncludeExcludeMatcher::ReIncludeExcludeMatcher(const QString& patterns,
      Qt::CaseSensitivity caseSensivity, bool anchored) :
   m_includes(ReQStringUtils::m_emptyList, caseSensivity, anchored),
   m_excludes(ReQStringUtils::m_emptyList, caseSensivity, anchored) {
   setPatterns(patterns);
}

/**
 * Constructor.
 *
 * @param includes			a list of include patterns
 * @param excludes			a list of exclude patterns
 * @param caseSensivity		CaseSensitive or CaseInsensitive
 * @param anchored			<code>true</code>: the pattern starts at the strings
 *							start<br>
 *							<code>false</code>: the pattern can start anywhere in
 *							the string
 */
ReIncludeExcludeMatcher::ReIncludeExcludeMatcher(const QStringList& includes,
      const QStringList& excludes, Qt::CaseSensitivity caseSensivity,
      bool anchored) :
   m_includes(includes, caseSensivity, anchored),
   m_excludes(excludes, caseSensivity, anchored) {
}

/**
 * Returns the singleton instance of a matcher matching all strings.
 *
 * @return a matcher matching all strings
 */
const ReIncludeExcludeMatcher& ReIncludeExcludeMatcher::allMatcher() {
   if (m_allMatcher == NULL) {
      QStringList includes;
      includes.append("*");
      QStringList excludes;
      m_allMatcher = new ReIncludeExcludeMatcher(includes, excludes);
   }
   return *m_allMatcher;
}

/**
 * Returns the case sensitivity of the pattern matching.
 *
 * @return	<code>true</code>: the character case is relevant
 */
Qt::CaseSensitivity ReIncludeExcludeMatcher::caseSensivitiy() const {
   return m_includes.caseSensivitiy();
}

/**
 * Returns the exclude matcher.
 * @return the exclude matcher
 */
const ReListMatcher& ReIncludeExcludeMatcher::excludes() const {
   return m_excludes;
}

/**
 * Returns the include matcher.
 * @return the include matcher
 */
const ReListMatcher& ReIncludeExcludeMatcher::includes() const {
   return m_includes;
}

/**
 * Tests whether a text matches the include patterns and does not match the
 * exclude patterns.
 *
 * @param text			text to test
 * @param excludeToo	<code>true</code>: the exclude patterns will be tested
 * @return				<code>true</code>: at least one of the include patterns
 *						matches and none of the exclude patterns matches
 */
bool ReIncludeExcludeMatcher::matches(const QString& text, bool excludeToo) const {
   bool rc = m_includes.matches(text);
   if (rc && excludeToo && !m_excludes.empty())
      rc = !m_excludes.matches(text);
   return rc;
}

/**
 * Sets the case sensitivity of the pattern matching.
 *
 * @param caseSensivitiy	<code>true</code>: the character case is relevant
 */
void ReIncludeExcludeMatcher::setCaseSensivitiy(
   const Qt::CaseSensitivity& caseSensivitiy) {
   m_includes.setCaseSensivitiy(caseSensivitiy);
   m_excludes.setCaseSensivitiy(caseSensivitiy);
}

/**
 * Sets the patterns from a string.
 *
 * @param patterns		a pattern list, delimited by <code>separator</code>
 * @param separator		separates the patterns
 * @param excludeMarker	if a pattern starts with this character, the pattern
 *						is a excluding pattern
 */
void ReIncludeExcludeMatcher::setPatterns(const QString& patterns,
      QChar separator, QChar excludeMarker) {
   QStringList includes;
   QStringList excludes;
   int start = 0;
   int ix;
   while( (ix = patterns.indexOf(separator, start)) >= 0) {
      int length = ix - start;
      if (length > 0) {
         if (patterns.at(start) == excludeMarker) {
            if (length > 1)
               excludes.append(patterns.mid(start, length - 1));
         } else {
            includes.append(patterns.mid(start, length));
         }
      }
      start = ix + 1;
   }
   if (patterns.length() > start) {
      if (patterns.at(start) == excludeMarker)
         excludes.append(patterns.mid(start + 1));
      else
         includes.append(patterns.mid(start));
   }
   m_includes.setPatterns(includes, m_includes.caseSensivitiy(),
                          m_includes.anchored());
   m_excludes.setPatterns(excludes, m_excludes.caseSensivitiy(),
                          m_excludes.anchored());
}

