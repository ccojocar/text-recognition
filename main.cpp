#include <iostream>
#include <QTime>
#include "QSmartTextRecognition.h"

int main()
{
  QSmartTextRecognition strRecognition;
  QMap<int, QString> entries;
  QString text("Summer fun is very very     good");
  QList<MatchResult> matchResults;
  QList<MatchResult>::const_iterator itMatchResult;
  QList<PartialMatchResult> partialMatchResults;
  QList<PartialMatchResult>::const_iterator itPartialMatchResult;
  QList<int>::const_iterator itKey;
  QTime timer;
  int nMilliseconds = 0;

  entries.insert(100, "sum");
  entries.insert(101, "summer");
  entries.insert(102, "summer fun");
  entries.insert(103, "summer fun");
  entries.insert(104, "summer  fun");
  entries.insert(105, "fun is very");
  entries.insert(106, "very good");
  entries.insert(107, "gold");

  timer.start();
  strRecognition.addEntries(entries);
  nMilliseconds = timer.elapsed();
  std::cout << std::endl << "Added entries in " << timer.elapsed()
    << " milliseconds." << std::endl;

  timer.restart();
  matchResults = strRecognition.matchText(text);
  nMilliseconds = timer.elapsed();
  std::cout << std::endl << "Complete Search in " << timer.elapsed()
    << " milliseconds." << std::endl;
  std::cout << "Result:" << std::endl;

  for (itMatchResult = matchResults.begin(); itMatchResult !=
      matchResults.end(); ++itMatchResult) {
    std::cout << itMatchResult->startPosition << ", "
      << itMatchResult->endPosition << ", {";
    for (itKey = itMatchResult->keys.begin();
	itKey != itMatchResult->keys.end(); ++itKey) {
      std::cout << *itKey;
      if (itKey < itMatchResult->keys.end() -1) {
        std::cout << ", ";
      }
    }
    std::cout << "}" << std::endl;
  }

  text = QString("summer ");
  timer.restart();
  partialMatchResults = strRecognition.partialMatch(text);
  nMilliseconds = timer.elapsed();
  std::cout << std::endl << "Partial Search in " << timer.elapsed()
    << " milliseconds." << std::endl;
  std::cout <<"Result:" << std::endl;
  for (itPartialMatchResult = partialMatchResults.begin(); itPartialMatchResult
      != partialMatchResults.end();
      ++itPartialMatchResult) {
    std::cout << "(" << itPartialMatchResult->numMatchedChars << ", "
      << itPartialMatchResult->entry.toStdString() << ", "
      << itPartialMatchResult->numCompletingChars << ")" << std::endl;
  }

  return 0;
}
