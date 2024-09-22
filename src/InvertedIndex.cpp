#include "InvertedIndex.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

InvertedIndex::InvertedIndex(const std::string &filePath)
    : filePath(filePath) {}

InvertedIndex::~InvertedIndex() { dictionary.clear(); }

void InvertedIndex::createIndex() {
  std::ifstream file(filePath);
  std::string line;
  int docID = 0;

  if (!file.is_open()) {
    std::cerr << "Error: File not found\n";
    return;
  }

  std::getline(file, line);
  auto start = std::chrono::high_resolution_clock::now();
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

  auto end = std::chrono::high_resolution_clock::now();
  double duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();

  std::cout << "Index created in " << duration << " ms\n";

  totalDocument = docID;
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

void InvertedIndex::addWord(const std::string &word, int docID) {
  if (dictionary[word].count(docID) == 0) {
    dictionary[word][docID] = {1, 0};
  } else {
    dictionary[word][docID].first++;
  }
}

void InvertedIndex::calculateTF() {
  for (auto &it : dictionary) {
    for (auto &node : it.second) {
      double tf = 1 + log10(static_cast<double>(node.second.first));
      node.second.second = tf;

      docLength[node.first] += tf * tf;
    }
  }

  for (auto [id, length] : docLength) {
    length = std::sqrt(length);
  }
}

void InvertedIndex::calculateIDF() {
  for (auto &it : dictionary) {
    double idf = log10(static_cast<double>(totalDocument) / it.second.size());
    IDF[it.first] = idf;
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
  calculateTF();
  calculateIDF();

  std::vector<std::string> inputQuery = splitQuery(query);
  std::priority_queue<std::pair<int, double>,
                      std::vector<std::pair<int, double>>, Compare>
      results;
  std::vector<std::string> queryWords;

  for (auto &word : inputQuery) {
    if (dictionary.count(word) > 0)
      queryWords.push_back(word);
  }

  if (queryWords.empty()) {
    std::cout << "No result found\n";
    return;
  }

  double queryLength = 0;
  std::unordered_map<std::string, double> queryWeights;
  for (const auto &word : queryWords) {
    double idf = IDF[word];
    queryWeights[word] = idf;
    queryLength += idf * idf;
  }

  queryLength = std::sqrt(queryLength);

  for (int i = 0; i < totalDocument; i++) {
    double dotProduct = 0;

    for (const auto &word : queryWords) {
      if (dictionary[word].count(i) > 0) {
        auto [freq, tf] = dictionary[word][i];
        double idf = IDF[word];
        dotProduct += tf * idf * queryWeights[word];
      }
    }

    if (dotProduct > 0) {
      double cosineSimilarity = dotProduct / (docLength[i] * queryLength);
      results.push({i, cosineSimilarity});

      if (results.size() > 10)
        results.pop();
    }
  }

  std::vector<std::pair<int, double>> output;
  while (!results.empty()) {
    output.push_back(results.top());
    results.pop();
  }

  std::reverse(output.begin(), output.end());

  for (const auto &[docID, similarity] : output) {
    std::cout << "ID: " << docID << " - " << docTitles[docID] << " - "
              << similarity << '\n';
  }
}
