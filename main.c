#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include "util.h"
#include "dp.h"
#include "main.h"
#include "annotate.h"
#include "enumerate.h"
#include "automate.h"

int mode = 0;

int main(int argc, char *argv[]){
	srand(time(0));
	//check args for mode, 0 userinput with s value enumeration, 1 full userinput, 2 fully automagic
	if(processArgs(argc, argv, &mode) == -1){
		reportBadArgs();
		return 0;
	}

	//initialize arrays
	int parameters[6];
	int *delayed = NULL;
	int *userDelayed = NULL;
	int **jobs = NULL;
	float epsilon = 1.0;
	float delta;
	int *vl = NULL;
	//get information either from user or automatically
	if(mode != 2){
		getParams(parameters, &delayed, &userDelayed, &epsilon);
		getJobs(&jobs, parameters, delayed, userDelayed);
	} else{
		generateJobs(&jobs, parameters);
		generateInstance(parameters, &delayed, &epsilon);
	}

	//calcultate the completion time of jobs
	calculateCj(jobs, parameters[NUMJOBS], parameters[NUMMACHINES]);

	//report problem information
	printf("Jobs: %d,r_D: %d, rejection bound: %d, scaling factor: %d, machines: %d, number of delayed jobs: %d\n",
		parameters[NUMJOBS], parameters[RD], parameters[REJECTIONCOSTBOUND], parameters[SCALINGFACTOR], parameters[NUMMACHINES], parameters[NUMDELAYED]);
	printf("internal delayed subset: ");
	for(int i = 0; i < parameters[NUMDELAYED]; i++){
		if(mode != 2)
			printf("%d ", userDelayed[i]);
		else
			printf("%d ", delayed[i]);
	}
	printf("\n");
	free(userDelayed);

/*
	long double sparsness = calculateProblemSparsness(parameters, jobs);
	printf("\nProblem Density(true/total): %.*Lf | %Le\n\n", DECIMAL_DIG, sparsness, sparsness);
*/

	calcDeltaV0(jobs, parameters, &epsilon, &delta, &vl);

	//printf("running reschedule\n");
	//run rescheduling
	//printf("mode: %d\n", mode);
	extern int *bestAnnotes;
	extern int *bestS;
	int *sValues = NULL;
	if(mode == 1){
		//get s values
		getSValues(&sValues, parameters[NUMMACHINES]);
		for(int i = 0; i < parameters[NUMMACHINES]; i++){
			printf("%d ", sValues[i]);
		}
		printf("\n");
		//get vl values
		calcVl(jobs, parameters, delta, vl, sValues);
		//print results
		for(int i = 0; i <= 2 * parameters[NUMMACHINES] + 1; i++){
			if(i == 0)
				printf("v0: %d\n", vl[0]);
			else if(i <= parameters[NUMMACHINES])
				printf("M%d: %d\n", i, vl[i]);
			else if(i <= 2 * parameters[NUMMACHINES])
				printf("P%d: %d\n", i - parameters[NUMMACHINES], vl[i]);
			else
				printf("Z, T, R: %d, %d, %d\n", vl[i], vl[i + 1], vl[i + 2]);
		}
		/*
		//label jobs for processing
		labelJobs(jobs, delayed, sValues, parameters);
		printf("Given Jobs\n");
		printJobs(jobs, parameters[NUMJOBS]);
		//run one instance of DP
		State *soln = reschedule(jobs, parameters, sValues, vl, delta);
		if(soln != NULL){
			//report result
			printf("\nReport:\n");
			int objVal = soln -> tCompTime + soln -> sumRejectCost + (parameters[SCALINGFACTOR] * soln -> maxTardy);
			int transcribe[parameters[NUMJOBS]];
			for(int i = 0; i < parameters[NUMJOBS]; i++)
				transcribe[i] = jobs[i][4];
			//print jobs
			sortJobs(jobs, 0, parameters[NUMJOBS] - 1, 4);
			printJobs(jobs, parameters[NUMJOBS]);
			//print backtracking
			reportAssignments(soln, parameters[NUMMACHINES], transcribe);
			//report result
			printf("Minimum objective value was: %d\n", objVal);
			freeSolnList(soln);
		} else
			printf("No solution\n");
		*/
		free(sValues);
	} else if(mode == 0){
		//enumerate on s_i
		//printf("Enumerating...\n");
		State *soln = enumerate(jobs, delayed, parameters, delta);
		if(soln != NULL){
			//report result
			printf("\nReport:\n");
			//set labels for printing
			for(int i = 0; i < parameters[NUMJOBS]; i++)
				jobs[i][3] = bestAnnotes[i];
			printf("\n");
			int transcribe[parameters[NUMJOBS]];
			for(int i = 0; i < parameters[NUMJOBS]; i++)
				transcribe[i] = jobs[i][4];
			//sort jobs according to user provided order
			sortJobs(jobs, 0, parameters[NUMJOBS] - 1, 4);
			printJobs(jobs, parameters[NUMJOBS]);
			int objVal = soln -> tCompTime + soln -> sumRejectCost + (parameters[SCALINGFACTOR] * soln -> maxTardy);
			//print backtracking
			reportAssignments(soln, parameters[NUMMACHINES], transcribe);
			//print s values of solution
			printf("Optimal S values: ");
			for(int i = 0; i < parameters[NUMMACHINES]; i++)
				printf("%d ", bestS[i]);
			printf("\n");
			//report obj value
			printf("Minimum objective value was: %d\n", objVal);
			freeSolnList(soln);
		} else
			printf("No solution\n");
		free(bestAnnotes);
		free(bestS);
	} else if(mode == 2){
		//automatic mode
		State *soln = enumerate(jobs, delayed, parameters, delta);
		if(soln != NULL){
			printf("\nReport:\n");
			//set labels for printing
			for(int i = 0; i < parameters[NUMJOBS]; i++)
				jobs[i][3] = bestAnnotes[i];
			printf("\n");
			printJobs(jobs, parameters[NUMJOBS]);
			int objVal = soln -> tCompTime + soln -> sumRejectCost + (parameters[SCALINGFACTOR] * soln -> maxTardy);
			int transcribe[parameters[NUMJOBS]];
			for(int i = 0; i < parameters[NUMJOBS]; i++)
				transcribe[i] = i + 1;
			//print backtracking
			reportAssignments(soln, parameters[NUMMACHINES], transcribe);
			//print s values of solution
			printf("Optimal S values: ");
			for(int i = 0; i < parameters[NUMMACHINES]; i++)
				printf("%d ", bestS[i]);
			printf("\n");
			//report result
			printf("Minimum objective value was: %d\n", objVal);
			freeSolnList(soln);
		} else
			printf("No solution\n");
		free(bestAnnotes);
		free(bestS);
	}

	//free memory of information
	cleanupMem(delayed, jobs, parameters[NUMJOBS], vl);
	
	return 0;
}

