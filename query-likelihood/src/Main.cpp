#include "InvertedIndex.h"
#include <atomic>
#include <iostream>
#include <thread>

std::atomic<bool> indexReady(false);
double indexTime;
double queryTime;

void buildIndex(InvertedIndex *index) {
  auto start = std::chrono::high_resolution_clock::now();
  index->createIndex();
  auto end = std::chrono::high_resolution_clock::now();

  indexTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count();
  indexReady = true;
}

int main(int argc, char *argv[]) {
  InvertedIndex index("../../data/News.csv");

  std::thread indexThread(buildIndex, &index);

  std::string query;
  std::cout << "Enter your query: ";
  std::getline(std::cin, query);

  if (!indexReady)
    std::cout << "Index is not ready yet. Please wait.\n";

  indexThread.join();

  auto start = std::chrono::high_resolution_clock::now();
  index.executeQuery(query);
  auto end = std::chrono::high_resolution_clock::now();

  queryTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count();

  std::cout << "\nIndex created in " << indexTime << " ms\n";
  std::cout << "Query executed in " << queryTime << " ms\n";
}
