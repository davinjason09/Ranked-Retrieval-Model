#include "InvertedIndex.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

InvertedIndex::InvertedIndex(const std::string &filePath)
    : filePath(filePath) {}

InvertedIndex::~InvertedIndex() {
  collectionFrequency.clear();
  dictionary.clear();
  docLength.clear();
  docTitles.clear();
}

void InvertedIndex::createIndex() {
  std::ifstream file(filePath);
  std::string line;
  int docID = 0;

  if (!file.is_open()) {
    std::cerr << "Error: File not found\n";
    return;
  }

  std::getline(file, line);
  while (std::getline(file, line)) {
    Document doc = getContent(line);

    std::string word = "";
    for (char c : doc.content) {
      if (isalnum(c)) {
        word += tolower(c);
      } else {
        if (!word.empty())
          addWord(word, doc.docID);

        word = "";
      }
    }

    if (!word.empty())
      addWord(word, doc.docID);

    docID++;
  }

  totalDocument = docID;
  calculateCollectionFrequency();
}

void InvertedIndex::readSegment(const std::string &line, bool forward = true) {
  bool inQuotes = false;
  bool condition = (forward ? index < line.size() : index > 0);

  current.clear();
  while (condition) {
    char c = line[index];

    if (c == ',' && !inQuotes) {
      forward ? index++ : index--;
      return;
    }

    if (c == '"') {
      inQuotes = !inQuotes;
      forward ? index++ : index--;

      continue;
    }

    current.push_back(c);
    forward ? index++ : index--;
  }
}

Document InvertedIndex::getContent(const std::string &line) {
  index = 0;
  readSegment(line);
  int id = std::stoi(current);

  readSegment(line);
  readSegment(line);
  std::string title = current;

  index = line.size() - 1;
  readSegment(line, false);
  readSegment(line, false);
  std::string content = current;
  std::reverse(content.begin(), content.end());

  if (id != docTitles.size())
    docTitles.push_back("---");

  docTitles.push_back(title);
  return Document(id, content);
}

void InvertedIndex::addWord(const std::string &word, int16_t docID) {
  dictionary[word][docID]++;
  collectionFrequency[word]++;
  docLength[docID]++;
  totalTerms++;
}

void InvertedIndex::calculateCollectionFrequency() {
  for (const auto &it : dictionary) {
    collectionFrequency[it.first] = 0;
    for (const auto &[_, tf] : it.second) {
      collectionFrequency[it.first] += tf;
    }
  }
}

std::vector<std::string> InvertedIndex::splitQuery(const std::string &query) {
  std::vector<std::string> result;
  std::string word = "";

  for (char c : query) {
    if (isalnum(c)) {
      word += tolower(c);
    } else {
      if (!word.empty())
        result.push_back(word);

      word = "";
    }
  }

  if (!word.empty())
    result.push_back(word);

  return result;
}

void InvertedIndex::executeQuery(const std::string &query, double alpha) {
  std::vector<std::string> inputQuery = splitQuery(query);
  std::vector<std::pair<int16_t, double>> results;
  std::unordered_map<std::string, int16_t> queryWords;
  results.reserve(totalDocument);

  for (auto &word : inputQuery) {
    if (dictionary.count(word) > 0)
      queryWords[word]++;
  }

  if (queryWords.empty()) {
    std::cout << "No result found\n";
    return;
  }

  for (int16_t i = 0; i < totalDocument; i++) {
    double score = 0;

    for (const auto &[word, _] : queryWords) {
      if (dictionary.count(word) > 0) {
        double termInDoc =
            dictionary[word].count(i) > 0
                ? static_cast<double>(dictionary[word][i]) / docLength[i]
                : 0;

        double termInCollection =
            static_cast<double>(collectionFrequency[word]) / totalTerms;

        double termLikelihood =
            alpha * termInDoc + (1 - alpha) * termInCollection;

        if (termLikelihood > 0)
          score += log10(termLikelihood);
      }
    }

    if (score != 0)
      results.push_back({i, score});
  }

  std::sort(results.begin(), results.end(), [](const auto &a, const auto &b) {
    if (a.second == b.second)
      return a.first < b.first;
    return a.second > b.second;
  });

  for (int i = 0; i < std::min(10, static_cast<int>(results.size())); i++)
    std::cout << "ID: " << results[i].first << " - "
              << docTitles[results[i].first] << " - [" << results[i].second
              << "]\n";

  std::cout << "Done\n";
}
