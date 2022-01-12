'''
Created on Feb 14, 2017

@author: shuangyinli
'''

from numpy import *
import numpy as np
import time
import sys
from scipy import spatial
import operator
import math
import pwe
from pwe import ANWE
from pwe import  Window
from pwe import Configuration
from pwe import  Model
import re
from copy import deepcopy
from collections import Counter
#from sklearn import cross_validation
from sklearn.model_selection import train_test_split
from sklearn import datasets
from sklearn.svm import SVC
from sklearn.datasets import load_svmlight_file
import random
import multiprocessing
import os
from fast_inference import fast_inference_xi, fast_inference_gamma, fast_compute_doc_likelihood, fast_do_inference, reset as reset_seed

topic_words_dic = {}
dictionary = []
word_topic_dis = {}

stopwords = {}
def readstopwords(stopwordsfile):
    return {}.fromkeys([ line.rstrip() for line in open(stopwordsfile, 'r') ])

def remove(abstract):
    abstract = re.sub("[^A-Za-z.]", " ", abstract)
    
    ###remove a single word
    abstract=re.sub(' [a-zA-Z] ', ' ', abstract)
    abstract=re.sub('^[ ]*[a-zA-Z] ', ' ', abstract)
    abstract=re.sub(' [a-zA-Z][ ]*$', ' ', abstract)

    abstract =  ' '.join(abstract.split())
    abstract = abstract.lower()
    return abstract

def readpwe_theta_pi_beta_dictionary(theta_file, pifile, betafile,vocabularyfile):
    num_semantic = 0
    vocabulary = []
    dictionary = []
    dicsDis ={}
    theta = open(theta_file, "r", encoding = "utf-8")
    pi = open(pifile, "r", encoding = "utf-8")
    vocabularyf = open(vocabularyfile, "r", encoding = "utf-8")
    for vs in vocabularyf:
        vocabulary.append(vs.split(":")[1].strip().lstrip().rstrip())
        
    for ts in theta:
        word = ts.split()[0].strip().lstrip().rstrip()
        dictionary.append(word)
        topicslist = np.array([float(m) for m in ts.split()[1:]], dtype=np.float64)
        num_semantic = len(topicslist)
        dicsDis.setdefault(str(word))
        dicsDis[word] = topicslist

    #newpi = np.array([0.0 for i in range(len(dictionary))])
    newpi = np.zeros(len(dictionary), dtype=np.float64)
    for p in pi:
        newpi[dictionary.index(p.split()[0])] = float(p.split()[1])

    num_words = len(open(betafile, "r").readlines()[0].split())
    log_beta = np.zeros(shape=(num_semantic, num_words), dtype=float64)
    logbetafile = open(betafile, "r").readlines()
    for i in range(len(logbetafile)):
        prolist = logbetafile[i].split()
        for j in range(len(prolist)):
            log_beta[i][j] = float(prolist[j])

    return dictionary, dicsDis, newpi, log_beta, vocabulary

def readkeywordsfromfile(keywordsfile):
    keywordlist = []
    for word in open(keywordsfile, "r").readlines():
        keywordlist.append(word.strip().lstrip().rstrip())
    return keywordlist

def reconstructpwe(settingsfile, pwe_dictionary, pwe_theta, pwe_pi, pwe_log_beta,pwe_vocabulary):
    configuration = Configuration(settingsfile)
    model = Model("", 0, 0, 0,0, 0)
    model.setmodel(pwe_dictionary, pwe_theta, pwe_pi, pwe_log_beta,pwe_vocabulary)
    return configuration, model

def codeTexttopwecode(fulltext, pwe, keyword):
    wordslist = remove(fulltext).split()
    wordlist = []
    dictionary = pwe.model.dictionary
    vocabulary = pwe.model.vocabulary
    for w in wordslist:
        if w not in stopwords:
            if w in dictionary:
                wordlist.append(w)
    #we only keep the words and tags who exist in keywords list.
    if keyword in wordlist:
        wordlist.remove(keyword) # remove keyword
    keywordlist = list(set(wordlist))
    num_keywords = len(keywordlist)
    num_words = len(set(wordlist))
    num_topics = pwe.num_topics
    keywords_ptr = []
    for kw in keywordlist:
        keywords_ptr.append(dictionary.index(kw))
    words_ptr = []
    words_cnt_ptr = []
    for wd, c in Counter(wordlist).items():
        words_ptr.append(vocabulary.index(wd))
        words_cnt_ptr.append(c)
    # ensure keywords_ptr, words_ptr, words_cnt_ptr are numpy array
    keywords_ptr = np.array(keywords_ptr, dtype=np.int32)
    words_ptr = np.array(words_ptr, dtype=np.int32)
    words_cnt_ptr = np.array(words_cnt_ptr, dtype=np.int32)
    #wordlist is a list who contains all the keywords in the context text
    return wordlist, Window(num_keywords, num_words, num_topics, 100, keywords_ptr, words_ptr,words_cnt_ptr)

