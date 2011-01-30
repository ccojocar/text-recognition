/*
 * Copyright 2008   Cosmin Cojocar <cosmin.cojocar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "QSmartTextRecognition.h"

QSmartTextRecognition::QSmartTextRecognition()
  : m_mapEntries()
  , m_mapBMEntriesTables()
{
}

QSmartTextRecognition::~QSmartTextRecognition()
{
  QMap<int, BMTables>::const_iterator it;

  for (it = m_mapBMEntriesTables.begin(); it != m_mapBMEntriesTables.end();
       ++it) {
    if (it.value().goodSufixTable != 0) {
      delete[] it.value().goodSufixTable;
    }
  }
}

QString QSmartTextRecognition::stripBlanks(const QString str)
{
  int prev = 0;
  int strSize = str.size();
  QString stripedStr;
  QChar blank(' ');

  // remove multiple blanks from string
  for (int i=0; i< strSize; ++i) {
    if ((str[i] != blank) || (str[prev] != blank)) {
      stripedStr.append(str[i]);
    }
    prev = i;
  }

  // remove trailings space if there are any
  if ((stripedStr.isEmpty() == false) &&
     (stripedStr[stripedStr.size() - 1] == blank)) {
    stripedStr.remove(stripedStr.size() - 1, 1);
  }

  return stripedStr;
}

void QSmartTextRecognition::addEntry(int key, const QString& str)
{
  QString stripedStr = stripBlanks(str).toLower();
  int stripedStrSize = stripedStr.size();

  if (stripedStrSize > 0) {
    // store the provided entry after striping the blanks
    m_mapEntries.insert(key, stripedStr);

    // process the bad character shift table
    BMTables bmTables;
    bmTables.goodSufixTable = new int[stripedStrSize];
    for (int i = 0; i < MAX_UNICODE; i++) {
      bmTables.badCharTable[i] = -1;
    }
    for (int i = 0; i < stripedStrSize - 1; ++i) {
      int j = stripedStr[i].unicode();
      if(j < MAX_UNICODE) {
        bmTables.badCharTable[j] = i;
      }
      bmTables.goodSufixTable[i] = stripedStrSize;
    }
    bmTables.goodSufixTable[stripedStrSize - 1] = stripedStrSize;

    // process good the suffix table
    for (int i = 0; i < stripedStrSize - 1; ++i) {
      int j = 0;
      bool isMatch = true;
      while ((isMatch) && (i - 1 >= 0) && (i - j >=0)) {
	isMatch = (stripedStr[i - j] == stripedStr[stripedStrSize - j - 1]);
        if (!isMatch)
          bmTables.goodSufixTable[stripedStrSize - j - 1] =
	      stripedStrSize - i - 1;
        ++j;
      }
      if ((isMatch) && ((i - j) < 0)) {
        while (j < stripedStrSize) {
          bmTables.goodSufixTable[stripedStrSize - j - 1] =
	      stripedStrSize - i -1;
          ++j;
        }
      }
    }

    // store the Boyer-Moore tables
    m_mapBMEntriesTables.insert(key, bmTables);
  }
}

void QSmartTextRecognition::addEntries(const QMap<int, QString>& entries)
{
  QMap<int, QString>::const_iterator it;
  for (it = entries.begin(); it != entries.end(); ++it) {
    addEntry(it.key(), it.value());
  }
}

void QSmartTextRecognition::removeEntry(int key)
{
  m_mapEntries.remove(key);
  if (m_mapBMEntriesTables[key].goodSufixTable != 0) {
    delete[] m_mapBMEntriesTables[key].goodSufixTable;
  }
  m_mapBMEntriesTables.remove(key);
}

void QSmartTextRecognition::removeEntries(const QList<int>& keys)
{
  QList<int>::const_iterator it;
  for (it = keys.begin(); it != keys.end(); ++it) {
    removeEntry(*it);
  }
}

QList<MatchResult> QSmartTextRecognition::matchText(const QString text) const
{
  int textSize = text.size();
  QMap<int, QString>::const_iterator it;
  QList<MatchResult> matchResults;
  QString tmpText = text.toLower();

  for (it = m_mapEntries.begin(); it != m_mapEntries.end(); ++it) {
    int entrySize = it.value().size();
    int i = 0;
    while (i <= (textSize - entrySize)) {
      int j = entrySize - 1;
      QChar prev('a'); // initialize with a non blank char
      int numBlanks = 0;
      while ((it.value()[j] == tmpText[i + j]) || (prev == QChar(' '))) {
        if (j == 0) {
          //take into account only complete tokens
          if (((i == 0) || (tmpText[i - 1] == QChar(' '))) &&
             ((i + numBlanks + entrySize == textSize) ||
              (tmpText[i + numBlanks + entrySize] == QChar(' ')))) {
            MatchResult matchResult;
            matchResult.startPosition = i;
            matchResult.endPosition = i + numBlanks + entrySize -1;
            matchResult.keys.push_back(it.key());
            matchResults.push_back(matchResult);
          }
          break;
        }
        // this is needed for skipping multiple blanks from provided text
        if (prev == QChar(' ') && (tmpText[i + j] == QChar(' '))) {
          prev = tmpText[i + j];
          --i;
          ++numBlanks;
        } else {
          prev = tmpText[i + j];
          --j;
        }
      }
      // stop the application if an invalid character is encountered.
      if (tmpText[i + j].unicode() >= MAX_UNICODE) {
        qFatal("An invalid character has been encountered \
          by the complete matching algorithm.");
      }

      i += max(m_mapBMEntriesTables[it.key()].goodSufixTable[j],
        (j - m_mapBMEntriesTables[it.key()].badCharTable[tmpText[i + j].unicode()]));
    }
  }

  processMatchResults(matchResults);

  return matchResults;
}

QList<PartialMatchResult>
QSmartTextRecognition::partialMatch(const QString text) const
{
  QMap<int, QString>::const_iterator it;
  QList<PartialMatchResult> partialMatchResults;
  QString tmpText = text;
  int textSize = text.size();

  // remove the trailing blanks
  int numTrailingBlanks = 0;
  while (tmpText[tmpText.size() - 1] == QChar(' ')) {
    ++numTrailingBlanks;
    tmpText.remove(tmpText.size() - 1 , 1);
  }
  textSize = tmpText.size();
  for (it = m_mapEntries.begin(); it != m_mapEntries.end(); ++it) {
    int entrySize = it.value().size();
    int i = textSize - 1;
    int j = 0;
    int numBlanks = 0;
    QChar prev('a'); // default non blank value
    while ((i >= 0) && (i >= textSize - entrySize - numBlanks)) {
      if (it.value()[j] != tmpText[i]) {
        if ((tmpText[i] == QChar(' ')) && (prev == QChar(' '))) {
          ++numBlanks;
        }
        prev = tmpText[i];
        --i;
      } else {
        int k = i;
        prev = QChar('a'); // default non blank value
        while ((k < textSize) && (j < entrySize)) {
          //skip multiple blanks from given text
          if ((tmpText[k] == QChar(' ')) && (prev == QChar(' '))) {
            prev = tmpText[k];
            ++k;
          } else {
            if (it.value()[j] == tmpText[k]) {
              prev = tmpText[k];
              ++k;
              ++j;
            } else {
              --i;
              j = 0;
              break;
            }
          }
        }
        // partial match has been found
        if (k == textSize) {
          bool isValidEntry = true;
          if (numTrailingBlanks > 0) {
            if (j < entrySize) {
              if (it.value()[j] == QChar(' ')) {
                ++j;
              } else {
                isValidEntry = false;
              }
            }
          }
          if (isValidEntry) {
            PartialMatchResult partialMatchResult;
            partialMatchResult.numMatchedChars = j;
            partialMatchResult.numCompletingChars =
	      textSize - i + numTrailingBlanks;
            partialMatchResult.entry = it.value();

            //store the found entry
            partialMatchResults.push_back(partialMatchResult);
          }
          break;
        }
      }
    }
  }

  // store the partial matching results based on number of characters of the
  // provided text that are being completed
  qSort(partialMatchResults.begin(), partialMatchResults.end(),
      lessThanForPartialMatchResult);

  // remove the equivalent strings
  processPartialMatchResults(partialMatchResults, numTrailingBlanks);

  return partialMatchResults;
}

void QSmartTextRecognition::processMatchResults(
    QList<MatchResult>& matchResults) const
{
  int i = 0;

  // sorting the matching results after start position
  qSort(matchResults.begin(), matchResults.end(), lessThanForMatchResult);
  while (i < matchResults.size() - 1) {
    // merge the same entries in only one match result
    if ((matchResults[i].startPosition == matchResults[i + 1].startPosition) &&
       (matchResults[i].endPosition   == matchResults[i + 1].endPosition)) {
      matchResults[i].keys.push_back(*matchResults[i + 1].keys.begin());
      matchResults.removeAt(i + 1);
    }
    // check if matchResults[i + 1] is sub entry of matchResults[i]
    else if ((matchResults[i].startPosition <= matchResults[i + 1].startPosition) &&
      (matchResults[i].endPosition   >= matchResults[i + 1].endPosition)) {
      matchResults.removeAt(i + 1);
    }
    // check if matchResults[i] is sub entry of matchResults[i + 1]
    else if ((matchResults[i].startPosition >= matchResults[i + 1].startPosition) &&
      (matchResults[i].endPosition   <= matchResults[i + 1].endPosition)) {
      matchResults.removeAt(i);
      ++i;
    }
    // check if matchResults[i + 1] overlaps matchResults[i]
    else if ((matchResults[i].startPosition <= matchResults[i + 1].startPosition) &&
      (matchResults[i].endPosition   >= matchResults[i + 1].startPosition)) {
      matchResults.removeAt(i + 1);
    } else {
      ++i;
    }
  }
}

void QSmartTextRecognition::processPartialMatchResults(
    QList<PartialMatchResult>& partialMatchResults, int numTrailingBlanks) const
{
  int i = 0;
  while (i < partialMatchResults.size()) {
    int j = i + 1;
    bool isValidEntry = false;
    while (j < partialMatchResults.size()) {
      if (partialMatchResults[i].entry.compare(partialMatchResults[j].entry)
	  == 0) {
        partialMatchResults.removeAt(j);
      } else {
        if (numTrailingBlanks > 0) {
          if (partialMatchResults[i].numMatchedChars <
	      partialMatchResults[j].numMatchedChars) {
            isValidEntry = true;
          }
          ++j;
        } else {
          ++j;
        }
      }
    }
    if ((!isValidEntry) && (numTrailingBlanks > 0) &&
       (partialMatchResults[i].numMatchedChars ==
	partialMatchResults[i].entry.size())) {
      partialMatchResults.removeAt(i);
    } else {
      ++i;
    }
  }
}

bool QSmartTextRecognition::lessThanForMatchResult(
    const MatchResult& matchResultLeft,
    const MatchResult& matchResultRight)
{
  return matchResultLeft.startPosition < matchResultRight.startPosition;
}

bool QSmartTextRecognition::lessThanForPartialMatchResult(
    const PartialMatchResult& partialMatchResultLeft,
    const PartialMatchResult& partialMatchResultRight)
{
  return partialMatchResultLeft.numMatchedChars <
    partialMatchResultRight.numMatchedChars;
}
