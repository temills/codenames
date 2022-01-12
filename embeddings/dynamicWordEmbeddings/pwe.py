'''
Created on Oct 12, 2016

@author: shuangyinli
'''
from numpy import *
import numpy as np
import random
import copy
import time
import copy
import sys
import math
import re
from collections import Counter
from fast_inference import fast_inference_xi, fast_inference_gamma, fast_compute_doc_likelihood, fast_do_inference, reset as reset_seed
stopwords = {}
def readstopwords(stopwordsfile):
    return {}.fromkeys([ line.rstrip() for line in open(stopwordsfile, 'r') ])

def remove(abstract):
    abstract = re.sub("[^A-Za-z.]", " ", abstract)

    ###remove a single word
    abstract = re.sub(' [a-zA-Z] ', ' ', abstract)
    abstract = re.sub('^[ ]*[a-zA-Z] ', ' ', abstract)
    abstract = re.sub(' [a-zA-Z][ ]*$', ' ', abstract)

    abstract = ' '.join(abstract.split())
    abstract = abstract.lower()
    return abstract

class Window():
    def __init__(self, num_tags_, num_words_, num_topics_, lik_,
                tags_ptr_, words_ptr_, words_cnt_ptr_):
        '''
        Constructor
        '''
        # ensure the args are numpy array
        self.num_tags = num_tags_ # tag num in each doc
        self.num_words = num_words_
        self.num_topics = num_topics_
        self.lik = lik_
        self.tags_ptr = tags_ptr_
        self.words_ptr = words_ptr_
        self.words_cnt_ptr = words_cnt_ptr_
        #self.xi = [0 for i in range(self.num_tags)]
        self.xi = np.zeros(self.num_tags, dtype=np.float64) #[0 for i in range(self.num_tags)]
        self.log_gamma = np.zeros(shape = (self.num_words, self.num_topics), dtype=np.float64)
        #self.topic = np.array([0.0 for i in range(self.num_topics)])
        self.topic = np.zeros(self.num_topics, dtype=np.float64)
        self.Window_init()

    def Window_init(self):
        for i in range(self.num_tags):
            self.xi[i] = random.random()+0.5
        for i in range(self.num_words):
            for k in range(self.num_topics):
                self.log_gamma[i][k] = log(1.0 / self.num_topics)

class Model():
    def __init__(self, model_root_, num_wins_,num_words_, num_topics_, num_tags_, num_all_words_):
        '''
        Constructor
        '''
        self.num_wins = num_wins_
        self.num_words = num_words_
        self.num_topics = num_topics_
        self.num_tags = num_tags_ # all the tags number
        self.num_all_words = num_all_words_
        #self.pi = [0.0 for i in range(self.num_tags)]
        self.pi = np.zeros(self.num_tags, dtype=np.float64)
        self.log_theta = np.zeros(shape=(self.num_tags, self.num_topics), dtype=np.float64)
        self.log_beta = np.zeros(shape=(self.num_topics, self.num_words), dtype=np.float64)
        self.model_root = model_root_
        self.model_init()

    def setmodel(self, pwe_dictionary, theta, pi, log_beta,pwe_vocabulary):
        #dictionary : list of word string, one word  per line
        #theta: dic {} word, distribution, 0-1
        #pi: np.array
        #log_beta: np.array
        #tag = dictionary
        #words = vocabulary
        self.num_tags = len(pi)
        self.num_topics = log_beta.shape[0]
        self.pi = pi
        self.log_beta = log_beta
        self.num_words = len(log_beta[0])
        self.log_theta=  np.zeros(shape=(self.num_tags,self.num_topics), dtype=np.float64)
        self.theta = theta
        self.dictionary = pwe_dictionary
        self.vocabulary = pwe_vocabulary
        for d in range(len(pwe_dictionary)):
            dis = theta[pwe_dictionary[d]]
            len_vector = len(dis)
            for i in range(len_vector):
                self.log_theta[d][i] = math.log(dis[i])
        pass

    def loadmodel(self, log_theta_file, log_phi_file, pi_file, num_topics_):
        self.num_topics=num_topics_
        self.pi = []
        for line in open(pi_file, "r").readlines():
            self.pi.append(float(line))
        self.pi = np.array(self.pi, dtype=np.float64)

        self.num_tags = self.pi.shape[0]
        self.log_theta = np.zeros(shape=(self.num_tags, self.num_topics), dtype=np.float64)
        logthetafile = open(log_theta_file, "r").readlines()
        for i in range(len(logthetafile)):
            prolist = logthetafile[i].split()
            for j in range(len(prolist)):
                self.log_theta[i][j] = float(prolist[j])

        self.num_words = len(open(log_phi_file, "r").readlines()[0].split())
        self.log_beta = np.zeros(shape=(self.num_topics, self.num_words), dtype=np.float64)
        logphifile = open(log_phi_file, "r").readlines()
        for i in range(len(logphifile)):
            prolist = logphifile[i].split()
            for j in range(len(prolist)):
                self.log_beta[i][j] = float(prolist[j])
        pass

    def model_init(self):
        for i in range(self.num_tags):
            self.pi[i] = random.random() * 0.5 +1
            temp = 0.0
            for k in range(self.num_topics):
                v = random.random()
                temp += v
                self.log_theta[i][k] = v
            for k in range(self.num_topics):
                self.log_theta[i][k] = log(self.log_theta[i][k] / temp)
        for k in range(self.num_topics):
            for w in range(self.num_words):
                self.log_beta[k][w] = log(1.0 / self.num_words)


