#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dp.h"
#include "main.h"
#include "annotate.h"

//global minimum
int minAchieved = -1;
int nextSolnID = 1;

/************************************************
 * runs the dp algorithm according to the provided jobs and parameters, returning a linked list solution
 * jobs: a pointer to a 2d array of jobs
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times for this problem
 ************************************************/
State *reschedule(int **jobs, int *para, int *s){
	//create inital State
	State *root = (State *)calloc(1, sizeof(State));
	root -> schedules = (int *)calloc(2 * para[NUMMACHINES], sizeof(int));
	root -> parent = NULL;

	//build tree
	State *soln = NULL;
	generateChildren(jobs, 0, para, s, root, &soln);

	if(soln == NULL){
		return NULL;
	}

	//minAchieved = soln -> tCompTime + solni -> sumRejectCost + (para[SCALINGFACTOR] * soln -> maxTardy);

	return soln;
}

/****************************************************************************************************************
 * adds all possiblilites of the next job to the tree based on its label and returns the number of children created
 * jobs: a pointer to a 2d array of jobs
 * j: job number
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of start times for the machines
 * prnt: the node in the tree that these children stemmed from
 * currentSoln: state pointer for current best solution
 ****************************************************************************************************************/
void generateChildren(int **jobs, int j, int *para, int *s, State *prnt, State **currentSoln){
	switch(jobs[j][3]){
		case EARLY:
			acceptEarly(jobs, j, para, s, prnt, currentSoln);
			break;
		case LATE:
			acceptLate(jobs, j, para, s, prnt, currentSoln);
			break;
		case pEARLY:
			acceptEarly(jobs, j, para, s, prnt, currentSoln);
			reject(jobs, j, para, s, prnt, currentSoln);
			break;
		case pLATE:
			acceptLate(jobs, j, para, s, prnt, currentSoln);
			reject(jobs, j, para, s, prnt, currentSoln);
			break;
		case REJECT:
			reject(jobs, j, para, s, prnt, currentSoln);
			break;
		case EL:
			acceptEarly(jobs, j, para, s, prnt, currentSoln);
			acceptLate(jobs, j, para, s, prnt, currentSoln);
			break;
		case pEL:
			acceptEarly(jobs, j, para, s, prnt, currentSoln);
			acceptLate(jobs, j, para, s, prnt, currentSoln);
			reject(jobs, j, para, s, prnt, currentSoln);
			break;
	}
	if(prnt -> children == 0){
		if(prnt -> parent != NULL)
			prnt -> parent -> children -= 1;
		freeRealEstate(prnt);
	}
}

/**********************************************************
 * validates solution
 * soln: leaf state pointer
 * para: array of parameters
 * s: array of machine start times
 *********************************************************/
int checkValidSoln(State *soln, int *para, int *s){
	int validState = 1;
	//the start time of each machine must be exactly max(M_i, RD) to be valid
	for(int i = 0; i < para[NUMMACHINES]; i++){
		if(s[i] != (soln -> schedules[i] > para[RD] ? soln -> schedules[i] : para[RD]))
			validState = 0;
	}
	return validState;
	
}

/***********************************************************************************************************
 * handles node generation for jobs that are to be accepted into the early schedule
 * jobs: a pointer to a 2d array of jobs
 * j: job number of the children to be generated
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times
 * prnt: a pointer to the node in the tree that these children stemmed from
 * head: a pointer to the head of the linked list for valid leaf nodes
 * all: a pointer to the head of a linked list tracking all tree nodes
 ***********************************************************************************************************/
