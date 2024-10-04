#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Document {
  int16_t docID; // documents is less than 2^16
  std::string content;

  Document(int docID, std::string content) : docID(docID), content(content) {}
};

class InvertedIndex {
public:
  InvertedIndex(const std::string &filePath);
  ~InvertedIndex();

  void createIndex();
  void executeQuery(const std::string &query);

private:
  std::string filePath;
  std::string current;
  int16_t totalDocument = 0;
  size_t index = 0;

  std::unordered_map<std::string,
                     std::unordered_map<int16_t, std::pair<int16_t, double>>>
      dictionary;
  std::unordered_map<std::string, double> IDF;
  std::vector<std::string> docTitles;

  void calculateTFIDF();
  void addWord(const std::string &word, int16_t docID);
  void readSegment(const std::string &line, bool forward);

  Document getContent(const std::string &line);
  std::vector<std::string> splitQuery(const std::string &query);
};
