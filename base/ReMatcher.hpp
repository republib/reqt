/*
 * ReMatcher.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REMATCHER_HPP
#define REMATCHER_HPP

/**
 * Processor for an efficient test whether a text matches a pattern.
 *
 * A pattern can contain wildcards: '*' for any string.
 */
class ReMatcher {
public:
   ReMatcher(const QString& pattern, Qt::CaseSensitivity caseSensitivty =
                Qt::CaseSensitive, bool anchored = false);
public:
   bool allMatching() const;
   bool anchored() const;
   Qt::CaseSensitivity caseSensivitiy() const;
   bool matches(const QString& text);
   const QString& pattern() const;
   void setAnchored(bool anchored);
   void setCaseSensivitiy(const Qt::CaseSensitivity& caseSensivitiy);
   void setPattern(const QString& pattern, bool anchored = false);
protected:
   QString m_pattern;
   QStringList m_needles;
   // m_restLengths[ix] = sum(m_needles[ix..last].size()
   QList<int> m_restLengths;
   bool m_anchored;
   Qt::CaseSensitivity m_caseSensivitiy;
   bool m_allMatching;
};

/**
 * Processor for efficient test whether a text matches a list of patterns.
 */
class ReListMatcher {
public:
   ReListMatcher(const QStringList& patterns,
                 Qt::CaseSensitivity caseSensitivty = Qt::CaseSensitive, bool anchored =
                    false);
   ~ReListMatcher();
public:
   bool allMatching() const;
   Qt::CaseSensitivity caseSensivitiy() const;
   bool empty() const;
   bool matches(const QString& text) const;
   const QStringList& patterns() const;
   void setCaseSensivitiy(const Qt::CaseSensitivity& caseSensivitiy);
   void setPatterns(const QStringList& patterns,
                    Qt::CaseSensitivity caseSensivity = Qt::CaseSensitive, bool anchored =
                       false);
public:
   bool anchored() const;
   static const ReListMatcher& allMatcher();
   static const QStringList& allMatchingList();
protected:
   void destroy();
private:
   static QStringList* m_allMatchingList;
   static ReListMatcher* m_allMatcher;
protected:
   QStringList m_patterns;
   QList<ReMatcher*> m_list;
   bool m_empty;
   bool m_allMatching;
};

/**
 * Processor for efficient test whether a text matches an include pattern list
 * and not an exclude pattern list.
 */
class ReIncludeExcludeMatcher {
public:
   ReIncludeExcludeMatcher(const QString& pattern = ReQStringUtils::m_empty,
                           Qt::CaseSensitivity caseSensitivty = Qt::CaseSensitive,
                           bool anchored = false);
   ReIncludeExcludeMatcher(const QStringList& includes,
                           const QStringList& excludes, Qt::CaseSensitivity caseSensitivty =
                              Qt::CaseSensitive, bool anchored = false);
public:
   Qt::CaseSensitivity caseSensivitiy() const;
   bool matches(const QString& text, bool excludeToo = true) const;
   const ReListMatcher& includes() const;
   const ReListMatcher& excludes() const;
   void setCaseSensivitiy(const Qt::CaseSensitivity& caseSensivitiy);
   void setPatterns(const QString& patterns, QChar separator = ',',
                    QChar excludeMarker = '-');
public:
   static const ReIncludeExcludeMatcher& allMatcher();
private:
   static ReIncludeExcludeMatcher* m_allMatcher;
protected:
   ReListMatcher m_includes;
   ReListMatcher m_excludes;
};

#endif // REMATCHER_HPP
