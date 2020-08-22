/* Written by Rylan Chin, Finalized August 2020
 * This file handles the actual execution of a reschedule given the parameters
 * and returns the solution associated with the minimum objective function.
 * A dynamic programming approach is used to compute a more sparse number
 * of states based on the given epsilon value.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "dp.h"
#include "main.h"
#include "annotate.h"
#include "util.h"

//global minimum
int minAchieved = -1;
unsigned long stateID = 1;

/************************************************
 * runs the dp algorithm according to the provided jobs and parameters, returning a linked list solution
 * table: an array of genericArr pointers, the dp table
 * jobs: a pointer to a 2d array of jobs
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times for this problem
 * vl: an array of vl values that define the table dimensions
 * delta: the scaling value calculated from epsilon
 ************************************************/
State *reschedule(genericArr **table, int **jobs, int *para, int *s, int *vl, float delta){
	stateID = 1;
	//create inital State
	State *root = (State *)calloc(1, sizeof(State));
	root -> schedules = (int *)calloc(2 * para[NUMMACHINES], sizeof(int));
	root -> parent = NULL;

	TerminalNode *parents = (TerminalNode *)malloc(sizeof(TerminalNode));
	parents -> st = root;
	parents -> next = NULL;
	TerminalNode *children = NULL;

	//build table
	for(int i = 0; i < para[NUMJOBS]; i++){
		//printf("Working on job %d...\n", i + 1);
		generateChildren(jobs, parents, &children, table, i, para, s, vl, delta);
		freeTerminalList(parents);
		if(children == NULL){
			freeRealEstate(root);
			return NULL;
		}
		parents = children;
		children = NULL;
	}

	//printf("States: %lu\n", stateID);

	//find solution
	State *soln = findBestSoln(parents, para);

	if(parents != NULL)
		freeTerminalList(parents);
	freeRealEstate(root);

	if(soln == NULL){
		return NULL;
	}

	if(minAchieved == -1 || soln -> tCompTime + soln -> sumRejectCost + (para[SCALINGFACTOR] * soln -> maxTardy) < minAchieved){
		minAchieved = soln -> tCompTime + soln -> sumRejectCost + (para[SCALINGFACTOR] * soln -> maxTardy);
		return soln;
	}
	return NULL;
}

/****************************************************************************************************************
 * adds all possiblilites of the next job to the tree based on its label
 * jobs: a pointer to a 2d array of jobs
 * parents: linked list of all states from the previous job
 * children: address of a linked list to put new states in
 * table: an array of genericArr pointers, the dp table
 * j: job number
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of start times for the machines
 * vl: an array of vl values that define the table dimensions
 * delta: the scaling value calculated from epsilon
 ****************************************************************************************************************/
void generateChildren(int **jobs, TerminalNode *parents, TerminalNode **children, genericArr **table, int j, int *para, int *s, int *vl, float delta){
	TerminalNode *current = parents;
	while(current != NULL){
		switch(jobs[j][3]){
			case ONTIME:
				acceptEarly(jobs, table, j, para, s, current -> st, children, delta);
				acceptLate(jobs, table, j, para, s, current -> st, children, delta);
				reject(jobs, table, j, para, s, current -> st, children, delta);
				break;
			case DELAYED:
				acceptLate(jobs, table, j, para, s, current -> st, children, delta);
				reject(jobs, table, j, para, s, current -> st, children, delta);
				break;
		}
		current = current -> next;
	}
}

/***********************************************************************************************************
 * handles node generation for jobs that are to be accepted into the early schedule
 * jobs: a pointer to a 2d array of jobs
 * table: an array of genericArr pointers, the dp table
 * j: job number of the children to be generated
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times
 * prnt: a pointer to the node in the tree that these children stemmed from
 * children: address of a linked list to put new states in
 * delta: the scaling value calculated from epsilon
 ***********************************************************************************************************/