def inferenceContext(text, pwe):
    context_word_list, doc = codeTexttopwecode(text, pwe, " ")
    pwe.fast_do_inference(doc)
    #pwe.do_inference(doc)
    return doc

def updatewordsemanticwithcontext(word, context, pwe, doc_u_cu):
    num_topics_ = pwe.model.num_topics
    context_word_list, doc_cu = codeTexttopwecode(context, pwe, word)
    context_word_set_non = list(set(context_word_list))
    u_topic = np.array(pwe.model.theta[remove(word)])
    
    pwe.fast_do_inference(doc_cu)
    #pwe.do_inference(doc_cu)
    
    sigma_c_topic = np.array([0.0 for n in range(num_topics_)]) 
    for wordc in context_word_list:
        wdis = np.array(pwe.model.theta[remove(wordc)])
        for i in range(num_topics_):
            sigma_c_topic[i] += wdis[i]
    
    type1_u_topic = np.zeros(num_topics_, dtype=np.float64) # p(u, cu) / p(cu)
    type2_u_topic = np.zeros(num_topics_, dtype=np.float64) # p(u)*p(cu) &
    type3_u_topic = np.zeros(num_topics_, dtype=np.float64) # p(u)*p(u, cu)
    type4_u_topic = np.zeros(num_topics_, dtype=np.float64) # p(u)*sigma_p(u, cu)
    type5_u_topic = np.zeros(num_topics_, dtype=np.float64) # p( t | w_c) = p(t|k) \sigma_N p(w_i | k)  &
    type6_u_topic = np.zeros(num_topics_, dtype=np.float64) # p(w|t_c) = p(w|k) \sigma_T p(t|k)
    type7_u_topic = np.zeros(num_topics_, dtype=np.float64) # p(w|t_c) = p(w|k) \sigma_T p(t|k) \dot \epslon
    type8_u_topic = np.zeros(num_topics_, dtype=np.float64) # p( t | w_c) = p(t|k) \sigma_N p(w_i | k)  \dot \epslon
   
    min_xi = np.min(doc_cu.xi)
    for i in range(num_topics_):
        type1_u_topic[i] = math.exp(doc_u_cu.topic[i]) / math.exp(doc_cu.topic[i])
        type2_u_topic[i] = math.exp(doc_cu.topic[i]) * u_topic[i]
        type3_u_topic[i] = math.exp(doc_u_cu.topic[i]) * u_topic[i]
        type4_u_topic[i] = u_topic[i]*sigma_c_topic[i]
        
        doc_cu_tags_ptr_list = doc_u_cu.tags_ptr.tolist()
        for w in context_word_list:
            wid = pwe.model.vocabulary.index(remove(w))
            widd = pwe.model.dictionary.index(remove(w))
            type5_u_topic[i] += math.exp(pwe.model.log_beta[i][wid])
            
            if widd not in doc_cu_tags_ptr_list:
                type8_u_topic[i] += math.exp(pwe.model.log_beta[i][wid]) * min_xi
            else:
                type8_u_topic[i] += math.exp(pwe.model.log_beta[i][wid]) * doc_u_cu.xi[doc_cu_tags_ptr_list.index(widd)]
        type5_u_topic[i] = type5_u_topic[i]*u_topic[i]
        type8_u_topic[i] = type8_u_topic[i]*u_topic[i]
        
        u_voc_id = pwe.model.vocabulary.index(remove(w))
        for t in context_word_set_non:
            type6_u_topic[i] += np.array(pwe.model.theta[remove(t)])[i]
            type7_u_topic[i] += np.array(pwe.model.theta[remove(t)])[i]*doc_cu.xi[doc_cu.tags_ptr.tolist().index(pwe.model.dictionary.index(remove(t)))]
        type6_u_topic[i] = type6_u_topic[i] * math.exp(pwe.model.log_beta[i][u_voc_id])
        type7_u_topic[i] = type7_u_topic[i] * math.exp(pwe.model.log_beta[i][u_voc_id])
        
    sum_type1_u_topic = np.sum(type1_u_topic)
    sum_type2_u_topic = np.sum(type2_u_topic)
    sum_type3_u_topic = np.sum(type3_u_topic)
    sum_type4_u_topic = np.sum(type4_u_topic)
    sum_type5_u_topic = np.sum(type5_u_topic)
    sum_type6_u_topic = np.sum(type6_u_topic)
    sum_type7_u_topic = np.sum(type7_u_topic)
    sum_type8_u_topic = np.sum(type8_u_topic)
    
    for i in range(num_topics_):
        type1_u_topic[i] = type1_u_topic[i] / sum_type1_u_topic
        type2_u_topic[i] = type2_u_topic[i] / sum_type2_u_topic
        type3_u_topic[i] = type3_u_topic[i] / sum_type3_u_topic
        type4_u_topic[i] = type4_u_topic[i] / sum_type4_u_topic
        type5_u_topic[i] = type5_u_topic[i] / sum_type5_u_topic
        type6_u_topic[i] = type6_u_topic[i] / sum_type6_u_topic
        type7_u_topic[i] = type7_u_topic[i] / sum_type7_u_topic
        type8_u_topic[i] = type8_u_topic[i] / sum_type8_u_topic
    
    return u_topic, type1_u_topic, type2_u_topic, type3_u_topic, type4_u_topic, type5_u_topic, type6_u_topic, type7_u_topic, type8_u_topic