class Configuration():
    def __init__(self, settingsfile):
        '''
        Constructor
        '''
        self.pi_learn_rate = 0.00001
        self.max_pi_iter=100
        self.pi_min_eps=1e-5
        self.max_xi_iter=100
        self.xi_min_eps=1e-5
        self.xi_learn_rate = 10
        self.max_em_iter=30
        self.num_threads=1
        self.max_var_iter=30
        self.var_converence = 1e-6
        self.em_converence = 1e-4
        self.read_settingfile(settingsfile)

    def read_settingfile(self,settingsfile):
        settingslist = open(settingsfile, "r", encoding = "utf-8")
        for line in settingslist:
            confname = line.split()[0]
            confvalue = line.split()[1]
            if confname == "pi_learn_rate":
                self.pi_learn_rate = float(confvalue)
            if confname == "max_pi_iter":
                self.max_pi_iter = int(confvalue)
            if confname == "pi_min_eps":
                self.pi_min_eps = float(confvalue)
            if confname == "max_xi_iter":
                self.max_xi_iter = int(confvalue)
            if confname == "xi_learn_rate":
                self.xi_learn_rate = float(confvalue)
            if confname == "xi_min_eps":
                self.xi_min_eps = float(confvalue)
            if confname == "max_em_iter":
                self.max_em_iter = int(confvalue)
            if confname == "num_threads":
                self.num_threads = int(confvalue)
            if confname == "var_converence":
                self.var_converence = float(confvalue)
            if confname == "max_var_iter":
                self.max_var_iter = int(confvalue)
            if confname == "em_converence":
                self.em_converence = float(confvalue)

