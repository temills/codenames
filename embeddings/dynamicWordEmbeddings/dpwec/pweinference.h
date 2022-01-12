/*
 * pweinference.h
 *
 *  Created on: Jul 25, 2016
 *      Author: shuangyinli
 */

#ifndef PWEINFERENCE_H_
#define PWEINFERENCE_H_

#include "utils.h"
#include "pwe.h"
#include "pthread.h"
#include "unistd.h"
#include "stdlib.h"
#include "pwelearn.h"

struct Thread_Data {
    Window** corpus;
    int start;
    int end;
    Configuration* configuration;
    Model* model;
    Thread_Data(Window** corpus_, int start_, int end_, Configuration* config_, Model* model_) : corpus(corpus_), start(start_), end(end_), configuration(config_), model(model_) {
    }
};

void inference_gamma(Window* doc, Model* model);
void inference_xi(Window* doc, Model* model,Configuration* configuration);
void run_thread_inference(Window** corpus, Model* model, Configuration* configuration);
void do_inference(Window* doc, Model* model, Configuration* configuration);



#endif /* PWEINFERENCE_H_ */