void acceptEarly(int **jobs, int j, int *para, int *s, State *prnt, State **currentSoln){
	for(int i = 0; i < para[NUMMACHINES]; i++){
		//for each machine where the job can be added a state is generated
		if(prnt -> schedules[i] < para[RD] && prnt -> schedules[i] + jobs[j][0] <= s[i] && prnt -> schedules[i] + jobs[j][0] - jobs[j][2] <= para[MAXTARDINESS]){
			//check if over global min, if so the state is not generated
			if((minAchieved != -1) && (prnt -> schedules[i] + jobs[j][0] + prnt -> tCompTime + prnt -> sumRejectCost) + (prnt -> maxTardy >
				(prnt -> schedules[i] + jobs[j][0] - jobs[j][2]) ? para[SCALINGFACTOR] * prnt -> maxTardy :
			       	para[SCALINGFACTOR] * (prnt -> schedules[i] + jobs[j][0] - jobs[j][2])) >= minAchieved)
				continue;
			int sum1 = 0;
			for(int l = 0; l < para[NUMMACHINES]; l++){
				if(s[l] > para[RD]){
					if(l == i)
						sum1 += (s[l] - (prnt -> schedules[l] + jobs[j][0]));
					else
						sum1 += (s[l] - prnt -> schedules[l]);
				}
			}
			int sum2 = 0;
			for(int l = j + 1; l < para[NUMJOBS]; l++){
				if(jobs[l][3] == EARLY || jobs[l][3] == pEARLY || jobs[l][3] == EL || jobs[l][3] == pEL)
					sum2 += jobs[l][0];
			}
			
			if(sum2 < sum1)
				continue;
			//allocate state struct
			State *temp = (State *)malloc(sizeof(State));
			*temp = *prnt;
			//setup new state info
			temp -> jobNum = j;
			temp -> children = 0;
			temp -> solnID = 0;
			//allocate new space for machine schedules
			temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
			//copy prnt schedule to child
			memcpy(temp -> schedules, prnt -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
			temp -> schedules[i] += jobs[j][0];
			temp -> tCompTime += temp -> schedules[i];
			temp -> parent = prnt;
			//update max tardiness
			if(temp -> schedules[i] - jobs[j][2] > temp -> maxTardy)
				temp -> maxTardy = temp -> schedules[i] - jobs[j][2];

			if(j >= para[NUMJOBS] - 1 ){
				if(checkValidSoln(temp, para, s)){
					markSolution(temp);
					if(*currentSoln != NULL)
						freeOldSoln(*currentSoln);
					*currentSoln = temp;
					nextSolnID %= 4;
					nextSolnID += 1;
					minAchieved = temp -> tCompTime + temp -> sumRejectCost + (para[SCALINGFACTOR] * temp -> maxTardy);
				} else{
					freeRealEstate(temp);
					continue;
				}
			}
			//increment the number of child states generated
			prnt -> children += 1;

			if(j < para[NUMJOBS] - 1){ //if job j is not a leaf
				generateChildren(jobs, j + 1, para, s, temp, currentSoln);
			}
		}
	}
}

/***********************************************************************************************************
 * handles node generation for jobs that are to be accepted into the later schedule
 * jobs: a pointer to a 2d array of jobs
 * j: job number of the children to be generated
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times
 * prnt: a pointer to the node in the tree that these children stemmed from
 * head: a pointer to the head of the linked list for valid leaf nodes
 * all: a pointer to the head of a linked list tracking all tree nodes
 ***********************************************************************************************************/
void acceptLate(int **jobs, int j, int *para, int *s, State *prnt, State **currentSoln){
	for(int i = para[NUMMACHINES]; i < 2 * para[NUMMACHINES]; i++){
		//for each machine where the job can be added to the later schedule a state is generated
		if(s[i - para[NUMMACHINES]] + prnt -> schedules[i] + jobs[j][0] - jobs[j][2] <= para[MAXTARDINESS]){
			//check if over global min, if so the state is not generated
			if(minAchieved != -1 && (s[i - para[NUMMACHINES]] + prnt -> schedules[i] + jobs[j][0] + prnt -> tCompTime + prnt -> sumRejectCost) + (prnt -> maxTardy
				> (s[i - para[NUMMACHINES]] + prnt -> schedules[i] + jobs[j][0] - jobs[j][2]) ?
				para[SCALINGFACTOR] * prnt -> maxTardy : para[SCALINGFACTOR] * (s[i - para[NUMMACHINES]] + prnt -> schedules[i] + jobs[j][0] - jobs[j][2])) >= minAchieved)
				continue;
			int sum1 = 0;
			for(int l = 0; l < para[NUMMACHINES]; l++){
				if(s[l] > para[RD])
					sum1 += (s[l] - prnt -> schedules[l]);
			}
			int sum2 = 0;
			for(int l = j + 1; l < para[NUMJOBS]; l++){
				if(jobs[l][3] == EARLY || jobs[l][3] == pEARLY || jobs[l][3] == EL || jobs[l][3] == pEL)
					sum2 += jobs[l][0];
			}
			
			if(sum2 < sum1)
				continue;
			//allocate state
			State *temp = (State *)malloc(sizeof(State));
			*temp = *prnt;
			//setup new state info
			temp -> jobNum = j;
			temp -> children = 0;
			temp -> solnID = 0;
			//allocate space for new nodes schedule
			temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
			//copy prnt schedule to child
			memcpy(temp -> schedules, prnt -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
			temp -> schedules[i] += jobs[j][0];
			temp -> tCompTime += s[i - para[NUMMACHINES]] + temp -> schedules[i];
			temp -> parent = prnt;
			//update max tardiness
			if(s[i - para[NUMMACHINES]] + temp -> schedules[i] - jobs[j][2] > temp -> maxTardy)
				temp -> maxTardy = s[i - para[NUMMACHINES]] + temp -> schedules[i] - jobs[j][2];
			if(j >= para[NUMJOBS] - 1 ){
				if(checkValidSoln(temp, para, s)){
					markSolution(temp);
					if(*currentSoln != NULL)
						freeOldSoln(*currentSoln);
					*currentSoln = temp;
					nextSolnID %= 4;
					nextSolnID += 1;
					minAchieved = temp -> tCompTime + temp -> sumRejectCost + (para[SCALINGFACTOR] * temp -> maxTardy);
				} else{
					freeRealEstate(temp);
					continue;
				}
			}
			//increment the number of child states generated
			prnt -> children += 1;

			if(j < para[NUMJOBS] - 1){ //if job j is not a leaf
				generateChildren(jobs, j + 1, para, s, temp, currentSoln);
			}
		}
	}
}

/***********************************************************************************************************
 * handles node generation for jobs that are to be rejected
 * jobs: a pointer to a 2d array of jobs
 * j: job number of the children to be generated
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times
 * prnt: a pointer to the node in the tree that these children stemmed from
 * head: a pointer to the head of the linked list for valid leaf nodes
 * all: a pointer to the head of a linked list tracking all tree nodes
 ***********************************************************************************************************/
void reject(int **jobs, int j, int *para, int *s, State *prnt, State **currentSoln){
	if(prnt -> sumRejectCost + jobs[j][1] <= para[REJECTIONCOSTBOUND]){
		//check if over global min, if so state is not generated
		if(minAchieved != -1 && (prnt -> tCompTime + (prnt -> sumRejectCost + jobs[j][1]) + (prnt -> maxTardy * para[SCALINGFACTOR])) >= minAchieved)
			return;
		int sum1 = 0;
		for(int l = 0; l < para[NUMMACHINES]; l++){
			if(s[l] > para[RD])
				sum1 += (s[l] - prnt -> schedules[l]);
		}
		int sum2 = 0;
		for(int l = j + 1; l < para[NUMJOBS]; l++){
			if(jobs[l][3] == EARLY || jobs[l][3] == pEARLY || jobs[l][3] == EL || jobs[l][3] == pEL)
				sum2 += jobs[l][0];
		}
		
		if(sum2 < sum1)
			return;
		//allocate state
		State *temp = (State *)malloc(sizeof(State));
		*temp = *prnt;
		//setup new state info
		temp -> jobNum = j;
		temp -> children = 0;
		temp -> solnID = 0;
		//allocate space for child schedule
		temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
		//copy prnt schedule to child
		memcpy(temp -> schedules, prnt -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
		temp -> parent = prnt;
		temp -> sumRejectCost += jobs[j][1];

		if(j >= para[NUMJOBS] - 1 ){
			if(checkValidSoln(temp, para, s)){
				markSolution(temp);
				if(*currentSoln != NULL)
					freeOldSoln(*currentSoln);
				*currentSoln = temp;
				nextSolnID %= 4;
				nextSolnID += 1;
				minAchieved = temp -> tCompTime + temp -> sumRejectCost + (para[SCALINGFACTOR] * temp -> maxTardy);
			} else{
				freeRealEstate(temp);
				return;
			}
		}
		//increment the number of child states generated
		prnt -> children += 1;

		if(j < para[NUMJOBS] - 1){ //if job j is not a leaf
			generateChildren(jobs, j + 1, para, s, temp, currentSoln);
		}
	}
}

/*********************************************************************
 * marks a path as a solution with the nextSolnID
 * leaf: a leaf state
 ********************************************************************/
void markSolution(State *soln){
	if(soln -> parent != NULL)
		markSolution(soln -> parent);
	soln -> solnID = nextSolnID;
}

/*******************************************************
 * frees old solution
 * previousSoln: pointer to a leaf state
 ******************************************************/
void freeOldSoln(State *previousSoln){
	if(previousSoln -> parent -> solnID == previousSoln -> solnID)
		freeOldSoln(previousSoln -> parent);
	freeRealEstate(previousSoln);
}

/*****************************************
 * frees the nodes of the linked list of leafs but not the states themselves
 * head: a pointer to the head of a linked list
 *****************************************/
void freeTerminalList(TerminalNode *head){
	//printf("Terminal List: %p\n", head);
	//for traversal
	TerminalNode *current = head;
	while(current -> next != NULL){
		//free all linked nodes
		TerminalNode *temp = current -> next;
		free(current);
		current = temp;
	}
	//free head of list
	free(current);
}

/************************************
 * frees every part of the dp structure
 * head: a pointer to the head of a linked list of all the states
 ************************************/
void freeDP(TerminalNode *head){
	//printf("Node list: %p\n", head);
	//for traversal
	TerminalNode *current = head;
	while(current -> next != NULL){
		TerminalNode *temp = current -> next;
		//free all parts of the state
		free(current -> st -> schedules);
		free(current -> st);
		free(current);
		current = temp;
	}
	//free head of list and state
	free(current -> st -> schedules);
	free(current -> st);
	free(current);
}

/******************************
 * frees the solution returned from reschedule
 * head: a pointer to a state
 ******************************/
void freeSolnList(State *head){
	if(head -> parent != NULL)
		freeSolnList(head -> parent);
	free(head -> schedules);
	free(head);
}

/**************************************************
 * frees a single state in its entirety
 * s: a pointer to the state to be freed
 *************************************************/
void freeRealEstate(State *s){
	free(s -> schedules);
	free(s);
}

/***********************************************************
 * shows which jobs went to which machines, a dot represents jobs in the later schedule
 * head: a pointer to a state
 * m: the number of machines
 * transcribe: an array that converts from internal job numbering to user job numbering
 ***********************************************************/
void reportAssignments(State *head, int m, int *transcribe){
	printf("Rejected: ");
	determineAssignment(head, m, 0, transcribe); //report rejected
	printf("\n");
	for(int i = 1; i <= m; i++){
		printf("Machine %d: ", i);
		determineAssignment(head, m, i, transcribe);
		printf("\n");
	}
	printf("\n");
}

/**************************************************************************
 * a standard backtracking to determine the arrangement of a solution
 * head: a pointer to the head of a solution
 * m: the number of machines
 * machine: the number of the machine to check
 * transcribe: an array that converts from internal job numbering to user job numbering
 **************************************************************************/
void determineAssignment(State *head, int m, int machine, int *transcribe){
	if(head -> parent != NULL && head -> parent -> parent != NULL)
		determineAssignment(head -> parent, m, machine, transcribe);
	//check fro rejects
	if(machine == 0){
		if(head -> parent -> sumRejectCost != head -> sumRejectCost)
			printf("%d ", transcribe[head -> jobNum]);
		return;
	}
	//scan for jobs on a single machine
	if(head -> schedules[machine - 1] != head -> parent -> schedules[machine - 1])
		printf("%d ", transcribe[head -> jobNum]);
	else if(head -> schedules[machine - 1 + m] != head -> parent -> schedules[machine - 1 + m])
		printf(".%d ", transcribe[head -> jobNum]);
}

State *prepass(int **jobs, int *para){
	State *head = (State *)calloc(1, sizeof(State));
	head -> schedules = (int *)calloc(2 * para[NUMMACHINES], sizeof(int));
	head -> parent = NULL;

	int proposed[para[NUMMACHINES]];
	for(int i = 0; i < para[NUMMACHINES]; i++)
		proposed[i] = 0;
	int mTracker = 0;
	for(int i = 0; i < para[NUMJOBS]; i++){
		if(jobs[i][3] == EARLY || jobs[i][3] == pEARLY || jobs[i][3] == EL || jobs[i][3] == pEL){
			proposed[mTracker % para[NUMMACHINES]] += jobs[i][0];
			//allocate state struct
			State *temp = (State *)malloc(sizeof(State));
			*temp = *head;
			//setup new state info
			temp -> jobNum = i;
			//allocate new space for machine schedules
			temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
			//copy prnt schedule to child
			memcpy(temp -> schedules, head -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
			temp -> schedules[mTracker % para[NUMMACHINES]] += jobs[i][0];
			temp -> tCompTime += temp -> schedules[mTracker % para[NUMMACHINES]];
			temp -> parent = head;
			//update max tardiness
			if(temp -> schedules[mTracker % para[NUMMACHINES]] - jobs[i][2] > temp -> maxTardy)
				temp -> maxTardy = temp -> schedules[mTracker % para[NUMMACHINES]] - jobs[i][2];
			head = temp;
			mTracker++;
		} else if(jobs[i][3] == REJECT){	
			//allocate state
			State *temp = (State *)malloc(sizeof(State));
			*temp = *head;
			//setup new state info
			temp -> jobNum = i;
			//allocate space for child schedule
			temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
			//copy prnt schedule to child
			memcpy(temp -> schedules, head -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
			temp -> parent = head;
			temp -> sumRejectCost += jobs[i][1];
			head = temp;
		} else if(jobs[i][3] != LATE){
			freeSolnList(head);
			return NULL;
		}
	}
	for(int i = 0; i < para[NUMMACHINES]; i++){
		if(proposed[i] > para[RD]){
			freeSolnList(head);
			return NULL;
		}
	}
	mTracker = 0;
	for(int i = 0; i < para[NUMJOBS]; i++){
		if(jobs[i][3] == LATE){
			//allocate state
			State *temp = (State *)malloc(sizeof(State));
			*temp = *head;
			//setup new state info
			temp -> jobNum = i;
			//allocate space for child schedule
			temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
			//copy prnt schedule to child
			memcpy(temp -> schedules, head -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
			temp -> schedules[(mTracker % para[NUMMACHINES]) + para[NUMMACHINES]] += jobs[i][0];
			temp -> tCompTime += para[RD] + temp -> schedules[(mTracker % para[NUMMACHINES]) + para[NUMMACHINES]];
			temp -> parent = head;
			//update max tardiness
			if(para[RD] + temp -> schedules[(mTracker % para[NUMMACHINES]) + para[NUMMACHINES]] - jobs[i][2] > temp -> maxTardy)
				temp -> maxTardy = para[RD] + temp -> schedules[(mTracker % para[NUMMACHINES]) + para[NUMMACHINES]] - jobs[i][2];
			mTracker++;
			head = temp;
		}
	}
	return head;
}

int rescheduleNB(int **jobs, int *para, int *s){
	//create inital State
	State *root = (State *)calloc(1, sizeof(State));
	root -> schedules = (int *)calloc(2 * para[NUMMACHINES], sizeof(int));
	root -> parent = NULL;

	//build tree
	State *soln = NULL;
	generateChildren(jobs, 0, para, s, root, &soln);

	if(soln != NULL){
		freeSolnList(soln);
		return 0;
	}

	return -1;
}

