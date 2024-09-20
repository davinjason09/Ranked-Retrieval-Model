#include "InvertedIndex.h"

int main(int argc, char *argv[]) {

  InvertedIndex index;

  std::string filePath = "../data/News.csv";
  index.ReadCSV(filePath);

  return 0;
}
