/*
 * pwe.h
 *
 *  Created on: Jul 25, 2016
 *      Author: shuangyinli
 */

#ifndef PWE_H_
#define PWE_H_

#include "stdio.h"
#include "utils.h"

struct Window {
    double* xi;
    double* log_gamma;
    int* keywords_ptr;
    int* words_ptr;
    int* words_cnt_ptr;
    int num_keywords;
    int num_words;
    int num_semantics;
    double* semantics;
    double likelihood;
    bool build_inside;
    Window() {
        build_inside = false;
    }
    Window(int* keywords_ptr_,int* words_ptr_,int* words_cnt_ptr_,int num_keywords_,int num_words_,int num_topics_) {
    	num_semantics = num_topics_;
    	num_keywords = num_keywords_;
        num_words = num_words_;
        xi = new double[num_keywords];
        log_gamma = new double[num_words * num_semantics];
        semantics = new double[num_semantics];
        keywords_ptr = keywords_ptr_;
        words_ptr = words_ptr_;
        words_cnt_ptr = words_cnt_ptr_;
        likelihood = 100;
        init();
        build_inside = true;
    }
    ~Window() {
        // delete the array if the Window is build with init
        if (build_inside) {
            if (xi)delete[] xi;
            if (log_gamma) delete[] log_gamma;
            if (keywords_ptr) delete[] keywords_ptr;
            if (words_ptr) delete[] words_ptr;
            if (words_cnt_ptr) delete[] words_cnt_ptr;
            if (semantics) delete[] semantics;
        }
    }
    void init();
};

struct Model {
    int num_windows;
    int num_vocabwords;
    int num_semantics;
    int num_keywords;
    int num_all_words; //num_all_words = num_vocabwords * frequency
    double* pi;
    double* log_theta;
    double* log_beta;
    bool build_inside;
    Model() {
        build_inside = false;
    }
    Model(int num_windows_, int num_words_,int num_semantics_, int num_keywords_, int num_all_words_, Model* init_model=NULL) {
    	num_keywords = num_keywords_;
    	num_windows = num_windows_;
    	num_semantics = num_semantics_;
    	num_vocabwords = num_words_;
        num_all_words = num_all_words_;
        pi = new double[num_keywords];
        log_theta = new double[num_keywords * num_semantics];
        log_beta = new double[num_semantics * num_vocabwords];
        init(init_model);
        build_inside = true;
    }
    Model(char* model_root, char* prefix) {
        char pi_file[1000];
        sprintf(pi_file, "%s/%s.pi", model_root,prefix);
        char theta_file[1000];
        sprintf(theta_file,"%s/%s.theta",model_root,prefix);
        char beta_file[1000];
        sprintf(beta_file,"%s/%s.beta",model_root,prefix);
        pi = load_mat(pi_file, num_keywords, 1);
        log_theta = load_mat(theta_file, num_keywords, num_semantics);
        log_beta = load_mat(beta_file, num_semantics, num_vocabwords);
    }
    ~Model() {
        // delete the Model class if it is build with args
        if (build_inside) {
            if (pi)delete[] pi;
            if (log_theta) delete[] log_theta;
            if (log_beta) delete[] log_beta;
        }
    }
    void init(Model* init_model=NULL);
    double* load_mat(char* filename,int row,int col);
};

struct Configuration {
    double pi_learn_rate;
    int max_pi_iter;
    double pi_min_eps;
    double xi_learn_rate;
    int max_xi_iter;
    double xi_min_eps;
    int max_em_iter;
    int num_threads;
    int max_var_iter;
    double var_converence;
    double em_converence;
    Configuration() {

    }
    Configuration(char* settingfile) {
        pi_learn_rate = 0.00001;
        max_pi_iter = 100;
        pi_min_eps = 1e-5;
        max_xi_iter = 100;
        xi_learn_rate = 10;
        xi_min_eps = 1e-5;
        max_em_iter = 30;
        num_threads = 1;
        var_converence = 1e-6;
        max_var_iter = 30;
        em_converence = 1e-4;
        if(settingfile) read_settingfile(settingfile);
    }
    void read_settingfile(char* settingfile);
};

#endif /* PWE_H_ */
