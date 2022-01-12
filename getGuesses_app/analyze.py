#!/usr/bin/env python3
import json

def getGoodTrials(trials):
    new_trials = []
    for t in trials:
        id = t['subject_id']
        trial_num = int(t['trial_order'])-1
        #check subject data to see if they passed the check for this trial
        subj = list(filter(lambda d: d['subject_id']==id, subj_data))[0]
        if subj['check_num_dif'][trial_num]==0:
            new_trials.append(t)
    return new_trials

def sanityCheck(trials):
    for t in trials[50:60]:
        print('Clue: ' + t['clue'])
        print(t['words'][0:4])
        print(t['words'][4:8])
        print(t['words'][8:12])
        print('\n\n\n\n\n\n\n\n\n\n')
        print(t['chosen_words'])
        print('\n\n\n\n')

#get data from files
def getData():
    with open('data.json') as f:
        td = json.load(f)
    with open('subject_data.json') as f:
        sd = json.load(f)
    return td, sd

#rewrite subject and trial data files with desired data types and return data
def fixDataTypes(td, sd):
    #fix up trial data
    for i in range(len(td)):  
        td[i]['words'] = eval(td[i]['words'])
        td[i]['chosen_words'] = eval(td[i]['chosen_words'])
        td[i]['rt'] = float(td[i]['rt'])
        td[i]['trial_order'] = int(td[i]['trial_order'])
    #rewrite file
    with open('data.json', 'w') as outfile:
        json.dump(td, outfile)
    #get data
    with open('data.json') as f:
        td = json.load(f)
    
    #fix up subject data
    for i in range(len(sd)):  
        sd[i]['check_words'] = eval(sd[i]['check_words'])
    #rewrite file
    with open('subject_data.json', 'w') as outfile:
        json.dump(sd, outfile)
    #get data
    with open('subject_data.json') as f:
        sd = json.load(f)
    return td, sd

data, subj_data = getData()
data, subj_data = fixDataTypes(data, subj_data)
#tr = getGoodTrials(data)
#print("___________________________________________________________")
#sanityCheck(tr)



        