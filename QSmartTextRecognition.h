/*
 * Copyright 2008   Cosmin Cojocar <cosmin.cojocar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef QSMARTTEXTRECOGNITION_H
#define QSMARTTEXTRECOGNITION_H

#include <QList>
#include <QMap>
#include <QString>

namespace {
  /*
   * the maximum unicode value used by the Boyer-Moore algorithm
   */
  const int MAX_UNICODE = 128;
}

/*
 * holds the start position, end position and the list of the keys of
 * matching strings. It is used to return the result of complete matching
 */
struct MatchResult {
  int startPosition;
  int endPosition;
  QList<int> keys;
};

/*
 * holds a string along with the number of characters that already match
 * a given text and the number of characters of the provided text that arei
 * being completed
 */
struct PartialMatchResult {
  int numMatchedChars;
  QString entry;
  int numCompletingChars;
};

/*
 * holds the first and second table used by the Boyer-Moore algorithm
 * for one entry
 */
struct BMTables {
  int  badCharTable[MAX_UNICODE];
  int* goodSufixTable;
};

/*
 * This class maintains a collection of entry strings and provides methods for
 * matching them completely or partially against a provided text string.
 */
class QSmartTextRecognition {

public:
  /*
   * default constructor
   */
  QSmartTextRecognition();

  /*
   * default destructor
   */
   ~QSmartTextRecognition();

  /*
   * adds a new entry in the patterns map. Furthermore it removes the
   * extra blanks from the entry string.
   *
   * @param key  the entry unique identifier in the map
   * @param str  the effective string
   */
  void addEntry(int key, const QString& str);

  /*
   * adds entries for each key, string pair in the map
   *
   * @param entries  the list of pairs key, string which
   *       is added in the map
   */
  void addEntries(const QMap<int, QString>& entries);

  /*
   *removes the entry with specified key from the map
   *
   * @param key  the identifier of the entry which is
   *         intended to be removed
   */
  void removeEntry(int key);

  /*
   *removes an entire list of entries with specified keys from the map
   *
   * @param keys  the identifiers of the entries which
   *    is intended to be removed.
   */
  void removeEntries(const QList<int>& keys);

  /*
   * matches the text against the current entries and provides the
   * specified matches
   *
   * @param text  a string that is intended to be used for matching the
   *              current entries
   * @return  a list of triples holding the start position, end position,
   *          and the list of the keys of the matching strings
   */
  QList<MatchResult> matchText(const QString text) const;

  /*
   * returns a list of strings that could be used to complete the provided text,
   * along with number of characters that already match and the number of
   * characters of the provided text that are being competed, sorted in
   * order of the last number of characters.
   *
   * @param text  a string that is intended to be used for matching the
   *              current entries
   * @return  a list of triples holding the result
   */
  QList<PartialMatchResult> partialMatch(const QString text) const;

private:
  /*
   * strips multiple spaces as well leading and trailing from given string.
   *
   * @param str  a string on which the striping process is applied
   * @return  a string without multiple spaces
   */
  QString stripBlanks(const QString str);

  /*
   * processes the matching results in order to merge similar entries and remove
   * subentries or overlapped entries
   *
   * @param matchResults  a list containing match results that should
   *                      be post processed
   */
  void processMatchResults(QList<MatchResult>& matchResults) const;

  /*
   * processes the partial matching results by removing the equivalent entries
   *
   * @param matchResults  a list with partial match results that should
   *			  be post processed.
   * @param nrTrailingBlanks  the number of blanks presented at the end of
   *			      provided text
   */
  void processPartialMatchResults(QList<PartialMatchResult>& partialMatchResults,
                int numTrailingBlanks) const;

  /*
   * compute the maximum of the given numbers
   */
  inline int max(int a, int b) const { return (a > b) ? a : b; }

  /*
   * less than operators for quick search
   */
  static bool lessThanForMatchResult(const MatchResult& matchResultLeft,
				     const MatchResult& matchResultRight);
  static bool lessThanForPartialMatchResult(
	const PartialMatchResult& partialMatchResultLeft,
	const PartialMatchResult& partialMatchResultRight);

  /*
   * keeps a collection of entries used for matching against
   * a provided text string
   */
  QMap<int, QString> m_mapEntries;

  /*
   * keeps a collection of tables used by the Boyer-Moore algorithm
   * for all entries
   */
  QMap<int, BMTables> m_mapBMEntriesTables;
};

#endif /* QSMARTTEXTRECOGNITION_H */
