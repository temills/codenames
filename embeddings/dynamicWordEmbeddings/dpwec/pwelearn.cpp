/*
 * pwelearn.cpp
 *
 *  Created on: Jul 25, 2016
 *      Author: shuangyinli
 */

#include "pwelearn.h"

double likehood(Window** corpus, Model* model) {
    int num_docs = model->num_windows;
    double lik = 0.0;
    for (int d = 0; d < num_docs; d++) {
        double temp_lik = compute_win_likehood(corpus[d],model);
        lik += temp_lik;
        corpus[d]->likelihood = temp_lik;
    }
    return lik;
}

double compute_win_likehood(Window* doc, Model* model) {
    double* log_semantics = doc->semantics;
    double* log_theta = model->log_theta;
    double* log_beta = model->log_beta;
    int num_semantics = model->num_semantics;
    int num_vocabwords = model->num_vocabwords;
    memset(log_semantics, 0, sizeof(double) * num_semantics);
    bool* reset_log_semantics = new bool[num_semantics];
    memset(reset_log_semantics, false, sizeof(bool) * num_semantics);
    double sigma_xi = 0;
    double* xi = doc->xi;
    int doc_num_keys = doc->num_keywords;
    double lik = 0.0;
    for (int i = 0; i < doc_num_keys; i++) {
        sigma_xi += xi[i];
    }

    for (int i = 0; i < doc_num_keys; i++) {
        int kid = doc->keywords_ptr[i];
        for (int k = 0; k < num_semantics; k++) {
            if (!reset_log_semantics[k]) {
            	log_semantics[k] = log_theta[kid * num_semantics + k] + log(xi[i]) - log(sigma_xi);
            	reset_log_semantics[k] = true;
            }
            else log_semantics[k] = util::log_sum(log_semantics[k], log_theta[kid * num_semantics + k] + log(xi[i]) - log(sigma_xi));
        }
    }
    int doc_num_words = doc->num_words;
    for (int i = 0; i < doc_num_words; i++) {
        double temp = 0;
        int wordid = doc->words_ptr[i];
        temp = log_semantics[0] + log_beta[wordid];
        for (int k = 1; k < num_semantics; k++) temp = util::log_sum(temp, log_semantics[k] + log_beta[k * num_vocabwords + wordid]);
        lik += temp * doc->words_cnt_ptr[i];
    }
    delete[] reset_log_semantics;
    return lik;
}


void get_descent_pi(Window** corpus, Model* model,double* descent_pi) {
    int num_keywords = model->num_keywords;
    int num_windows = model->num_windows;
    memset(descent_pi,0,sizeof(double)* num_keywords);
    double* pi = model->pi;
    for (int d = 0; d < num_windows; d++) {
        double sigma_pi = 0.0;
        Window* doc = corpus[d];
        int doc_num_keywords= doc->num_keywords;
        double sigma_xi = 0.0;
        for (int i = 0; i < doc_num_keywords; i++) {
            sigma_pi += pi[doc->keywords_ptr[i]];
            sigma_xi += doc->xi[i];
        }
        for (int i = 0; i < doc_num_keywords; i++) {
            int key_id = doc->keywords_ptr[i];
            double pis = pi[key_id];
            descent_pi[key_id] += util::digamma(sigma_pi) - util::digamma(pis) + util::digamma(doc->xi[i]) - util::digamma(sigma_xi);
        }
    }
}

void init_pi(double* pi, int num_keywords) {
    for (int i = 0; i < num_keywords; i++) {
        pi[i] = util::random() * 2;
    }
}

double get_pi_function(Window** corpus, Model* model) {
    double pi_function_value = 0.0;
    int num_windows = model->num_windows;
    double* pi = model->pi;
    for (int d = 0; d < num_windows; d++) {
        double sigma_pi = 0.0;
        double sigma_xi = 0.0;
        Window* doc = corpus[d];
        for (int i = 0; i < doc->num_keywords; i++) {
            sigma_pi += pi[doc->keywords_ptr[i]];
            sigma_xi += doc->xi[i];
        }
        pi_function_value += util::log_gamma(sigma_pi);
        for (int i = 0; i < doc->num_keywords; i++) {
            int keywords_id = doc->keywords_ptr[i];
            pi_function_value -= util::log_gamma(pi[keywords_id]);
            pi_function_value += (pi[keywords_id] - 1) * (util::digamma(doc->xi[i]) - util::digamma(sigma_xi));
        }
    }
    return pi_function_value;
}

