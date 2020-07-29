#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "subsetsum.h"
#include "dp.h"
#include "annotate.h"
#include "main.h"

/**********************************************
 * checks if a number is a possible sum of a given set of values
 * values: a pointer to an array of values
 * n: the number of values
 * sum: the number to check
 **********************************************/
int isPossibleSum(int *values, int n, int sum){
	bool table[n+1][sum+1];

	for(int i = 0; i <= n; i++){
		table[i][0] = true;
	}

	for(int i = 1; i <= sum; i++){
		table[0][i] = false;
	}

	for(int i = 1; i <= n; i++){
		for(int j = 1; j <= sum; j++){
			if(j < values[i-1])
				table[i][j] = table[i-1][j];
			if(j >= values[i-1])
				table[i][j] = table[i-1][j] || table[i-1][j - values[i-1]];
		}
	}

	return (int)table[n][sum];
}

/************************************************************
 * returns the number of possible s values, fills svalues with said values
 * sValues: the address of a pointer to an array
 * jobs: a pointer to a 2d array of jobs
 * para: a pointer to an array of general parameters
 ************************************************************/
int getPossibleSValues(int **sValues, int **jobs, int *para){
	IntNode *pTimes = NULL;
	int length = 0;
	int max = -1; //used to determine the upper bound of s values
	//create a list job processing times from the early schedule
	//printf("jobs: %d\n", para[NUMJOBS]);
	for(int i = 0; i < para[NUMJOBS]; i++){
		switch(jobs[i][3]){
			case EARLY:
			case pEARLY:
			case EL:
			case pEL:
				length += 1;
				if(pTimes == NULL){
					pTimes = (IntNode *)malloc(sizeof(IntNode));
					pTimes -> next = NULL;
					pTimes -> val = jobs[i][0];
					max = jobs[i][0];
				} else{
					IntNode *temp = (IntNode *)malloc(sizeof(IntNode));
					temp -> next = pTimes;
					temp -> val = jobs[i][0];
					pTimes = temp;
					if(jobs[i][0] > max)
						max = jobs[i][0];
				}
		}
	}

	//indicates no jobs in the early schedule, only possible s value is rd
	if(max == -1){
		*sValues = (int *)malloc(sizeof(int));
		(*sValues)[0] = para[RD];
		return 1;
	}

	int set[length];
	//printf("convert early p values to array\n");
	listToArray(set, pTimes, length);
	//printf("free list\n");
	freeList(pTimes);
	//printf("finished early convert\n");

	//printf("RD: %d, max: %d\n", para[RD], max);
	int upper = para[RD] - 1 + max;

	//essentially runs the subsetsum algorithm to generate the table
	bool table[length + 1][upper + 1];
	pTimes = NULL;

	//printf("length: %d, upper: %d\n", length, upper);

	for(int i = 0; i <= length; i++){
		table[i][0] = true;
	}
	for(int i = 1; i <= upper; i++){
		table[0][i] = false;
	}
	for(int i = 1; i <= length; i++){
		for(int j = 1; j <= upper; j++){
			if(j < set[i-1]){
				table[i][j] = table[i-1][j];
			} else{
				table[i][j] = table[i-1][j] || table[i-1][j - set[i-1]];
			}
		}
	}

	//checks which numbers from rd to upper were possible sums
	int validVals = 0;
	for(int i = para[RD]; i <= upper; i++){
		if(table[length][i]){
			if(pTimes == NULL){
				pTimes = (IntNode *)malloc(sizeof(IntNode));
				pTimes -> val = i;
				pTimes -> next = NULL;
			} else{
				IntNode *temp = (IntNode *)malloc(sizeof(IntNode));
				temp -> next = pTimes;
				temp -> val = i;
				pTimes = temp;
			}
			validVals += 1;
		}
	}

	//put possible s values into sValues
	//printf("building result\n");
	if(isPossibleSum(set, length, para[RD])){
		//printf("preadded\n");
		*sValues = (int *)malloc(sizeof(int) * (validVals));
		int temprev[validVals];
		listToArray(temprev, pTimes, validVals);
		for(int i = 0; i < validVals; i++)
			(*sValues)[(validVals - 1) - i] = temprev[i];
	} else{
		//printf("manual add\n");
		validVals += 1;
		*sValues = (int *)malloc(sizeof(int) * validVals);
		int temprev[validVals];
		listToArray(temprev, pTimes, validVals - 1);
		temprev[validVals - 1] = para[RD];
		for(int i = 0; i < validVals; i++)
			(*sValues)[(validVals - 1) - i] = temprev[i];
	}

	freeList(pTimes);

/*	printf("%d candidates: ", validVals);
	for(int i = 0; i < validVals; i++)
		printf("%d ", (*sValues)[i]);
	printf("\n");
*/
	return validVals;
}

/**************************************************
 * converts a linked list to an array
 * array: the array to fill
 * head: the head of the linked list to convert
 * n: the number of items in the linked list
 **************************************************/
void listToArray(int *array, IntNode *head, int n){
	IntNode *current = head;
	for(int i = 0; i < n; i++){
		array[i] = current -> val;
		if(current -> next != NULL)
			current = current -> next;
	}
}

/****************************
 * frees the linked list
 * head: the head of a linked list of type IntNode
 ****************************/
void freeList(IntNode *head){
	if(head == NULL)
		return;
	if(head -> next != NULL)
		freeList(head -> next);
	free(head);
}
