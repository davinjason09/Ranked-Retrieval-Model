#include "InvertedIndex.h"
#include <iostream>

int main(int argc, char *argv[]) {
  InvertedIndex indexNew("../data/News.csv");
  indexNew.createIndex();

  std::cout << "Index created successfully\n";

  std::string query;
  std::cout << "Enter your query: ";
  std::getline(std::cin, query);

  indexNew.executeQuery(query);
}
