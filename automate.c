#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "util.h"

//processing time upper bound
#define PUPPER 60
//rejection cost upper bound
#define EUPPER 60
//number of jobs upper bound
#define JUPPER 15
//number of machines upper bound
#define MUPPER 10
//rejection cost bound upper bound
#define RUPPER 150
//a modifier pertaining to rejection cost upper bound that relates to number of jobs
#define RSCALE 10
//scaling factor upper bound
#define SUPPER 5

/****************************************
 * creates a set of jobs in SPT order
 * jobs: the address of a pointer to a 2d array for storing job info
 * para: a pointer to an int array of general parameters for the problem
 ****************************************/
void generateJobs(int ***jobs, int *para){
	para[NUMJOBS] = (rand() % JUPPER) + 1;
	*jobs = (int **)malloc(sizeof(int *) * para[NUMJOBS]);
	//create array of random processing times
	int pTemp[para[NUMJOBS]];
	for(int i = 0; i < para[NUMJOBS]; i++){
		pTemp[i] = (rand() % PUPPER) + 1;
	}
	//sort array (SPT)
	quickSort(pTemp, 0, para[NUMJOBS] - 1);
	//fill job information
	for(int i = 0; i < para[NUMJOBS]; i++){
		(*jobs)[i] = (int *)malloc(sizeof(int) * 4);
		(*jobs)[i][0] = pTemp[i];
		(*jobs)[i][1] = (rand() % EUPPER) + 1;
		(*jobs)[i][3] = -1;
	}
}

/**********************************************
 * generates all the parameters needed to run an instance
 * para: a pointer to an int array of general parameters for the problem
 * delayed: the address of a pointer to an array of delayed jobs
 *********************************************/
void generateInstance(int *para, int **delayed, float *eps){
	//set the number of delayed jobs
	para[NUMDELAYED] = (rand() % para[NUMJOBS]) + 1;
	//set the number of machines
	para[NUMMACHINES] = (rand() % MUPPER) + 1;
	//set the rejection cost bound, should scale with jobs
	para[REJECTIONCOSTBOUND] = ((rand() % RUPPER) + 1) * ((para[NUMJOBS] / RSCALE) + 1);
	//set scaling factor
	para[SCALINGFACTOR] = (rand() % SUPPER) + 1;
	//set RD, should scale with jobs
	para[RD] = (rand() % (PUPPER * ((para[NUMJOBS] / RSCALE) + 1))) + 1;

	*eps = (float)(1.0 / ((rand() % 10) + 1));

	//create a set of delayed jobs
	*delayed = (int *)malloc(sizeof(int) * para[NUMDELAYED]);
	for(int i = 0; i < para[NUMDELAYED]; i++){
		while(1){
			int new = 1;
			int temp = (rand() % para[NUMJOBS]) + 1;
			for(int j = 0; j < i; j++){
				if((*delayed)[j] == temp)
					new = 0;
			}
			if(new){
				(*delayed)[i] = temp;
				break;
			}
		}
	}
}
