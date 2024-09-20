#include "InvertedIndex.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void InvertedIndex::ReadCSV(const std::string &filePath) {
  std::ifstream file(filePath);

  if (!file.is_open()) {
    std::cerr << "Error: file not found" << std::endl;
    return;
  }

  std::string line;

  std::getline(file, line);

  auto start = std::chrono::high_resolution_clock::now();
  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    auto [id, content] = ParseCSVLine(line);
    AddToIndex(content, std::stoi(id));
  }

  auto end = std::chrono::high_resolution_clock::now();
  double elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  std::cout << "Time to read and parse CSV: " << elapsed << "ms\n";
}

std::pair<std::string, std::string>
InvertedIndex::ParseCSVLine(const std::string &line) {
  typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
  Tokenizer tok(line, boost::escaped_list_separator<char>('\n', ',', '\"'));
  std::vector<std::string> tokens(tok.begin(), tok.end());

  if (tokens.size() < 3) {
    std::cerr << "Error: invalid CSV line\n";
    return {};
  }

  return {tokens[0], tokens[7]};
}

void InvertedIndex::AddToIndex(const std::string &content, int documentID) {
  boost::regex re("[a-zA-Z0-9]+");
  boost::sregex_token_iterator it(content.begin(), content.end(), re, -1);
  boost::sregex_token_iterator end;

  for (; it != end; ++it) {
    std::string token = *it;
    boost::to_lower(token);
    TermFrequency[token]++;
    DocumentFrequency[token][documentID]++;
  }
}
