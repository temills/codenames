#!/bin/bash

rm -rf ./build
rm fast_inference.cpp
rm fast_inference.cpython*

python setup.py build_ext --inplace

CC=g++ python setup.py build_ext --inplace

echo

python getWordSim.py wordSim.test stopwords setting.txt wordVectors200.pwe wordVectors200.pi wordVectors200.beta DICTIONARY.txt 5 final.theta 5

#python dynamicComputeWordSemanticsBasedonContexts.py wordsWithContexts.test stopwords setting.txt wordVectors200.pwe wordVectors200.pi wordVectors200.beta DICTIONARY.txt 5 final.theta 5

