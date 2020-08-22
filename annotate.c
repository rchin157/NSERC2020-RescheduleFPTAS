/* Written by Rylan Chin, Finalized August 2020
 * Labelling jobs helps to reduce state generation later.
 * Jobs may be labelled Ontime or delayed.
 */

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

	//mark delayed jobs
	for(int i = 0; i < para[NUMDELAYED]; i++){
		jobs[d[i]-1][3] = DELAYED;
	}

}

/******************************
 * sets all the labels of a set of jobs to -1 or default
 * jobs: pointer to a 2d array of jobs
 * n: the number of jobs
 *****************************/
void wipeLabels(int **jobs, int n){
	for(int i = 0; i < n; i++)
		jobs[i][3] = ONTIME;
}