void learn_pi(Window** corpus, Model* model, Configuration* config) {
    int num_round = 0;
    int num_keywords = model->num_keywords;
    double* last_pi = new double [model->num_keywords];
    double* descent_pi = new double[num_keywords];
    double z;
    int num_wait_for_z = 0;
    do {
        init_pi(model->pi,num_keywords);
        z = get_pi_function(corpus,model);
        fprintf(stderr, "wait for z >=0\n");
        num_wait_for_z ++;
    }
    while ( z < 0 && num_wait_for_z <= 20);
    double last_z;
    double learn_rate = config->pi_learn_rate;
    double eps = 1000;
    int max_pi_iter = config->max_pi_iter;
    double pi_min_eps = config->pi_min_eps;
    bool has_neg_value_flag = false;
    do {
        last_z = z;
        memcpy(last_pi,model->pi,sizeof(double) * num_keywords);
        get_descent_pi(corpus,model,descent_pi);
        for (int i = 0; i < num_keywords; i++) {
            model->pi[i] += learn_rate * descent_pi[i];
            if (model->pi[i] < 0) has_neg_value_flag = true;
        }
        if (has_neg_value_flag || last_z > (z=get_pi_function(corpus,model))) {
            learn_rate *= 0.1;
            z = last_z;
            memcpy(model->pi,last_pi,sizeof(double) * num_keywords);
            eps = 1000.0;
        }
        else eps = util::norm2(last_pi, model->pi, num_keywords);
        num_round += 1;
    }
    while (num_round < max_pi_iter && eps > pi_min_eps);
    delete[] last_pi;
    delete[] descent_pi;
}

void normalize_matrix_rows(double* mat, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        double temp = 0;
        for (int j = 0; j < cols; j++) temp += mat[ i * cols + j];
        for (int j = 0; j < cols; j++) {
            mat[i*cols +j] /= temp;
            if (mat[i*cols + j] == 0)mat[i*cols + j] = 1e-300;
        }
    }
}

void normalize_log_matrix_rows(double* log_mat, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        double temp = log_mat[ i * cols];
        for (int j = 1; j < cols; j++) temp = util::log_sum(temp, log_mat[i * cols + j]);
        for (int j = 0; j < cols; j++) log_mat[i*cols + j] -= temp;
    }
}

void learn_theta_beta(Window** corpus, Model* model) {
    int num_windows = model->num_windows;
    int num_semantics = model->num_semantics;
    int num_vocabwords = model->num_vocabwords;
    bool* reset_theta_flag = new bool[model->num_keywords * num_semantics];
    memset(reset_theta_flag, 0, sizeof(bool) * model->num_keywords * num_semantics);
    bool* reset_beta_flag = new bool[num_semantics * model->num_vocabwords];
    memset(reset_beta_flag, 0, sizeof(bool) * num_semantics * model->num_vocabwords);
    for (int d = 0; d < num_windows; d++) {
        Window* doc = corpus[d];
        int doc_num_keywords = doc->num_keywords;
        int doc_num_words = doc->num_words;
        double sigma_xi = 0;
        for (int i = 0; i < doc_num_keywords; i++)sigma_xi += doc->xi[i];
        for (int i = 0; i < doc_num_keywords; i++) {
            int keywords_id = doc->keywords_ptr[i];
            for (int k = 0; k < num_semantics; k++) {
                for (int j = 0; j < doc_num_words; j++) {
                    if (!reset_theta_flag[keywords_id * num_semantics + k]) {
                        reset_theta_flag[keywords_id * num_semantics + k] = true;
                        model->log_theta[keywords_id * num_semantics + k] = log(doc->words_cnt_ptr[j]) + doc->log_gamma[j * num_semantics + k] + log(doc->xi[i]) - log(sigma_xi);
                    }
                    else {
                        model->log_theta[keywords_id * num_semantics + k] = util::log_sum(model->log_theta[keywords_id * num_semantics + k], log(doc->words_cnt_ptr[j]) +doc->log_gamma[j * num_semantics + k] + log(doc->xi[i]) - log(sigma_xi));
                    }
                }
            }
        }
        for (int k = 0; k < num_semantics; k++) {
            for (int i = 0; i < doc_num_words; i++) {
                int wordid = doc->words_ptr[i];
                if (!reset_beta_flag[k * num_vocabwords + wordid]) {
                	reset_beta_flag[k * num_vocabwords + wordid] = true;
                    model->log_beta[k * num_vocabwords + wordid] = log(doc->words_cnt_ptr[i]) + doc->log_gamma[i*num_semantics + k];
                }
                else {
                    model->log_beta[k * num_vocabwords + wordid] = util::log_sum(model->log_beta[k * num_vocabwords + wordid], doc->log_gamma[i*num_semantics + k] + log(doc->words_cnt_ptr[i]));
                }
            }
        }
    }
    normalize_log_matrix_rows(model->log_theta, model->num_keywords, num_semantics);
    normalize_log_matrix_rows(model->log_beta, num_semantics, model->num_vocabwords);
    delete[] reset_theta_flag;
    delete[] reset_beta_flag;
}
