# distutils: language = c++
# distutils: sources = dpwec/pwe.cpp dpwec/pweinference.cpp dpwec/pwelearn.cpp
# cython: boundscheck=False
# cython: wraparound=False
# cython: cdivision=True
# coding: utf-8
import cython
import numpy as np
cimport numpy as np
from libcpp cimport bool
from libc.stdio cimport printf
from libc.stdlib cimport srand

cdef extern from "dpwec/pwe.h":
    cdef cppclass Window:
        double* xi
        double* log_gamma
        int* keywords_ptr
        int* words_ptr
        int* words_cnt_ptr
        int num_keywords
        int num_words
        int num_semantics
        double* semantics
        double likelihood
        bool build_inside
        Window()

    cdef cppclass Model:
        int num_windows
        int num_vocabwords
        int num_semantics
        int num_keywords
        int num_all_words
        double* pi
        double* log_theta
        double* log_beta
        bool build_inside
        Model()

    cdef cppclass Configuration:
        double pi_learn_rate
        int max_pi_iter
        double pi_min_eps
        double xi_learn_rate
        int max_xi_iter
        double xi_min_eps
        int max_em_iter
        int num_threads
        int max_var_iter
        double var_converence
        double em_converence
        Configuration()


cdef extern from "dpwec/pweinference.h":
    cdef void inference_gamma(Window* doc, Model* model) nogil
    cdef void inference_xi(Window* doc, Model * model, Configuration* configuration) nogil
    cdef void do_inference(Window * doc, Model * model, Configuration* configuraion) nogil

cdef extern from "dpwec/pwelearn.h":
    cdef double compute_win_likehood(Window *doc, Model* model) nogil




def fast_inference_xi(win, model, configuration):
    # transform python Configuration into c
    cdef Configuration c_configuration = Configuration()
    c_configuration.pi_learn_rate = <double> configuration.pi_learn_rate
    c_configuration.max_pi_iter = <int> configuration.max_pi_iter
    c_configuration.pi_min_eps = <double> configuration.pi_min_eps
    c_configuration.max_xi_iter = <int> configuration.max_xi_iter
    c_configuration.xi_min_eps = <double> configuration.xi_min_eps
    c_configuration.xi_learn_rate = <double> configuration.xi_learn_rate
    c_configuration.max_em_iter = <int > configuration.max_em_iter
    c_configuration.num_threads = <int > configuration.num_threads
    c_configuration.max_var_iter = <int > configuration.max_var_iter
    c_configuration.var_converence = <double > configuration.var_converence
    c_configuration.em_converence = <double > configuration.em_converence

    # tranform python Model into C
    cdef Model c_model = Model()
    c_model.num_windows = <int > model.num_wins
    c_model.num_vocabwords = <int > model.num_words
    c_model.num_semantics = <int > model.num_topics
    c_model.num_keywords = <int > model.num_tags
    c_model.num_all_words = <int > model.num_all_words
    c_model.pi = <double *> (np.PyArray_DATA(model.pi))
    c_model.log_theta = <double *> (np.PyArray_DATA(model.log_theta))
    c_model.log_beta = <double *> (np.PyArray_DATA(model.log_beta))


    cdef Window c_window = Window()
    c_window.xi = <double*  > (np.PyArray_DATA(win.xi))
    c_window.log_gamma = <double* > (np.PyArray_DATA(win.log_gamma))
    c_window.keywords_ptr = <int* > (np.PyArray_DATA(win.tags_ptr))
    c_window.words_ptr = <int* > (np.PyArray_DATA(win.words_ptr))
    c_window.words_cnt_ptr = <int* > (np.PyArray_DATA(win.words_cnt_ptr))
    c_window.num_keywords = <int > win.num_tags
    c_window.num_words = <int > win.num_words
    c_window.num_semantics = <int > win.num_topics
    c_window.semantics = <double *> (np.PyArray_DATA(win.topic))
    c_window.likelihood = <double > win.lik

    with nogil:
        inference_xi(&c_window, &c_model, &c_configuration)

    return