void acceptEarly(int **jobs, genericArr **table, int j, int *para, int *s, State *prnt, TerminalNode **children, float delta){
	for(int i = 0; i < para[NUMMACHINES]; i++){
		//for each machine where the job can be added a state is generated
		if(prnt -> schedules[i] < para[RD] && prnt -> schedules[i] + jobs[j][0] <= s[i]){
			//check if over global min, if so the state is not generated
			if((minAchieved != -1) && (prnt -> schedules[i] + jobs[j][0] + prnt -> tCompTime + prnt -> sumRejectCost) + (prnt -> maxTardy >
				(prnt -> schedules[i] + jobs[j][0] - jobs[j][2]) ? para[SCALINGFACTOR] * prnt -> maxTardy :
			       	para[SCALINGFACTOR] * (prnt -> schedules[i] + jobs[j][0] - jobs[j][2])) >= minAchieved)
				continue;
			//get grid
			//a grid is identified by a 2m + 3 long vector
			int gridPos[(2 * para[NUMMACHINES]) + 3];
			for(int k = 0; k < (2 * para[NUMMACHINES]) + 3; k++){
				if(k < 2 * para[NUMMACHINES]){
					double val = (double)(k == i ? prnt -> schedules[k] + jobs[j][0] : prnt -> schedules[k]);
					if(val < 1.0f)
						val = 1.0f;
					gridPos[k] = (int)floor(myLog((double)delta, val));
				} else if(k == 2 * para[NUMMACHINES]){
					gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> tCompTime > 0 ? prnt -> tCompTime : 1)));
				} else if(k == (2 * para[NUMMACHINES]) + 1){
					gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> maxTardy > 0 ? prnt -> maxTardy : 1)));
				} else if(k == (2 * para[NUMMACHINES]) + 2){
					gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> sumRejectCost > 0 ? prnt -> sumRejectCost : 1)));
				}
			}
			//check determined grid
			State *gridState = arrGet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3);
			if(gridState != NULL && gridState -> sumRejectCost <= prnt -> sumRejectCost)
				continue;
			//allocate state struct
			State *temp = (State *)malloc(sizeof(State));
			*temp = *prnt;
			//setup new state info
			temp -> jobNum = j;
			//allocate new space for machine schedules
			temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
			//copy prnt schedule to child
			memcpy(temp -> schedules, prnt -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
			temp -> schedules[i] += jobs[j][0];
			temp -> tCompTime += temp -> schedules[i];
			temp -> parent = prnt;
			temp -> id = stateID;
			stateID++;
			//update max tardiness
			if(temp -> schedules[i] - jobs[j][2] > temp -> maxTardy)
				temp -> maxTardy = temp -> schedules[i] - jobs[j][2];
			//set grid
			if(gridState != NULL && gridState -> sumRejectCost > prnt -> sumRejectCost){
				//update list
				TerminalNode *current = *children;
				while(current != NULL){
					if(current -> st -> id == gridState -> id){
						freeRealEstate(gridState);
						current -> st = temp;
						arrSet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3, temp);
						break;
					}
					current = current -> next;
				}
			} else if(gridState == NULL){
				arrSet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3, temp);
				//add to list
				TerminalNode *listTemp = (TerminalNode *)malloc(sizeof(TerminalNode));
				listTemp -> st = temp;
				listTemp -> next = *children;
				*children = listTemp;
			}
		}
	}
}

/***********************************************************************************************************
 * handles node generation for jobs that are to be accepted into the later schedule
 * jobs: a pointer to a 2d array of jobs
 * table: an array of genericArr pointers, the dp table
 * j: job number of the children to be generated
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times
 * prnt: a pointer to the node in the tree that these children stemmed from
 * children: address of a linked list to put new states in
 * delta: the scaling value calculated from epsilon
 ***********************************************************************************************************/
