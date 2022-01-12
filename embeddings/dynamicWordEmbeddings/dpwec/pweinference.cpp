/*
 * pweinference.cpp
 *
 *  Created on: Jul 25, 2016
 *      Author: shuangyinli
 */
#include "pweinference.h"

void inference_gamma(Window* doc, Model* model) {
    double* log_theta = model->log_theta;
    double* log_beta = model->log_beta;
    int num_semantics = model->num_semantics;
    int num_vocabwords = model->num_vocabwords;
    int doc_num_words = doc->num_words;
    double* log_gamma = doc->log_gamma;
    double* theta_xi = new double[num_semantics];
    double sigma_xi = 0;
    for (int i = 0; i < doc->num_keywords; i++){
        sigma_xi += doc->xi[i];
    }
    for (int k = 0; k < num_semantics; k++) {
        double temp = 0;
        for (int i = 0; i < doc->num_keywords; i++) {
            temp += doc->xi[i]/sigma_xi * log_theta[doc->keywords_ptr[i]*num_semantics + k];
        }
        theta_xi[k] = temp;
        /*if (isnan(temp)) {
            printf("temp nan sigma_xi:%lf\n",sigma_xi);
        }*/
    }
    for (int i = 0; i < doc_num_words; i++) {
        int wordid = doc->words_ptr[i];
        double sum_log_gamma = 0;
        for (int k = 0; k < num_semantics; k++) {
            double temp = log_beta[k * num_vocabwords + wordid] + theta_xi[k];
            log_gamma[ i * num_semantics + k] = temp;
            if (k == 0) sum_log_gamma = temp;
            else sum_log_gamma = util::log_sum(sum_log_gamma, temp);
        }
        for (int k = 0; k < num_semantics; k++)log_gamma[i*num_semantics + k] -= sum_log_gamma;
    }
    delete[] theta_xi;
}


void get_descent_xi(Window* doc, Model* model,double* descent_xi) {
    double sigma_xi = 0.0;
    double sigma_pi = 0.0;
    int num_keywords = doc->num_keywords;
    for (int i = 0; i < num_keywords; i++) {
        sigma_xi += doc->xi[i];
        sigma_pi += model->pi[doc->keywords_ptr[i]];
    }
    for (int i = 0; i < num_keywords; i++) {
        descent_xi[i] = util::trigamma(doc->xi[i]) * ( model->pi[doc->keywords_ptr[i]] - doc->xi[i]);
        descent_xi[i] -= util::trigamma(sigma_xi) * (sigma_pi - sigma_xi);
    }
    int doc_num_words = doc->num_words;
    int num_semantics = model->num_semantics;
    double* log_theta = model->log_theta;
    double* sum_log_theta = new double[num_semantics];
    memset(sum_log_theta, 0, sizeof(double) * num_semantics);
    for (int k = 0; k < num_semantics; k++) {
        sum_log_theta[k] = 0;
        for (int i = 0; i < num_keywords; i++) {
            int keywords_id = doc->keywords_ptr[i];
            sum_log_theta[k] +=log_theta[keywords_id * num_semantics + k] * doc->xi[i];
        }
    }
    double* sum_gamma_array = new double[num_semantics];
    for (int k = 0; k < num_semantics; k++) {
        sum_gamma_array[k] = 0;
        for (int i = 0; i < doc_num_words; i++) {
            sum_gamma_array[k] += exp(doc->log_gamma[i * num_semantics + k]) * doc->words_cnt_ptr[i];
        }
    }
    for (int j = 0; j < num_keywords; j++) {
        for (int k = 0; k < num_semantics; k++) {
            double temp = 0;
            double sum_gamma = 0.0;
            temp += log_theta[doc->keywords_ptr[j] * num_semantics + k] * sigma_xi;
            sum_gamma = sum_gamma_array[k];
            temp -= sum_log_theta[k];
            temp = sum_gamma * (temp/(sigma_xi * sigma_xi));
            /*if (isnan(temp)) {
                printf("sum_gamma:%lf temp:%lf descent_xi:%lf\n",sum_gamma,temp,descent_xi[j]);
            }*/
            descent_xi[j] += temp;
        }
        /*if (isnan(descent_xi[j])) {
            printf("descent_xi nan\n");
        }*/
    }
    delete[] sum_log_theta;
    delete[] sum_gamma_array;
}


double get_xi_function(Window* doc, Model* model) {
    double xi_function_value = 0.0;
    int num_keywords = doc->num_keywords;
    double sigma_xi = 0.0;
    double* pi = model->pi;
    double* log_theta = model->log_theta;
    for (int i = 0; i < num_keywords; i++) sigma_xi += doc->xi[i];
    for (int i = 0; i < num_keywords; i++) {
        xi_function_value += (pi[doc->keywords_ptr[i]] - doc->xi[i] )* (util::digamma(doc->xi[i]) - util::digamma(sigma_xi)) + util::log_gamma(doc->xi[i]);
    }
    xi_function_value -= util::log_gamma(sigma_xi);

    int doc_num_words = doc->num_words;
    int num_semantics = model->num_semantics;

    double* sum_log_theta = new double[num_semantics];
    for (int k = 0; k < num_semantics; k++) {
        double temp = 0;
        for (int j = 0; j < num_keywords; j++) temp += log_theta[doc->keywords_ptr[j] * num_semantics + k] * doc->xi[j]/sigma_xi;
        sum_log_theta[k] = temp;
    }

    for (int i = 0; i < doc_num_words; i++) {
        for (int k = 0; k < num_semantics; k++) {
            double temp = sum_log_theta[k];
            xi_function_value += temp * exp(doc->log_gamma[i * num_semantics + k]) * doc->words_cnt_ptr[i];
        }
    }
    delete[] sum_log_theta;
    return xi_function_value;
}


