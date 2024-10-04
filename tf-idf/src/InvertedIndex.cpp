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
  dictionary.clear();
  docTitles.clear();
  IDF.clear();
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
  calculateTFIDF();
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
  if (dictionary[word].count(docID) == 0) {
    dictionary[word][docID] = {1, 0};
  } else {
    dictionary[word][docID].first++;
  }
}

void InvertedIndex::calculateTFIDF() {
  for (auto &[word, docs] : dictionary) {
    double idf = log10(static_cast<double>(totalDocument) / docs.size());
    IDF[word] = idf;

    for (auto &[_, details] : docs) {
      int16_t freq = details.first;
      details.second = (freq > 0) ? (1 + log10(freq)) * idf : 0;
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

void InvertedIndex::executeQuery(const std::string &query) {
  std::vector<std::string> inputQuery = splitQuery(query);
  std::unordered_map<std::string, int> queryWords;
  std::vector<std::pair<int, double>> results;
  results.reserve(totalDocument);

  for (auto &word : inputQuery) {
    if (dictionary.count(word) > 0)
      queryWords[word]++;
  }

  if (queryWords.empty()) {
    std::cout << "No result found\n";
    return;
  }

  double queryLength = 0;
  std::unordered_map<std::string, double> queryWeights;
  for (const auto &[word, freq] : queryWords) {
    double idf = IDF[word];
    double tfidf = (1 + log10(freq)) * idf;
    queryWeights[word] = tfidf;
    queryLength += tfidf * tfidf;
  }

  queryLength = std::sqrt(queryLength);

  for (int i = 0; i < totalDocument; i++) {
    double dotProduct = 0;
    double docWeight = 0;

    for (const auto &[word, _] : queryWords) {
      if (dictionary[word].count(i) > 0) {
        auto [freq, tfidf] = dictionary[word][i];
        dotProduct += tfidf * queryWeights[word];
        docWeight += tfidf * tfidf;
      }
    }

    if (dotProduct > 0) {
      docWeight = std::sqrt(docWeight);
      double cosineSimilarity = dotProduct / (docWeight * queryLength);

      results.push_back({i, cosineSimilarity});
    }
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