void acceptLate(int **jobs, genericArr **table, int j, int *para, int *s, State *prnt, TerminalNode **children, float delta){
	for(int i = para[NUMMACHINES]; i < 2 * para[NUMMACHINES]; i++){
		//check if over global min, if so the state is not generated
		if(minAchieved != -1 && 
			(s[i - para[NUMMACHINES]] + prnt -> schedules[i] + jobs[j][0] + prnt -> tCompTime + prnt -> sumRejectCost) + (prnt -> maxTardy
			> (s[i - para[NUMMACHINES]] + prnt -> schedules[i] + jobs[j][0] - jobs[j][2]) ?
			para[SCALINGFACTOR] * prnt -> maxTardy : 
			para[SCALINGFACTOR] * (s[i - para[NUMMACHINES]] + prnt -> schedules[i] + jobs[j][0] - jobs[j][2])) >= minAchieved)
			continue;
		//get grid
		//a grid is identified by a 2m + 3 long vector
		int gridPos[(2 * para[NUMMACHINES]) + 3];
		for(int k = 0; k < (2 * para[NUMMACHINES]) + 3; k++){
			if(k < 2 * para[NUMMACHINES]){
				double val = (double)(k == i ? prnt -> schedules[k] + jobs[j][0] : prnt -> schedules[k]);
				if(val < 1.0f)
					val = 1.0f;
				gridPos[k] = (int)floor(myLog((double)delta, val));
			} else if(k == 2 * para[NUMMACHINES]){
				gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> tCompTime > 0 ? prnt -> tCompTime : 1)));
			} else if(k == (2 * para[NUMMACHINES]) + 1){
				gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> maxTardy > 0 ? prnt -> maxTardy : 1)));
			} else if(k == (2 * para[NUMMACHINES]) + 2){
				gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> sumRejectCost > 0 ? prnt -> sumRejectCost : 1)));
			}
		}
		//check determined grid
		State *gridState = arrGet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3);
		if(gridState != NULL && gridState -> sumRejectCost <= prnt -> sumRejectCost)
			continue;
		//allocate state struct
		State *temp = (State *)malloc(sizeof(State));
		*temp = *prnt;
		//setup new state info
		temp -> jobNum = j;
		//allocate new space for machine schedules
		temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
		//copy prnt schedule to child
		memcpy(temp -> schedules, prnt -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
		temp -> schedules[i] += jobs[j][0];
		temp -> tCompTime += temp -> schedules[i];
		temp -> parent = prnt;
		temp -> id = stateID;
		stateID++;
		//update max tardiness
		if(s[i - para[NUMMACHINES]] + temp -> schedules[i] - jobs[j][2] > temp -> maxTardy)
			temp -> maxTardy = s[i - para[NUMMACHINES]] + temp -> schedules[i] - jobs[j][2];
		//set grid
		if(gridState != NULL && gridState -> sumRejectCost > prnt -> sumRejectCost){
			//update list
			TerminalNode *current = *children;
			while(current != NULL){
				if(current -> st -> id == gridState -> id){
					freeRealEstate(gridState);
					current -> st = temp;
					arrSet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3, temp);
					break;
				}
				current = current -> next;
			}
		} else if(gridState == NULL){
			arrSet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3, temp);
			//add to list
			TerminalNode *listTemp = (TerminalNode *)malloc(sizeof(TerminalNode));
			listTemp -> st = temp;
			listTemp -> next = *children;
			*children = listTemp;
		}
	}
}

/***********************************************************************************************************
 * handles node generation for jobs that are to be rejected
 * jobs: a pointer to a 2d array of jobs
 * table: an array of genericArr pointers, the dp table
 * j: job number of the children to be generated
 * para: a pointer to an array of general parameters
 * s: a pointer to an array of machine start times
 * prnt: a pointer to the node in the tree that these children stemmed from
 * children: address of a linked list to put new states in
 * delta: the scaling value calculated from epsilon
 ***********************************************************************************************************/
