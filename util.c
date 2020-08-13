#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "main.h"
#include "annotate.h"

/***********************************************
 * get required bounds and info for problem from user
 * p: a pointer to an array of general parameters
 * delayed: the address of a pointer to an array of delayed jobs
 * ud: the address of a pointer to an array of delayed jobs according to user provided numbering
 ***********************************************/
void getParams(int *p, int **delayed, int **ud, float *epsilon){
	//get info from stdin (user)

	while(1){
		printf("Enter r_D: ");
		int temp;
		if(scanf("%d", &temp) == 1){
			p[RD] = temp;
			char c;
			while((c = getchar()) != '\n'){}
			break;
		}
		char c;
		while((c = getchar()) != '\n'){}
	}
	while(1){
		printf("Enter # of delayed jobs: ");
		int temp;
		if(scanf("%d", &temp) == 1){
			p[NUMDELAYED] = temp;
			char c;
			while((c = getchar()) != '\n'){}
			break;
		}
		char c;
		while((c = getchar()) != '\n'){}
	}

	//delayed and ud are intially copies of each other
	*delayed = (int *)malloc(sizeof(int) * p[NUMDELAYED]);
	*ud = (int *)malloc(sizeof(int) * p[NUMDELAYED]);
	while(1){
		printf("Enter delayed jobs (space separated): ");
		int read = 0;
		for(int i = 0; i < p[NUMDELAYED]; i++){
		
			int temp;
			if(scanf("%d", &temp) == 1){
				read += 1;
				(*delayed)[i] = temp;
				(*ud)[i] = temp;
			}
		
		}
		char c;
		while((c = getchar()) != '\n'){}
		if(read == p[NUMDELAYED])
			break;
	}
	while(1){
		printf("Enter rejection cost bound: ");
		int temp;
		if(scanf("%d", &temp) == 1){
			p[REJECTIONCOSTBOUND] = temp;
			char c;
			while((c = getchar()) != '\n'){}
			break;
		}
		char c;
		while((c = getchar()) != '\n'){}
	}
	while(1){
		printf("Enter scaling factor: ");
		int temp;
		if(scanf("%d", &temp) == 1){
			p[SCALINGFACTOR] = temp;
			char c;
			while((c = getchar()) != '\n'){}
			break;
		}
		char c;
		while((c = getchar()) != '\n'){}
	}
	while(1){
		printf("Enter number of machines: ");
		int temp;
		if(scanf("%d", &temp) == 1){
			p[NUMMACHINES] = temp;
			char c;
			while((c = getchar()) != '\n'){}
			break;
		}
		char c;
		while((c = getchar()) != '\n'){}
	}
	while(1){
		printf("Enter epsilon value: ");
		float temp;
		if(scanf("%f", &temp) == 1){
			*epsilon = temp;
			char c;
			while((c = getchar()) != '\n'){}
			break;
		}
		char c;
		while((c = getchar()) != '\n'){}
	}
}

/*****************************************
 * gets jobs from file
 * jobs: the address of a pointer to a 2d array of jobs
 * p: a pointer to an array of general parameters
 * d: a pointer to an array of delayed jobs
 *****************************************/
void getJobs(int ***jobs, int *p, int *d, int *ud){
	char s[50];
	FILE *jobsFile = NULL;
	while(1){
		printf("Enter filename of jobs file: ");
		scanf("%s", s);
		if((jobsFile = fopen(s, "r")) != NULL) break;
	}

	//copy contents of file to array
	char n[5];
	fgets(n, 5, jobsFile);
	p[NUMJOBS] = atoi(n);
	*jobs = (int **)malloc(sizeof(int *) * p[NUMJOBS]);

	char buff[30];
	for(int i = 0; i < p[NUMJOBS]; i++){
		(*jobs)[i] = (int *)malloc(sizeof(int) * 5);
		fgets(buff, 30, jobsFile);
		char *token = strtok(buff, " ");
		for(int j = 0; j < 2; j++){
			(*jobs)[i][j] = atoi(token);
			token = strtok(NULL, " ");
		}
		//mark user provided order in jobs array
		(*jobs)[i][3] = -1;
		(*jobs)[i][4] = i + 1; //user provided order
	}
	//sort jobs according to processing time
	sortJobs(*jobs, 0, p[NUMJOBS] - 1, 0);
	//rewrite d with internal job numbers
	for(int j = 0; j < p[NUMDELAYED]; j++){
		for(int i = 0; i < p[NUMJOBS]; i++){
			if(ud[j] == (*jobs)[i][4]){
				d[j] = i + 1;
			}
		}
	}
	fclose(jobsFile);
}