inline void init_xi(double* xi,int num_keywords) {
    for (int i = 0; i < num_keywords; i++) xi[i] = util::random();//init 100?!
}


void inference_xi(Window* doc, Model* model,Configuration* configuration) {
    int num_keywords = doc->num_keywords;
    double* descent_xi = new double[num_keywords];
    init_xi(doc->xi,num_keywords);
    double z = get_xi_function(doc,model);
    double learn_rate = configuration->xi_learn_rate;
    double eps = 10000;
    int num_round = 0;
    int max_xi_iter = configuration->max_xi_iter;
    double xi_min_eps = configuration->xi_min_eps;
    double last_z;
    double* last_xi = new double[num_keywords];
    do {
        last_z = z;
        memcpy(last_xi,doc->xi,sizeof(double)*num_keywords);
        get_descent_xi(doc,model,descent_xi);

        bool has_neg_value_flag = false;
        for (int i = 0; !has_neg_value_flag && i < num_keywords; i++) {
            doc->xi[i] += learn_rate * descent_xi[i];
            if (doc->xi[i] < 0)has_neg_value_flag = true;
            //if (isnan(-doc->xi[i])) printf("doc->xi[i] nan\n");
        }
        if ( has_neg_value_flag || last_z > (z = get_xi_function(doc,model))) {
            learn_rate *= 0.1;
            z = last_z;
            eps = 10000;
            memcpy(doc->xi,last_xi,sizeof(double)*num_keywords);
        }
        else eps = util::norm2(last_xi,doc->xi,num_keywords);
        num_round ++;
    }
    while (num_round < max_xi_iter && eps > xi_min_eps);
    delete[] last_xi;
    delete[] descent_xi;
}


void do_inference(Window* doc, Model* model, Configuration* configuration) {
    int var_iter = 0;
    //double lik_old = 0.0;
    double lik_old = -100000;
    double converged = 1;
    double lik = 0;
    double * old_doc_loggamma = new double[doc->num_words * doc->num_semantics];
    double * old_doc_semantics = new double[doc->num_semantics];
    double * old_doc_xi = new double[doc->num_keywords];
    memcpy(old_doc_loggamma,doc->log_gamma,sizeof(double)*doc->num_words*doc->num_semantics);
    memcpy(old_doc_semantics,doc->semantics,sizeof(double)*doc->num_semantics);
    memcpy(old_doc_xi,doc->xi,sizeof(double)*doc->num_keywords);

    while ((converged > configuration->var_converence) && ((var_iter < configuration->max_var_iter || configuration->max_var_iter == -1))) {
        var_iter ++;
        inference_xi(doc, model, configuration);
        inference_gamma(doc, model);
        lik = compute_win_likehood(doc,model);
        doc->likelihood = lik;
        converged = (lik_old -lik) / lik_old;
        if(converged <0){
            memcpy(doc->log_gamma,old_doc_loggamma,sizeof(double)*doc->num_words*doc->num_semantics);
            memcpy(doc->semantics,old_doc_semantics,sizeof(double)*doc->num_semantics);
            memcpy(doc->xi, old_doc_xi,sizeof(double)*doc->num_keywords);
            doc->likelihood = lik;
            break;
        }

        memcpy(old_doc_loggamma,doc->log_gamma,sizeof(double)*doc->num_words*doc->num_semantics);
        memcpy(old_doc_semantics,doc->semantics,sizeof(double)*doc->num_semantics);
        memcpy(old_doc_xi,doc->xi,sizeof(double)*doc->num_keywords);

        lik_old = lik;
    }
    //printf("var_iter %d\n", var_iter);
    //doc -> likelihood = lik_old;
    delete[] old_doc_loggamma;
    delete[] old_doc_semantics;
    delete[] old_doc_xi;
    return;
}

void* thread_inference(void* thread_data) {
    Thread_Data* thread_data_ptr = (Thread_Data*) thread_data;
    Window** corpus = thread_data_ptr->corpus;
    int start = thread_data_ptr->start;
    int end = thread_data_ptr->end;
    Configuration* configuration = thread_data_ptr->configuration;
    Model* model = thread_data_ptr->model;
    for (int i = start; i < end; i++) {
        do_inference(corpus[i], model, configuration);
    }
    return NULL;
}

void run_thread_inference(Window** corpus, Model* model, Configuration* configuration) {
    int num_threads = configuration->num_threads;
    pthread_t* pthread_ts = new pthread_t[num_threads];
    int num_windows = model->num_windows;
    int num_per_threads = num_windows/num_threads;
    int i;
    Thread_Data** thread_datas = new Thread_Data* [num_threads];
    for (i = 0; i < num_threads - 1; i++) {
        thread_datas[i] = new Thread_Data(corpus, i * num_per_threads, (i+1)*num_per_threads, configuration, model);;
        pthread_create(&pthread_ts[i], NULL, thread_inference, (void*) thread_datas[i]);
    }
    thread_datas[i] = new Thread_Data(corpus, i * num_per_threads, num_windows, configuration, model);;
    pthread_create(&pthread_ts[i], NULL, thread_inference, (void*) thread_datas[i]);
    for (i = 0; i < num_threads; i++) pthread_join(pthread_ts[i],NULL);
    for (i = 0; i < num_threads; i++) delete thread_datas[i];
    delete[] thread_datas;
}