class ANWE():
    def __init__(self,filename = "", num_semantics = 0, settingsfile = "", model_root_ = "", process=""):
        '''
        Constructor
        '''
        self.corpus = []
        self.num_wins=0
        self.num_tags=0 # all the tag number in the corpus
        self.num_words=0
        self.num_all_words=0
        self.num_semantics = num_semantics
        self.model_root = model_root_

        if filename != "":
            self.read_data(filename)

        if model_root_ != "":
            self.model = Model(self.model_root, self.num_wins, self.num_words, self.num_topics,self.num_tags, self.num_all_words)

        if settingsfile != "":
            self.configuration = Configuration(settingsfile)
        if process =='est':
            pass
        elif process == 'inf':
            for win in self.corpus:
                self.do_inference(win)

    def read_data(self, filename):
        datalist = open(filename, "r", encoding = "utf-8").readlines()
        for onedata in datalist:
            labelslist = onedata.split("@")[0]
            wordslist = onedata.split("@")[1]
            win_num_tags = int(labelslist.split()[0])
            tags_ptr_ = [int(m) for m in labelslist.split()[1:]]
            num_word = int(wordslist.split()[0])
            words = wordslist.split()[1:]
            tags_ptr = []
            words_ptr = []
            words_cnt_ptr = []
            for t in range(win_num_tags):
                tags_ptr.append(int(tags_ptr_[t]))
            if self.num_tags < max(tags_ptr):
                self.num_tags = max(tags_ptr)
            for i in range(num_word):
                id_count = words[i].split(":")
                words_ptr.append(int(id_count[0]))
                words_cnt_ptr.append(int(id_count[1]))
                self.num_all_words += words_cnt_ptr[i]
            if self.num_words < max(words_ptr):
                self.num_words = max(words_ptr)
            win = Window(win_num_tags, num_word, self.num_topics, 100, tags_ptr_, words_ptr,words_cnt_ptr)
            self.corpus.append(win)
            self.num_wins += 1
        self.num_tags += 1
        self.num_words +=1
        if self.corpus[1].num_tags != len(self.corpus[1].tags_ptr):
            print("the number of tags in a window doesn't equal with its ptr..")
            exit(0)
        if self.corpus[1].num_words != len(self.corpus[1].words_ptr):
            print("the number of words in a window doesn't equal with its ptr..")
        #
        print("num_wins: "+str(self.num_wins)+", num_tags: "+str(self.num_tags)+", num_words: "+str(self.num_words)+" \n")

    def save_parameters_wins(self, num_round):
        if num_round != -1:
            xi_file = self.model.model_root+"xi."+str(num_round)
            topic_dis_file = self.model.model_root+"topic_dis_wins."+str(num_round)
        else:
            xi_file = self.model.model_root+"xi.final"
            topic_dis_file = self.model.model_root+"topic_dis_wins.final"
        xi_fp = open(xi_file, "w", encoding = "utf-8")
        topic_dis_fp = open(topic_dis_file,"w", encoding = "utf-8")

        for win in self.corpus:
            for i in range(win.num_tags):
                xi_fp.write(str(win.tags_ptr[i]) +":"+ str(win.xi[i]))
            xi_fp.write("\n")

            for k in range(self.num_topics):
                topic_dis_fp.write(str(win.topic[k]) + ' ')
            topic_dis_fp.write("\n")

        xi_fp.close()
        topic_dis_fp.close()

    def fast_do_inference(self, win):
        fast_do_inference(win, self.model, self.configuration)
    
    def updateSomeThetas(self, thetas):
        #thetas is list of (keywords, its new topics)
        for the in thetas:
            keys = the[0]
            topics_array = the[1]
            wid = self.model.dictionary.index(keys)
            for i in range(self.model.num_topics):
                self.model.log_theta[wid][i] = topics_array[i]
        pass

    def do_inference(self, win):
        var_iter =0
        lik_old = -100000.
        converged = 1.
        lik = 0
        old_win = copy.deepcopy(win)
        while (converged > self.configuration.var_converence) and ( var_iter < self.configuration.max_var_iter or self.configuration.max_var_iter == -1):
            var_iter += 1
            self.inference_xi(win)
            self.inference_gamma(win)
            lik= self.compute_doc_likelihood(win)
            win.lik = lik
            converged = (lik_old - lik) / lik_old
            if converged < 0:
                win = copy.deepcopy(old_win)
                print(win.lik)
                break
            lik_old = lik
            print(lik)
            old_win = copy.deepcopy(win)
        return win

    def inference_xi(self, win):
        fast_inference_xi(win, self.model, self.configuration)
        return
    def inference_gamma(self, win):
        fast_inference_gamma(win, self.model)
        return
    def compute_doc_likelihood(self, win):
        lik = fast_compute_doc_likelihood(win, self.model)
        return lik

