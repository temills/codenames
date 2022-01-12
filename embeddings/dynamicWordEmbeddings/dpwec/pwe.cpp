//============================================================================
// Name        : PWE.cpp
// Author      : shuangyinli
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "pwe.h"
#include "pweinference.h"
#include "pwelearn.h"

void readinitlogbeta(Model* model, char* initbeta_file){
	// init the log_phi
	FILE* fp_beta = fopen(initbeta_file, "r");
	double * log_beta_ = model->log_beta;
	int num_semantics = model->num_semantics;
	int num_vocabwords = model->num_vocabwords;
	for (int i = 0; i < num_semantics; i++) {
		for (int j = 0; j < num_vocabwords; j++) {
			fscanf(fp_beta, "%lf", &log_beta_[i * num_vocabwords + j]);
		}
	}
	fclose(fp_beta);
}

Window** read_data(char* filename,int num_semantics,int& num_vocabwords, int& num_wins, int& num_keywords, int& num_all_words) {
	num_vocabwords = 0;
    num_wins = 0;
    num_keywords = 0;
    num_all_words = 0;
    FILE* fp = fopen(filename,"r");
    char c;
    while((c=getc(fp))!=EOF) {
        if (c=='\n') num_wins++;
    }
    fclose(fp);
    fp = fopen(filename,"r");
    int doc_num_keys;
    int doc_num_words;
    char str[10];
    Window** corpus = new Window* [num_wins + 10];
    num_wins = 0;
    while(fscanf(fp,"%d",&doc_num_keys) != EOF) {
        int* keys_ptr = new int[doc_num_keys];
        for (int i = 0; i < doc_num_keys; i++) {
            fscanf(fp,"%d",&keys_ptr[i]);
            num_keywords = num_keywords > keys_ptr[i]?num_keywords:keys_ptr[i];
        }
        fscanf(fp,"%s",str); //read @
        fscanf(fp,"%d", &doc_num_words);
        int* words_ptr = new int[doc_num_words];
        int* words_cnt_ptr = new int [doc_num_words];
        for (int i =0; i < doc_num_words; i++) {
            fscanf(fp,"%d:%d", &words_ptr[i],&words_cnt_ptr[i]);
            num_vocabwords = num_vocabwords < words_ptr[i]?words_ptr[i]:num_vocabwords;
            num_all_words += words_cnt_ptr[i];
        }
        corpus[num_wins++] = new Window(keys_ptr, words_ptr, words_cnt_ptr, doc_num_keys, doc_num_words, num_semantics);
    }
    fclose(fp);
    num_vocabwords ++;
    num_keywords ++;
    printf("num_windows: %d\nnum_keywords: %d\nnum_vocabwords:%d\n",num_wins,num_keywords,num_vocabwords);
    return corpus;
}

void Configuration::read_settingfile(char* settingfile) {

    FILE* fp = fopen(settingfile,"r");
    char key[100];
    while (fscanf(fp,"%s",key)!=EOF){
        if (strcmp(key,"pi_learn_rate")==0) {
            fscanf(fp,"%lf",&pi_learn_rate);
            continue;
        }
        if (strcmp(key,"max_pi_iter") == 0) {
            fscanf(fp,"%d",&max_pi_iter);
            continue;
        }
        if (strcmp(key,"pi_min_eps") == 0) {
            fscanf(fp,"%lf",&pi_min_eps);
            continue;
        }
        if (strcmp(key,"xi_learn_rate") == 0) {
            fscanf(fp,"%lf",&xi_learn_rate);
            continue;
        }
        if (strcmp(key,"max_xi_iter") == 0) {
            fscanf(fp,"%d",&max_xi_iter);
            continue;
        }
        if (strcmp(key,"xi_min_eps") == 0) {
            fscanf(fp,"%lf",&xi_min_eps);
            continue;
        }
        if (strcmp(key,"max_em_iter") == 0) {
            fscanf(fp,"%d",&max_em_iter);
            continue;
        }
        if (strcmp(key,"num_threads") == 0) {
            fscanf(fp, "%d", &num_threads);
        }
        if (strcmp(key, "var_converence") == 0) {
            fscanf(fp, "%lf", &var_converence);
        }
        if (strcmp(key, "max_var_iter") == 0) {
            fscanf(fp, "%d", &max_var_iter);
        }
        if (strcmp(key, "em_converence") == 0) {
            fscanf(fp, "%lf", &em_converence);
        }
    }

}

