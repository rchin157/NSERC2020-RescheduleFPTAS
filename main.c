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
	int parameters[7];
	int *delayed = NULL;
	int *userDelayed = NULL;
	int **jobs = NULL;
	//get information either from user or automatically
	if(mode != 2){
		getParams(parameters, &delayed, &userDelayed);
		getJobs(&jobs, parameters, delayed, userDelayed);
	} else{
		generateJobs(&jobs, parameters);
		generateInstance(parameters, &delayed);
	}

	//calcultate the completion time of jobs
	calculateCj(jobs, parameters[NUMJOBS], parameters[NUMMACHINES]);

	//report problem information
	printf("Jobs: %d,r_D: %d, rejection bound: %d, tardiness bound: %d, scaling factor: %d, machines: %d, number of delayed jobs: %d\n",
		parameters[NUMJOBS], parameters[RD], parameters[REJECTIONCOSTBOUND], parameters[MAXTARDINESS], parameters[SCALINGFACTOR], parameters[NUMMACHINES], parameters[NUMDELAYED]);
	printf("internal delayed subset: ");
	for(int i = 0; i < parameters[NUMDELAYED]; i++){
		if(mode != 2)
			printf("%d ", userDelayed[i]);
		else
			printf("%d ", delayed[i]);
	}
	printf("\n");
	free(userDelayed);

	long double sparsness = calculateProblemSparsness(parameters, jobs);
	printf("\nProblem Density(true/total): %.*Lf | %Le\n\n", DECIMAL_DIG, sparsness, sparsness);

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
		//label jobs for processing
		labelJobs(jobs, delayed, sValues, parameters);
		printf("Given Jobs\n");
		printJobs(jobs, parameters[NUMJOBS]);
		//run prepass
		State *soln = prepass(jobs, parameters);
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
		} else{
			//run one instance of DP
			soln = reschedule(jobs, parameters, sValues);
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
		}
		free(sValues);
	} else if(mode == 0){
		//enumerate on s_i
		//printf("Enumerating...\n");
		State *soln = enumerate(jobs, delayed, parameters);
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
		State *soln = enumerate(jobs, delayed, parameters);
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
	} else if(mode == 3){
		//no backtracking
		int result = enumerateNB(jobs, delayed, parameters);
		if(result){
			printf("\nReport\n");
			for(int i = 0; i < parameters[NUMJOBS]; i++)
				jobs[i][3] = bestAnnotes[i];
			printf("\n");
			printJobs(jobs, parameters[NUMJOBS]);
			printf("Optimal S values: ");
			for(int i = 0; i < parameters[NUMMACHINES]; i++)
				printf("%d ", bestS[i]);
			printf("\n");
			printf("Minimum objective value was: %d\n", result);
		} else
			printf("No solution\n");
		free(bestAnnotes);
		free(bestS);
	}

	//free memory of information
	cleanupMem(delayed, jobs, parameters[NUMJOBS]);
	
	return 0;
}