/********************************************************
 * quicksort jobs by any job parameter
 * jobs: a pointer to a 2d array of jobs
 * low: start index for quicksort
 * high: end index for quicksort
 * sortby: the index of a job information to sort by
 ********************************************************/
void sortJobs(int **jobs, int low, int high, int sortby){
	if(low < high) {
		int part = partitionJobs(jobs, low, high, sortby);
		sortJobs(jobs, low, part - 1, sortby);
		sortJobs(jobs, part + 1, high, sortby);
	}
}

/************************************************************
 * the partition section of a basic quicksort
 * jobs: a pointer to a 2d array of jobs
 * low: start index for partition
 * high: end index for partition
 * sortby: the index of a job information to sort by
 ************************************************************/
int partitionJobs(int **jobs, int low, int high, int sortby){
	int piv = jobs[high][sortby];
	int j = low - 1;

	for(int i = low; i < high; i++){
		if(jobs[i][sortby] < piv){
			j++;
			int *temp = jobs[i];
			jobs[i] = jobs[j];
			jobs[j] = temp;
		}
	}
	int *temp = jobs[j + 1];
	jobs[j + 1] = jobs[high];
	jobs[high] = temp;
	return j + 1;	
}

/******************************************************
 * frees various allocs from main
 * delayed: a pointer to delayed jobs
 * jobs: pointer to jobs
 * numJobs: the number of jobs
 ******************************************************/
void cleanupMem(int *delayed, int **jobs, int numJobs, int *v){
	free(delayed);
	for(int i = 0; i < numJobs; i++){
		free(jobs[i]);
	}
	free(jobs);
	free(v);
}

/**************************************************
 * process program arguments, return -1 on invalid
 * argc: the number of arguments
 * argv: the arguments
 * mode: a pointer to an int indicating how main should continue
 **************************************************/
int processArgs(int argc, char *argv[], int *mode){
	if(argc == 1) return 0;
	char *manual = "-m"; //manual mode, all info from user
	char *automatic = "-g"; //automatic mode, all info generated
	if(strcmp(manual, argv[1]) == 0){
		*mode = 1;
		return 0;
	}
	if(strcmp(automatic, argv[1]) == 0){
		*mode = 2;
		return 0;
	}
	return -1;
}

/********************
 * prints bad arguments message
 ********************/
void reportBadArgs(){
	printf("Bad arguments. Valid arguments include \"-m\", \"-g\", \"-n\", and no arguments. See readme for more.\n");
}

/************************************
 * gets s values from user
 * values: the address of a pointer to an array of ints
 * m: the number of machines
 ************************************/
void getSValues(int **values, int m){
	*values = (int *)malloc(sizeof(int) * m);
	while(1){
		printf("Please enter %d s_i values in order (space separated): ", m);
		int read = 0;
		for(int i = 0; i < m; i++){
			int temp;
			if(scanf("%d", &temp) == 1){
				read += 1;
				(*values)[i] = temp;
			}
		}
		char c;
		while((c = getchar()) != '\n'){}
		if(read == m)
			break;
	}
}

/*********************************
 * print all jobs in a readable format
 * jobs: a pointer to a 2d array of jobs
 * n: the number of jobs
 *********************************/
void printJobs(int **jobs, int n){
	for(int i = 0; i < n; i++){
		printf("job %d\n", i+1);
		const char *annote = "default";
		switch(jobs[i][3]){
			case DELAYED:
				annote = "DELAYED";
				break;
			case ONTIME:
				annote = "ONTIME";
				break;
		}
		printf("processing time: %d rejection cost: %d original completion time: %d note: %s\n", jobs[i][0], jobs[i][1], jobs[i][2], annote);
	}
	printf("\n");
}

/******************************************
 * calculate the completion time of jobs according to the number of machines, input must be sorted in SPT order
 * jobs: a pointer to a 2d array of jobs
 * n: the number of jobs
 * m: the number of machines
 ******************************************/
void calculateCj(int **jobs, int n, int m){
	//simply distributes and sums processing time across machines
	int *sched = (int *)calloc(m, sizeof(int));
	for(int i = 0; i < n; i++){
		sched[i % m] += jobs[i][0];
		jobs[i][2] = sched[i % m];
	}
	free(sched);
}

/*****************************************
 * generic implemenetation of quick sort
 * a: pointer to an array to be sorted
 * low: the start index of quicksort
 * high: the end index of quicksort
 *****************************************/
void quickSort(int *a, int low, int high){
	if(low < high) {
		int part = partition(a, low, high);
		quickSort(a, low, part - 1);
		quickSort(a, part + 1, high);
	}
}

/****************************************
 * partition section of quicksort
 * a: pointer to an array to be sorted
 * low: start index of parition
 * high: end index of partition
 ****************************************/