void Model::init(Model* init_model) {
    if (init_model) {
        for (int i = 0; i < num_keywords; i++) {
            pi[i] = init_model->pi[i];
            for (int k = 0; k < num_semantics; k++) log_theta[i*num_semantics + k] = init_model->log_theta[i*num_semantics + k];
        }
        for (int k = 0; k < num_semantics; k++) {
            for (int i = 0; i < num_vocabwords; i++) log_beta[k*num_vocabwords + i] = init_model->log_beta[k*num_vocabwords + i];
        }
        return;
    }
    for (int i = 0;i < num_keywords; i++) {
        pi[i] = util::random()*0.5 + 1;
        double temp = 0;
        for (int k = 0; k < num_semantics; k++){
            double v = util::random();
            temp += v;
            log_theta[i*num_semantics + k] = v;
        }
        for (int k = 0; k < num_semantics; k++)log_theta[i*num_semantics + k] = log(log_theta[i*num_semantics + k] / temp);
    }

    for (int k = 0; k < num_semantics; k++) {
        for (int i = 0; i < num_vocabwords; i++)log_beta[k*num_vocabwords + i] = log(1.0/num_vocabwords);
    }
}


void Window::init() {
    for (int i = 0; i < num_keywords; i++) {
        xi[i] = util::random();
    }
    for (int i = 0; i < num_words; i++) {
        for (int k = 0; k < num_semantics; k++) log_gamma[i*num_semantics + k] = log(1.0/num_semantics);
    }
}
void print_mat(double* mat, int row, int col, char* filename) {
    FILE* fp = fopen(filename,"w");
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            fprintf(fp,"%lf ",mat[i*col + j]);
        }
        fprintf(fp,"\n");
    }
    fclose(fp);
}

void print_documents_topics(Window** corpus, int num_docs, char* output_dir) {
    char filename[1000];
    sprintf(filename, "%s/win-semantics-dis.txt", output_dir);
    char liks_file[1000];
    sprintf(liks_file, "%s/likehoods.txt", output_dir);
    FILE* liks_fp = fopen(liks_file, "w");
    FILE* fp = fopen(filename,"w");
    for (int i = 0; i < num_docs; i++) {
        Window* win = corpus[i];
        fprintf(fp, "%lf", win->semantics[0]);
        fprintf(liks_fp, "%lf\n", win->likelihood);
        for (int k = 1; k < win->num_semantics; k++) fprintf(fp, " %lf", win->semantics[k]);
        fprintf(fp, "\n");
    }
    fclose(fp);
    fclose(liks_fp);
}

void print_para(Window** corpus, int num_round, char* model_root, Model* model) {
    char pi_file[1000];
    char theta_file[1000];
    char beta_file[1000];
    char xi_file[1000];
    char senmatics_dis_file[1000];
    char liks_file[1000];
    if (num_round != -1) {
        sprintf(pi_file, "%s/%03d.pi", model_root, num_round);
        sprintf(theta_file, "%s/%03d.theta", model_root, num_round);
        sprintf(beta_file, "%s/%03d.beta", model_root, num_round);
        sprintf(xi_file, "%s/%03d.xi", model_root, num_round);
        sprintf(senmatics_dis_file,"%s/%03d.semantics_dis", model_root, num_round);
        sprintf(liks_file, "%s/%03d.likelihoods", model_root, num_round);
    }
    else {
        sprintf(pi_file, "%s/final.pi", model_root);
        sprintf(theta_file, "%s/final.theta", model_root);
        sprintf(beta_file, "%s/final.beta", model_root);
        sprintf(xi_file, "%s/final.xi", model_root);
        sprintf(senmatics_dis_file,"%s/final.semantics_dis", model_root);
        sprintf(liks_file, "%s/final.likelihoods", model_root);
    }
    print_mat(model->log_beta, model->num_semantics, model->num_vocabwords, beta_file);
    print_mat(model->log_theta,model->num_keywords,model->num_semantics,theta_file);
    print_mat(model->pi, model->num_keywords, 1, pi_file);
    FILE* xi_fp = fopen(xi_file, "w");
    FILE* topic_dis_fp = fopen(senmatics_dis_file,"w");
    int num_docs = model->num_windows;
    FILE* liks_fp = fopen(liks_file, "w");
    for (int d = 0; d < num_docs; d++) {
        fprintf(liks_fp, "%lf\n", corpus[d]->likelihood);
        int doc_num_keywords = corpus[d]->num_keywords;
        int* num_keywords = corpus[d]->keywords_ptr;
        double* xi = corpus[d]->xi;
        for (int i = 0; i < doc_num_keywords; i++) {
            fprintf(xi_fp, "%d:%lf ", num_keywords[i], xi[i]);
        }
        fprintf(xi_fp,"\n");
        Window* doc = corpus[d];
        fprintf(topic_dis_fp, "%lf", doc->semantics[0]);
        for (int k = 1; k < doc->num_semantics; k++)fprintf(topic_dis_fp, " %lf", doc->semantics[k]);
        fprintf(topic_dis_fp, "\n");
    }
    fclose(xi_fp);
    fclose(topic_dis_fp);
    fclose(liks_fp);
}

