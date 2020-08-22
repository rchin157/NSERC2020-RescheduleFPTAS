#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "dp.h"
#include "enumerate.h"
#include "main.h"
#include "annotate.h"
#include "util.h"

//tracks the start times associated with the best solution
int *bestS = NULL;
//tracks the job labels associated with the best solution
int *bestAnnotes = NULL;
extern int minAchieved;

/******************************************************
 * repeatedly runs reschedule with all valid s value combos
 * jobs: a pointer to a 2d array of jobs
 * delayed: a pointer to an array of delayed jobs
 * para: a pointer to an array of general parameters
 ******************************************************/
State *enumerate(int **jobs, int *delayed, int *para, float delta, int *vl){
	int *sValues = NULL;

	//initialize s values to rd for labeling
	int *temp = (int *)malloc(sizeof(int) * para[NUMMACHINES]);
	for(int i = 0; i < para[NUMMACHINES]; i++)
		temp[i] = para[RD];
	
	bestAnnotes = (int *)malloc(sizeof(int) * para[NUMJOBS]);
	bestS = (int *)malloc(sizeof(int) * para[NUMMACHINES]);

	//print initialized jobs
	//printf("Initial labeling\n");
	labelJobs(jobs, delayed, temp, para);
	printf("Given Jobs\n");
	printJobs(jobs, para[NUMJOBS]);
	free(temp);
/*	
	State *soln = prepass(jobs, para);
	if(soln != NULL){
		printf("prepass soln found\n");
		for(int i = 0; i < para[NUMMACHINES]; i++)
			bestS[i] = para[RD];
		for(int i = 0; i < para[NUMJOBS]; i++)
			bestAnnotes[i] = jobs[i][3];
		return soln;
	}
*/
	//list of candidate s values is put in sValues
	//printf("determining valid s values\n");
	determineS(para, &sValues, delta, vl);

	//get vl values
	calcVl(jobs, para, delta, vl, sValues);
	//print results
	for(int i = 0; i <= 2 * para[NUMMACHINES] + 1; i++){
		if(i == 0)
			printf("v0: %d\n", vl[0]);
		else if(i <= para[NUMMACHINES])
			printf("M%d: %d\n", i, vl[i]);
		else if(i <= 2 * para[NUMMACHINES])
			printf("P%d: %d\n", i - para[NUMMACHINES], vl[i]);
		else
			printf("Z, T, R: %d, %d, %d\n", vl[i], vl[i + 1], vl[i + 2]);
	}
	//allocate table
	genericArr *table[para[NUMJOBS]];
	for(int i = 0; i < para[NUMJOBS]; i++)
		table[i] = buildArr(vl, (2 * para[NUMMACHINES]) + 4, 1);

	//run reschedule with all s value combos and return best
	State *minState = NULL;
	int combo[para[NUMMACHINES]];
	//printf("Running combinations...\n");
	runCombinations(sValues, combo, 0, vl[0] - 1, 0, para[NUMMACHINES], jobs, para, &minState, delayed, vl, delta, table);
/*	printf("Optimal s values: ");
	for(int i = 0; i < para[NUMMACHINES]; i++)
		printf("%d ", bestS[i]);
	printf("\n");*/
	free(sValues);
	for(int i = 0; i < para[NUMJOBS]; i++)
		freeArr(table[i], vl, (2 * para[NUMMACHINES]) + 4, 1);
	return minState;
}

/*********************************************************************************************************************************
 * runs reschedule with every combination of s values
 * sValues: a pointer to an array of candidate s values
 * combo: an array of length m that will hold finished combos
 * start: the index in sValues to start at
 * end: the index in sValues to end at
 * index: the position in the combo array curently being decided
 * m: the number of machines
 * jobs: a pointer to a 2d array of jobs
 * para: a pointer to an array of general parameters
 * min: the address of a pointer to the minimum achieved solution
 * d: a pointer to a list of delayed jobs
 *********************************************************************************************************************************/
void runCombinations(int *sValues, int combo[], int start, int end, int index, int m, int **jobs, int *para, State **min, int *d, int *vl, float delta, genericArr **table){
	//if index == m a full combo has been completed
	if(index == m){
/*		printf("combo: ");
		for(int i = 0; i < m; i++)
			printf("%d ", combo[i]);
		printf("\n");*/
		//label jobs again due to the new set of s values
		labelJobs(jobs, d, combo, para);
		for(int i = 0; i < para[NUMJOBS]; i++)
			resetArr(table[i], vl, (2 * para[NUMMACHINES]) + 4, 1);
		State *temp = reschedule(table, jobs, para, combo, vl, delta);
		//printf("min global obj: %d\n", minAchieved);
		//check obtained solution
		if(temp == NULL)
			return;
		if(*min == NULL)
			*min = temp;
		else{
			//comparing solutions is not required since reschedule only returns better solutions
			freeSolnList(*min);
			*min = temp;
		}
		//track details of best solution
		for(int i = 0; i < m; i++)
			bestS[i] = combo[i];
		for(int i = 0; i < para[NUMJOBS]; i++)
			bestAnnotes[i] = jobs[i][3];
		return;
	}
	//build combos
	for(int i = start; i <= end; i++){
		combo[index] = sValues[i];
		runCombinations(sValues, combo, i, end, index + 1, m, jobs, para, min, d, vl, delta, table);
	}
}

