/*
 * pwelearn.h
 *
 *  Created on: Jul 25, 2016
 *      Author: shuangyinli
 */

#ifndef PWELEARN_H_
#define PWELEARN_H_

#include "utils.h"
#include "pwe.h"

void learn_pi(Window** corpus, Model* model, Configuration* configuration);
void learn_theta_beta(Window** corpus, Model* model);
double likehood(Window** corpus, Model* model);
double compute_win_likehood(Window* doc, Model* model);

#endif /* PWELEARN_H_ */