void reject(int **jobs, genericArr **table, int j, int *para, int *s, State *prnt, TerminalNode **children, float delta){
	if(prnt -> sumRejectCost + jobs[j][1] <= para[REJECTIONCOSTBOUND]){
		//check if over global min, if so state is not generated
		if(minAchieved != -1 && (prnt -> tCompTime + (prnt -> sumRejectCost + jobs[j][1]) + (prnt -> maxTardy * para[SCALINGFACTOR])) >= minAchieved)
			return;
		//get grid
		//a grid is identified by a 2m + 3 long vector
		int gridPos[(2 * para[NUMMACHINES]) + 3];
		for(int k = 0; k < (2 * para[NUMMACHINES]) + 3; k++){
			if(k < 2 * para[NUMMACHINES]){
				double val = (double)(prnt -> schedules[k]);
				if(val < 1.0f)
					val = 1.0f;
				gridPos[k] = (int)floor(myLog((double)delta, val));
			} else if(k == 2 * para[NUMMACHINES]){
				gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> tCompTime > 0 ? prnt -> tCompTime : 1)));
			} else if(k == (2 * para[NUMMACHINES]) + 1){
				gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> maxTardy > 0 ? prnt -> maxTardy : 1)));
			} else if(k == (2 * para[NUMMACHINES]) + 2){
				gridPos[k] = (int)floor(myLog((double)delta, (double)(prnt -> sumRejectCost + jobs[j][1])));
			}
		}
		//check determined grid
		State *gridState = arrGet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3);
		if(gridState != NULL && gridState -> sumRejectCost <= prnt -> sumRejectCost + jobs[j][1]){
			return;
		}
		//allocate state struct
		State *temp = (State *)malloc(sizeof(State));
		*temp = *prnt;
		//setup new state info
		temp -> jobNum = j;
		//allocate new space for machine schedules
		temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
		//copy prnt schedule to child
		memcpy(temp -> schedules, prnt -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
		temp -> parent = prnt;
		temp -> sumRejectCost += jobs[j][1];
		temp -> id = stateID;
		stateID++;
		//set grid
		if(gridState != NULL && gridState -> sumRejectCost > prnt -> sumRejectCost + jobs[j][1]){
			//update list
			TerminalNode *current = *children;
			while(current != NULL){
				if(current -> st -> id == gridState -> id){
					freeRealEstate(gridState);
					current -> st = temp;
					break;
				}
				current = current -> next;
			}
			arrSet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3, temp);
		} else if(gridState == NULL){
			arrSet(table[j], gridPos, (2 * para[NUMMACHINES]) + 3, temp);
			//add to list
			TerminalNode *listTemp = (TerminalNode *)malloc(sizeof(TerminalNode));
			listTemp -> st = temp;
			listTemp -> next = *children;
			*children = listTemp;
		}
	}
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

/******************************
 * frees the solution returned from reschedule
 * head: a pointer to a state
 ******************************/
void freeSolnList(State *head){
	if(head -> parent != NULL)
		freeSolnList(head -> parent);
	freeRealEstate(head);
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

State *findBestSoln(TerminalNode *list, int *para){
	if(list == NULL)
		return NULL;
	TerminalNode *current = list;
	State *minSt = current -> st;
	int minObj = (current -> st -> tCompTime) + (current -> st -> sumRejectCost) + (current -> st -> maxTardy * para[SCALINGFACTOR]);
	while(current -> next != NULL){
		int tempObj = (current -> st -> tCompTime) + (current -> st -> sumRejectCost) + (current -> st -> maxTardy * para[SCALINGFACTOR]);
		if(tempObj < minObj){
			minObj = tempObj;
			minSt = current -> st;
		}
		current = current -> next;
	}
	int tempObj = (current -> st -> tCompTime) + (current -> st -> sumRejectCost) + (current -> st -> maxTardy * para[SCALINGFACTOR]);
	if(tempObj < minObj){
		minObj = tempObj;
		minSt = current -> st;
	}

	//copy soln
	minSt = copySoln(minSt, para);
	return minSt;
}

State *copySoln(State *leaf, int *para){
	if(leaf == NULL)
		return NULL;
	State *temp = (State *)malloc(sizeof(State));
	*temp = *leaf;
	temp -> schedules = (int *)malloc(sizeof(int) * 2 * para[NUMMACHINES]);
	memcpy(temp -> schedules, leaf -> schedules, sizeof(int) * 2 * para[NUMMACHINES]);
	temp -> parent = copySoln(leaf -> parent, para);
	return temp;
}
