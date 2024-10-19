# Ranked Retrieval Model

A simple CLI-based Ranked Retrieval Model implementation. This repository implements the Vector Space Model using TF-IDF with Cosine Similarity as the similarity measure and a Query Likelihood Model using Jelinek-Mercer Smoothing.
The code is written in C++ with only using STL.

## Usage

The code for each model is in its respective folder. 
```bash
# go to the desired model directory
cd tf-idf/src

#or 
cd query-likelihood/src
```

You can run the C++ implementation using the following commands: 
```bash
g++ -O3 -mtune=native -march=native BooleanRetrieval.cpp InvertedIndex.cpp Main.cpp -o Main -pthread
./Main  # Linux
./Main.exe  # Windows
```

or you can use the CMake build system:
```bash
soon
```

## Speed Comparison

Query retrieval times are dependent on the query and the number of documents in the collection. On average, the vector space model is slightly faster than the query likelihood model. Although keep in mind that the code have yet to be optimized.

## Future Work

The inverted index creation is barely optimized, there are still a lot of hash collision and a lot of move and copy operations that can be avoided. The code can be optimized further to improve the index building time.