def inference(word, context, sentence_string, pwe):
    #dataSplit: each contains id and text string (tuple)
    word_o_list = remove(context).split()
    select_keywords_set = set()
    select_keywords_set.add(remove(word))
    word_list_all = list()
    for wol in word_o_list:
        if wol in pwe.model.theta:
            word_list_all.append(wol)
            
    if len(word_list_all) ==0:
        print("word_list_all is empty")
        exit(0)
    
    sentence_doc = inferenceContext(sentence_string, pwe)

    for keyword in select_keywords_set:
        u_topic, type1_u_topic, type2_u_topic, type3_u_topic, type4_u_topic, \
        type5_u_topic, type6_u_topic, type7_u_topic, type8_u_topic \
        = updatewordsemanticwithcontext(keyword, sentence_string, pwe, sentence_doc)
    
    return u_topic, type1_u_topic, type2_u_topic, type3_u_topic, type4_u_topic, type5_u_topic,\
        type6_u_topic, type7_u_topic, type8_u_topic
   
def read_topics(beta_file, nwords):
    # for each line in the beta file
    beta = open(beta_file, "r", encoding = "utf-8")
    topicno = 0
    for topic in beta:
        #print("topic %03d :"% topicno)
        dicT = {}
        probabilitylist = topic.split()
        diclen = len(probabilitylist)
        if diclen != len(dictionary):
            print("the size of dictionary doesn't match the probability number.")
            print("the size of dictionary is %d, "% len(dictionary))
            print("and the probability number per line in word-probability-file is %d.\n"% diclen)
            sys.exit(1)
        for word in range(diclen):
            dicT.setdefault(dictionary[word]) # word text
            dicT[dictionary[word]] = float(probabilitylist[word]) # word probability
        
        sortedtopList = sorted(dicT.items(), key = lambda a:a[1], reverse=True)[:nwords]
        #word, prob pairs for each word in vocab for this topic

        topic_words_dic.setdefault(topicno)
        topic_words_dic[topicno] = sortedtopList #words most related to this topic
        topicno = topicno +1     

def getDictionary(vocab_file):
    vocab = open(vocab_file, 'r', encoding = "utf-8").readlines()
    for line in vocab:
        dictionary.append(line.strip().rstrip().split(":")[1])
    pass

def loadWordembeddings(thetafile):
    thetalines = open(thetafile,"r", encoding = "utf-8")
    tagno =0
    for topics_string in thetalines:
        tagprobabilitylist = topics_string.split()
        word = dictionary[tagno]
        if word not in word_topic_dis:
            word_topic_dis.setdefault(word)
            word_topic_dis[word] = tagprobabilitylist
        tagno = tagno+1
    pass

