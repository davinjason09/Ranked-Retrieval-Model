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

InvertedIndex::~InvertedIndex() {
  dictionary.clear();
  collectionFrequency.clear();
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

void InvertedIndex::addWord(const std::string &word, int docID) {
  /*if (dictionary[word].count(docID) == 0) {*/
  /*  dictionary[word][docID] = {1, 0};*/
  /*} else {*/
  /*  dictionary[word][docID].first++;*/
  /*}*/

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

void InvertedIndex::executeQuery(const std::string &query) {
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

  const double lambda = 0.5;

  for (int i = 0; i < totalDocument; i++) {
    double score = 0;

    for (const auto &word : queryWords) {
      if (dictionary.count(word) > 0) {
        double termInDoc =
            dictionary[word].count(i) > 0
                ? static_cast<double>(dictionary[word][i]) / docLength[i]
                : 0;

        double termInCollection =
            static_cast<double>(collectionFrequency[word]) / totalTerms;

        double termLikelihood =
            lambda * termInDoc + (1 - lambda) * termInCollection;

        score += log(termLikelihood);
      }
    }

    /*std::cout << "Score: " << score << " - ID: " << i << '\n';*/
    if (score != 0) {
      results.push({i, score});

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
