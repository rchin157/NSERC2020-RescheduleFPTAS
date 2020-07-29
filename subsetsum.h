//a struct for creating linked lists of ints
typedef struct IntNode{
	int val;
	struct IntNode *next;
} IntNode;

//used to determine possible s values
int isPossibleSum(int *, int, int);
int getPossibleSValues(int **, int **, int *);
void listToArray(int *, IntNode *, int);
void freeList(IntNode *);