def printonewordTopics(wordstring,topn):
    print(wordstring + ": ")
    tagT = {}
    tagprobabilitylist = word_topic_dis[wordstring]
    topic_no = len(tagprobabilitylist)
    for topic in range(topic_no):
        tagT.setdefault(topic) # topic id
        tagT[topic] = float(tagprobabilitylist[topic]) # topic probability
    sortedTopList = sorted(tagT.items(), key= lambda a:a[1], reverse = True)[:topn] # top
    
    for i in range(topn):
        topicid = sortedTopList[i][0] # topic id for the tag
        topicpro = sortedTopList[i][1] # topic probability of the tag
        sortedwordList = topic_words_dic[topicid]
        print("    top"+str(i)+" (topic id: "+str(topicid)+"  "+str(topicpro)+") :" +str([word[0].strip().rstrip() for word in sortedwordList]))
    print("")
    pass

#takes target word vector, which is probabilities for each topic num
def printtops(vec):
    tagT = {}
    tagprobabilitylist = vec
    topic_no = len(tagprobabilitylist)
    for topic in range(topic_no):
        tagT.setdefault(topic) # topic id
        tagT[topic] = math.log(float(tagprobabilitylist[topic]))
    sortedTopList = sorted(tagT.items(), key= lambda a:a[1], reverse = True)[:top_num_topics] # top
    #tagT is probabilities for top 5 topic IDs
        
    #how to calculate sim given these topics?

    for i in range(top_num_topics):
        topicid = sortedTopList[i][0] # topic id for the tag
        topicpro = sortedTopList[i][1] # topic probability of the tag
        sortedwordList = topic_words_dic[topicid] #dic that holds top words for the topic ID
        print("    top"+str(i)+" (topic id: "+str(topicid)+"  "+str(topicpro)+") :" +str([word[0].strip().rstrip() for word in sortedwordList]))
    print("")
    pass
        
if __name__ == '__main__':
    if (len(sys.argv) != 11):
        print("usage: python addcrossvalidationClassfications.py <inputsourcetext> <stopwords>")
        print("<settingsfile> <pwe_file> <pifile> <betafile>  <vocabulary> <topnwords>\n")
        print("<tag_probability_file> <top_num_topics>\n")
        print(" ")
        sys.exit(1)
    
    inputsourcetext = sys.argv[1] # split by #
    stopwords = readstopwords(sys.argv[2])
    settingsfile = sys.argv[3]
    pwe_file = sys.argv[4]
    pifile = sys.argv[5]
    betafile = sys.argv[6]
    vocabularyfile = sys.argv[7] # input/vocabulary
    
    topnwords = int(sys.argv[8])
    tag_probability_file = sys.argv[9] # final.theta
    top_num_topics = int(sys.argv[10])

    getDictionary(vocabularyfile)
    read_topics(betafile, topnwords)
    
    print("begin to load DPWE's parameters: theta, pi, beta and dictionary.")
    pwe_dictionary, pwe_theta, pwe_pi, pwe_log_beta, pwe_vocabulary= readpwe_theta_pi_beta_dictionary(pwe_file, pifile, betafile,vocabularyfile)
    
    
    print("begin reconstruct DPWE..")
    configuration, model = reconstructpwe(settingsfile, pwe_dictionary, pwe_theta, pwe_pi, pwe_log_beta, pwe_vocabulary)
    pwe = ANWE()
    pwe.model = model
    pwe.configuration = configuration
    pwe.num_topics = model.num_topics
    pwe.num_tags = model.num_tags
    pwe.num_words = model.num_words
    
    word = []
    context = []
    sentence_string = []
    for line in open(inputsourcetext,"r"):
        textl = line.split("#")
        word.append(textl[0])
        #context.append(textl[1])
        sentence_string.append(textl[1])
        context.append(textl[1].replace(textl[0], ""))

    #loadWordembeddings(tag_probability_file)
    print("Results: \n")
    print("-----------------------------------")
    lent = len(word)
    #word gives list of target words
    #context gives list of contexts
    for i in range(lent):
        aword = word[i]
        acontext = context[i]
        asentence_string = sentence_string[i]
        #printonewordTopics(aword,top_num_topics)
        wordvectors = inference(aword, acontext, asentence_string, pwe)
        #returns 9 wordvectors, each length 200 - more tuned to context as you go? First one is just original
        #for two words, get these vectors, take fifth of each
        #find topic where they are both rated highly
        #take top word for that topic
        #good clue?
        print("The semantics of \''"+aword+"\'' with the following context:")
        num =0
        for v in wordvectors:
            if num == 5:
                printtops(v)
            num= num +1
        print("-----------------------------------")
    pass