def fast_inference_gamma(win, model):

    # tranform python Model into C
    cdef Model c_model = Model()
    c_model.num_windows = <int > model.num_wins
    c_model.num_vocabwords = <int > model.num_words
    c_model.num_semantics = <int > model.num_topics
    c_model.num_keywords = <int > model.num_tags
    c_model.num_all_words = <int > model.num_all_words
    c_model.pi = <double *> (np.PyArray_DATA(model.pi))
    c_model.log_theta = <double *> (np.PyArray_DATA(model.log_theta))
    c_model.log_beta = <double *> (np.PyArray_DATA(model.log_beta))


    cdef Window c_window = Window()
    c_window.xi = <double*  > (np.PyArray_DATA(win.xi))
    c_window.log_gamma = <double* > (np.PyArray_DATA(win.log_gamma))
    c_window.keywords_ptr = <int* > (np.PyArray_DATA(win.tags_ptr))
    c_window.words_ptr = <int* > (np.PyArray_DATA(win.words_ptr))
    c_window.words_cnt_ptr = <int* > (np.PyArray_DATA(win.words_cnt_ptr))
    c_window.num_keywords = <int > win.num_tags
    c_window.num_words = <int > win.num_words
    c_window.num_semantics = <int > win.num_topics
    c_window.semantics = <double *> (np.PyArray_DATA(win.topic))
    c_window.likelihood = <double > win.lik

    with nogil:
        inference_gamma(&c_window, &c_model)

    return


def fast_compute_doc_likelihood(win, model):

    # tranform python Model into C
    cdef Model c_model = Model()
    c_model.num_windows = <int > model.num_wins
    c_model.num_vocabwords = <int > model.num_words
    c_model.num_semantics = <int > model.num_topics
    c_model.num_keywords = <int > model.num_tags
    c_model.num_all_words = <int > model.num_all_words
    c_model.pi = <double *> (np.PyArray_DATA(model.pi))
    c_model.log_theta = <double *> (np.PyArray_DATA(model.log_theta))
    c_model.log_beta = <double *> (np.PyArray_DATA(model.log_beta))


    cdef Window c_window = Window()
    c_window.xi = <double*  > (np.PyArray_DATA(win.xi))
    c_window.log_gamma = <double* > (np.PyArray_DATA(win.log_gamma))
    c_window.keywords_ptr = <int* > (np.PyArray_DATA(win.tags_ptr))
    c_window.words_ptr = <int* > (np.PyArray_DATA(win.words_ptr))
    c_window.words_cnt_ptr = <int* > (np.PyArray_DATA(win.words_cnt_ptr))
    c_window.num_keywords = <int > win.num_tags
    c_window.num_words = <int > win.num_words
    c_window.num_semantics = <int > win.num_topics
    c_window.semantics = <double *> (np.PyArray_DATA(win.topic))
    c_window.likelihood = <double > win.lik
    cdef double lik;

    with nogil:
        lik = compute_win_likehood(&c_window, &c_model)
    return lik

def fast_do_inference(win, model, configuration):
    # transform python Configuration into c
    cdef Configuration c_configuration = Configuration()
    c_configuration.pi_learn_rate = <double> configuration.pi_learn_rate
    c_configuration.max_pi_iter = <int> configuration.max_pi_iter
    c_configuration.pi_min_eps = <double> configuration.pi_min_eps
    c_configuration.max_xi_iter = <int> configuration.max_xi_iter
    c_configuration.xi_min_eps = <double> configuration.xi_min_eps
    c_configuration.xi_learn_rate = <double> configuration.xi_learn_rate
    c_configuration.max_em_iter = <int > configuration.max_em_iter
    c_configuration.num_threads = <int > configuration.num_threads
    c_configuration.max_var_iter = <int > configuration.max_var_iter
    c_configuration.var_converence = <double > configuration.var_converence
    c_configuration.em_converence = <double > configuration.em_converence

    # tranform python Model into C
    cdef Model c_model = Model()
    c_model.num_windows = <int > model.num_wins
    c_model.num_vocabwords = <int > model.num_words
    c_model.num_semantics = <int > model.num_topics
    c_model.num_keywords = <int > model.num_tags
    c_model.num_all_words = <int > model.num_all_words
    c_model.pi = <double *> (np.PyArray_DATA(model.pi))
    c_model.log_theta = <double *> (np.PyArray_DATA(model.log_theta))
    c_model.log_beta = <double *> (np.PyArray_DATA(model.log_beta))


    cdef Window c_window = Window()
    c_window.xi = <double*  > (np.PyArray_DATA(win.xi))
    c_window.log_gamma = <double* > (np.PyArray_DATA(win.log_gamma))
    c_window.keywords_ptr = <int* > (np.PyArray_DATA(win.tags_ptr))
    c_window.words_ptr = <int* > (np.PyArray_DATA(win.words_ptr))
    c_window.words_cnt_ptr = <int* > (np.PyArray_DATA(win.words_cnt_ptr))
    c_window.num_keywords = <int > win.num_tags
    c_window.num_words = <int > win.num_words
    c_window.num_semantics = <int > win.num_topics
    c_window.semantics = <double *> (np.PyArray_DATA(win.topic))
    c_window.likelihood = <double > win.lik

    with nogil:
        do_inference(&c_window, &c_model, &c_configuration)
    win.lik = c_window.likelihood

def reset(_seed):
    cdef unsigned int seed = _seed 
    srand(seed)
