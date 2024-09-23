#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct Compare {
  bool operator()(const std::pair<int, double> &a,
                  const std::pair<int, double> &b) {
    return a.second > b.second;
  }
};

struct Document {
  int docID;
  std::string content;

  Document(int docID, std::string content) : docID(docID), content(content) {}
};

class InvertedIndex {
public:
  InvertedIndex(const std::string &filePath);
  ~InvertedIndex();

  void createIndex();
  void executeQuery(const std::string &query, double alpha = 0.5);

private:
  std::string filePath;
  std::string current;
  int totalDocument = 0;
  int totalTerms = 0;
  size_t index = 0;

  std::unordered_map<std::string, std::unordered_map<int, int>> dictionary;
  std::unordered_map<std::string, int> collectionFrequency;
  std::unordered_map<int, double> docLength;
  std::vector<std::string> docTitles;

  void calculateCollectionFrequency();
  void addWord(const std::string &word, int docID);
  void readSegment(const std::string &line, bool forward);

  Document getContent(const std::string &line);
  std::vector<std::string> splitQuery(const std::string &query);
};
