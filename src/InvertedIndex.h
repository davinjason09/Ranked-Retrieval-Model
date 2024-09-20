#pragma once

#include <boost/unordered_map.hpp>
#include <string>
#include <utility>
#include <vector>

class InvertedIndex {
public:
  void ReadCSV(const std::string &filePath);
  std::pair<std::string, std::string> ParseCSVLine(const std::string &line);
  void AddToIndex(const std::string &content, int documentID);
  void TFIDF(std::vector<std::string> query);

private:
  boost::unordered_map<std::string, float> IDFScore;
  boost::unordered_map<std::string, boost::unordered_map<int, int>>
      DocumentFrequency;
  boost::unordered_map<std::string, int> TermFrequency;
};