int partition(int *a, int low, int high){
	int piv = a[high];
	int j = low - 1;

	for(int i = low; i < high; i++){
		if(a[i] < piv){
			j++;
			int temp = a[i];
			a[i] = a[j];
			a[j] = temp;
		}
	}
	int temp = a[j + 1];
	a[j + 1] = a[high];
	a[high] = temp;
	return j + 1;
}

/******************************************
 * calculates problem sparsness/density
 * para: array of problem parameters
 * jobs: array of jobs
 ******************************************
long double calculateProblemSparsness(int *para, int **jobs){
	int pmax = pMax(jobs, para);
	long double states = 2 * pow(para[NUMJOBS], para[NUMMACHINES] + 3) * pow(para[RD] + pmax, para[NUMMACHINES]) * pow(pmax, (2 * para[NUMMACHINES]) + 1) * para[MAXTARDINESS] * para[REJECTIONCOSTBOUND];
	long double trues = pow((2 * para[NUMMACHINES]) + 1, para[NUMJOBS]);
	//printf("states: %Lf\n", states);
	//printf("trues: %Lf\n", trues);
	return (long double)(trues / states);
}
*/

void calcDeltaV0(int **jobs, int *para, float *epsilon, float *delta, int **v){
	int pmax = pMax(jobs, para);
       	*delta = 1.0 + (float)(*epsilon / (2 * para[NUMJOBS]));
	double logthing = (double)((para[RD] - 1 + pmax) / para[RD]);
	*v = (int *)malloc(sizeof(int) * ((2 * para[NUMMACHINES]) + 3));
	(*v)[0] = para[NUMJOBS] + (int)ceil(myLog((double)*delta, logthing));

	return;
}

void calcVl(int **jobs, int *para, float delta, int *v, int *s){
	int pmax = pMax(jobs, para);
	int S1 = para[NUMJOBS] * pmax;
	int S3 = s[0] + (para[NUMJOBS] * pmax);
	int S2 = para[NUMJOBS] * S3;
	int S4 = para[REJECTIONCOSTBOUND];
	for(int i = 1; i <= (2 * para[NUMMACHINES]) + 3; i++){
		if(i <= para[NUMMACHINES])
			v[i] = (int)ceil(myLog((double)delta, (double)s[i - 1]));
		else if(i <= 2 * para[NUMMACHINES])
			v[i] = (int)ceil(myLog((double)delta, (double)S1));
		else if(i == (2 * para[NUMMACHINES]) + 1)
			v[i] = (int)ceil(myLog((double)delta, (double)S2));
		else if(i == (2 * para[NUMMACHINES]) + 2)
			v[i] = (int)ceil(myLog((double)delta, (double)S3));
		else if(i == (2 * para[NUMMACHINES]) + 3)
			v[i] = (int)ceil(myLog((double)delta, (double)S4));
	}
}

double myLog(double base, double result){
	return (double)(log(result) / log(base));
}

int pMax(int **jobs, int *para){
	int pmax = jobs[0][0];
	for(int i = 1; i < para[NUMJOBS]; i++){
		if(jobs[i][0] > pmax)
			pmax = jobs[i][0];
	}
	return pmax;
}

genericArr *buildArr(int *vl, int l, int level){
	genericArr *temp = (genericArr *)malloc(sizeof(genericArr));
	if(level < l - 1)
		temp -> arr = malloc(sizeof(genericArr *) * (vl[level] + 1));
	else{
		temp -> arr = calloc(vl[level] + 1, sizeof(int));
		return temp;
	}
	for(int i = 0; i <= vl[level]; i++){
		((genericArr **)(temp -> arr))[i] = buildArr(vl, l, level + 1);
	}
	return temp;
}

int arrGet(genericArr *temp, int *pos, int l){
	genericArr *current = ((genericArr **)(temp -> arr))[pos[0]];
	for(int i = 1; i < l - 1; i++){
		current = ((genericArr **)(current -> arr))[pos[i]];
	}
	return ((int *)(current -> arr))[pos[l - 1]];
}

void arrSet(genericArr *temp, int *pos, int l, int val){
	genericArr *current = ((genericArr **)(temp -> arr))[pos[0]];
	for(int i = 1; i < l - 1; i++){
		current = ((genericArr **)(current -> arr))[pos[i]];
	}
	((int *)(current -> arr))[pos[l - 1]] = val;
	return;
}

void freeArr(genericArr *temp, int *vl, int l, int level){
	if(level < l - 1){
		for(int i = 0; i < vl[level] + 1; i++)
			freeArr(((genericArr **)(temp -> arr))[i], vl, l, level + 1);
	}
	free(temp -> arr);
	free(temp);
	return;
}

