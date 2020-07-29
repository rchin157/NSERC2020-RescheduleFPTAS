#include "annotate.h"
#include "main.h"

/**********************************
 * takes a list of jobs and marks if and where a job may be scheduled
 * jobs: a 2d array containing at least processing time and rejection cost
 * d: a pointer to an array of ints indicating which jobs are delayed
 * s: a pointer to an array of ints indicating the later schedule start time of each machine
 * para: a list of general parameters needed for the problem
 **********************************/
void labelJobs(int **jobs, int *d, int *s, int *para){
	//jobs addressed from 0 to n-1 internally
	//intialize labels as -1
	wipeLabels(jobs, para[NUMJOBS]);

	//mark delayed jobs in bool array
	int isDelayed[para[NUMJOBS]];
	for(int i = 0; i < para[NUMJOBS]; i++){
		isDelayed[i] = 0;
	}

	//get min start time to help determine minimum possible tardiness
	int min = s[0];
	for(int i = 1; i < para[NUMMACHINES]; i++){
		if(s[i] < min){
			min = s[i];
		}
	}

	//mark jobs that must be rejected and sum min cost
	int tardyMin = 0;
	for(int i = 0; i < para[NUMDELAYED]; i++){
		int tardiness = min + jobs[d[i]-1][0] - jobs[d[i]-1][2]; //this is the min possible tardiness using the machine with the earliest start time
		isDelayed[d[i]-1] = 1;
		if(tardiness > para[MAXTARDINESS]){ //a delayed job must be rejected if it cannot be in the later schedule
			jobs[d[i]-1][3] = REJECT;
			tardyMin += jobs[d[i]-1][1]; //this will indicate the absolute minimum required rejection cost of the problem
		}
	}

	//label remaining jobs
	int remainingRejection = para[REJECTIONCOSTBOUND] - tardyMin;
	for(int i = 0; i < para[NUMJOBS]; i++){
		if(jobs[i][3] != -1) continue;
		if(jobs[i][1] > remainingRejection){
			//job must be accepted since rejection costs too mmuch
			if(isDelayed[i]){
				jobs[i][3] = LATE; //delayed jobs cannot enter the early schedule
			} else{
				if((min + jobs[i][0] - jobs[i][2]) > para[MAXTARDINESS]){
					jobs[i][3] = EARLY; //the job is too tardy in the later schedule
				} else{
					jobs[i][3] = EL; //the job is still within tardiness limit in the later schedule
				}
			}
		} else{
			//job may be accepted as rejection is an option
			if(isDelayed[i]){
				jobs[i][3] = pLATE; //delayed jobs cannot enter the early schedule
			} else{
				if((min + jobs[i][0] - jobs[i][2]) > para[MAXTARDINESS]){
					jobs[i][3] = pEARLY; //the job is too tardy for the later schedule
				} else{
					jobs[i][3] = pEL; //the job is still within the tardiness limit in the later schedule
				}
			}
		}
	}
}

/******************************
 * sets all the labels of a set of jobs to -1 or default
 * jobs: pointer to a 2d array of jobs
 * n: the number of jobs
 *****************************/
void wipeLabels(int **jobs, int n){
	for(int i = 0; i < n; i++)
		jobs[i][3] = -1;
}

