#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Document {
  int16_t docID;
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
  int16_t totalDocument = 0;
  int totalTerms = 0;
  size_t index = 0;

  std::unordered_map<std::string, std::unordered_map<int16_t, int16_t>>
      dictionary;
  std::unordered_map<std::string, int16_t> collectionFrequency;
  std::unordered_map<int16_t, int> docLength;
  std::vector<std::string> docTitles;

  void calculateCollectionFrequency();
  void addWord(const std::string &word, int16_t docID);
  void readSegment(const std::string &line, bool forward);

  Document getContent(const std::string &line);
  std::vector<std::string> splitQuery(const std::string &query);
};