def readpwe_theta_pi_beta_dictionary(theta_file, pifile, betafile):
    num_semantic = 0
    dictionary = []
    tagsDis ={}
    theta = open(theta_file, "r", encoding = "utf-8")
    pi = open(pifile, "r", encoding = "utf-8")
    for ts in theta:
        word = ts.split()[0].strip().lstrip().rstrip()
        dictionary.append(word)
        topicslist = np.array([float(m) for m in ts.split()[1:]], dtype=np.float64)
        num_semantic = len(topicslist)
        tagsDis.setdefault(str(word))
        tagsDis[word] = topicslist

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

    return dictionary, tagsDis, newpi, log_beta

def reconstructpwe(settingsfile, pwe_dictionary, pwe_theta, pwe_pi, log_beta):
    configuration = Configuration(settingsfile)
    model = Model("", 0, 0, 0,0, 0)
    model.setmodel(pwe_dictionary, pwe_theta, pwe_pi, log_beta)
    return configuration, model

def codeTexttopwecode(fulltext, pwe, keyword):
    wordslist = remove(fulltext).split()
    wordlist = []
    dictionary = pwe.model.dictionary
    for w in wordslist:
        if w not in stopwords:
            if w in dictionary:
                wordlist.append(w)
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
        words_ptr.append(dictionary.index(wd))
        words_cnt_ptr.append(c)
    # ensure keywords_ptr, words_ptr, words_cnt_ptr are numpy array
    keywords_ptr = np.array(keywords_ptr, dtype=np.int32)
    words_ptr = np.array(words_ptr, dtype=np.int32)
    words_cnt_ptr = np.array(words_cnt_ptr, dtype=np.int32)
    return Window(num_keywords, num_words, num_topics, 100, keywords_ptr, words_ptr,words_cnt_ptr)


if __name__ == '__main__':
    if (len(sys.argv) != 6 and len(sys.argv) != 9):
        print("usage1: anwe.py est <input data file> <setting.txt> <num_semantics> <model save dir>")
        print("usage2: anwe.py inf <input test file> <num_semantics> <settingsfile> <pwe_file> <pifile> <betafile> \n")
        exit(0)
    if sys.argv[1] == 'est':
        inputfile = sys.argv[2]
        settingsfile = sys.argv[3]
        num_semantics = int(sys.argv[4])
        model_root = sys.argv[5]
        if model_root.endswith("/") is False:
            model_root = model_root+"/"
        ANWE(inputfile, num_semantics, settingsfile, model_root, 'est')
        pass
    elif sys.argv[1] == 'inf':
        inputtestfile = sys.argv[2]
        num_semantics = int(sys.argv[3])
        settingsfile = sys.argv[4]
        anwe_file = sys.argv[5]
        pifile = sys.argv[6]
        betafile = sys.argv[7]
        stopwords = readstopwords(sys.argv[8])

        #reconstruct model
        anwe_dictionary, anwe_theta, anwe_pi, log_beta= readpwe_theta_pi_beta_dictionary(anwe_file, pifile, betafile)
        configuration, model = reconstructpwe(settingsfile, anwe_dictionary, anwe_theta, anwe_pi, log_beta)
        anwe = ANWE()
        anwe.model = model
        anwe.configuration = configuration
        anwe.num_topics = model.num_topics
        anwe.num_tags = model.num_tags
        anwe.num_words = model.num_words

        print("begin read text")
        for line in open(inputtestfile, "r").readlines():
            print("next line")
            win = codeTexttopwecode(line, anwe, " ")
            #win2 = copy.deepcopy(win)
            cost = - time.time()
            reset_seed(0)
            win = anwe.do_inference(win)
            cost += time.time()
            print("python version", cost)
            #cost = - time.time()
            #reset_seed(0)
            anwe.fast_do_inference(win)
            
            #cost += time.time()
            #print("c version", cost)
            #print(win2.log_gamma - win.log_gamma)
            
            #print(win2.xi - win.xi)
    print("end")