void print_model_info(char* model_root, int num_vocabwords, int num_keywords,int num_semantics) {
    char filename[1000];
    sprintf(filename, "%s/model.info",model_root);
    FILE* fp = fopen(filename,"w");
    fprintf(fp, "num_vocabwords: %d\n", num_vocabwords);
    fprintf(fp, "num_keywords: %d\n", num_keywords);
    fprintf(fp, "num_semantics: %d\n", num_semantics);
    fclose(fp);
}

double* Model::load_mat(char* filename, int row, int col) {
    FILE* fp = fopen(filename,"r");
    double* mat = new double[row * col];
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            fscanf(fp, "%lf", &mat[i*col+j]);
        }
    }
    fclose(fp);
    return mat;
}


void beginpwe(char* inputfile, char* settingfile,int num_semantics, char* initbeta_file, char* model_root) {
    setbuf(stdout,NULL);
    int num_windows;
    int num_vocabwords;
    int num_keywords;
    int num_all_words;
    srand(unsigned(time(0)));
    Window** corpus = read_data(inputfile,num_semantics,num_vocabwords,num_windows,num_keywords, num_all_words);
    Model* model = new Model(num_windows,num_vocabwords,num_semantics,num_keywords,num_all_words);

    Configuration configuration = Configuration(settingfile);

    print_model_info(model_root, num_vocabwords,num_keywords, num_semantics);

    readinitlogbeta(model, initbeta_file);

    time_t learn_begin_time = time(0);
    int num_round = 0;
    printf("compute the corpus likehood...\n");
    double lik = likehood(corpus,model);
    double plik;
    double converged = 1;
    do {
        time_t cur_round_begin_time = time(0);
        plik = lik;
        printf("Round %d begin...\n", num_round);
        printf("inference...\n");
        run_thread_inference(corpus, model, &configuration);
        printf("learn pi ...\n");
        learn_pi(corpus,model,&configuration);
        printf("learn theta...\n");
        learn_theta_beta(corpus,model);
        printf("compute likehood...\n");
        lik = likehood(corpus,model);
        double perplex = exp(-lik/model->num_all_words);
        converged = (plik - lik) / plik;
        if (converged < 0) configuration.max_var_iter *= 2;
        unsigned int cur_round_cost_time = time(0) - cur_round_begin_time;
       printf("Round %d: likehood=%lf last_likehood=%lf perplex=%lf converged=%lf cost_time=%u secs.\n",num_round,lik,plik,perplex,converged, cur_round_cost_time);
        num_round += 1;
        if (num_round % 5 == 0)print_para(corpus,num_round, model_root, model);
    }
    while (num_round < configuration.max_em_iter && (converged < 0 || converged > configuration.em_converence || num_round < 10));
    unsigned int learn_cost_time = time(0) - learn_begin_time;

    printf("all learn runs %d rounds and cost %u secs.\n", num_round, learn_cost_time);

    print_para(corpus,-1,model_root, model);


    for (int i = 0; i < num_windows; i++)delete corpus[i];
    delete[] corpus;
    delete model;
}


void inferpwe(char* test_file, char* settingfile, char* model_root,char* prefix,char* out_dir=NULL) {
    setbuf(stdout,NULL);
    int num_windows;
    int num_vocabwords;
    int num_keywords;
    Model* model = new Model(model_root,prefix);
    int num_semantics = model->num_semantics;
    srand(unsigned(time(0)));
    Window** corpus = read_data(test_file,num_semantics,num_vocabwords,num_windows,num_keywords, model->num_all_words);
    model->num_windows = num_windows;
    Configuration config = Configuration(settingfile);
    run_thread_inference(corpus, model, &config);
    double lik = likehood(corpus, model);
    double perplex = exp(-lik/model->num_all_words);
    printf("likehood: %lf perplexity:%lf num all words: %d\n", lik, perplex,model->num_all_words);
    if (out_dir) {
        print_documents_topics(corpus, model->num_windows, out_dir);
    }

    for (int i = 0; i < num_windows; i++) {
        delete corpus[i];
    }
    delete[] corpus;
}


int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1],"est") == 0 && argc == 7){
    	beginpwe(argv[2],argv[3],atoi(argv[4]),argv[5], argv[6]);
    }
    else if (argc > 1 && strcmp(argv[1], "inf") == 0 && argc == 6 ){
            inferpwe(argv[2], argv[3],argv[4],argv[5]);
    }else{
    	printf("usage1: ./pwe est <input data file> <setting.txt> <num_topics> <init_word_probability>  <model save dir>\n");
    	printf("usage2: ./pwe inf <input data file> <setting.txt> <model dir> <prefix> <output dir>\n");
    	return 1;
    }
    return 0;
}
