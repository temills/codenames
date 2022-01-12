#!/usr/bin/env python3

# Checks which codenames words are not included in model dictionary (76/402)
# Tracey Mills 1/2022

embedFile = 'DICTIONARY.txt'
codenamesFile = 'codenamesWords.txt'


def fileToList(filename):
    l = []
    with open(filename) as file:
        while (line := file.readline().rstrip()):
            word = str.split(line, ':')
            if len(word)>1:
                word=word[1]
            else:
                word=word[0]
            l.append(word)
    return l

l1 = fileToList(embedFile)
l2 = fileToList(codenamesFile)

count=0
for w in l2:
    if w not in l1:
        print(w)
        count=count+1
print(count)

