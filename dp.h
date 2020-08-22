#ifndef STATE
#define STATE //prevent double definitions

typedef struct State{
	int jobNum;
	int *schedules; //early/later for each machine, size will be 2m ints
	int tCompTime; //total completion time
	int maxTardy;
	int sumRejectCost;
	struct State *parent;
	unsigned long id;
} State;
#endif

#ifndef GA
#define GA
typedef struct genericarray{
	void *arr;
} genericArr;
#endif

//for creating traversable lists of states
typedef struct TerminalNode{
	State *st;
	struct TerminalNode *next;
} TerminalNode;

//responsible for running the exact algorithm
State *reschedule(genericArr **, int **, int *, int *, int *, float);
void generateChildren(int **, TerminalNode *, TerminalNode **, genericArr **, int, int *, int *, int *, float);
State *prepass(int **, int *);
State *findBestSoln(TerminalNode *, int *);
State *copySoln(State *, int *);

//handle a job based on its annotation
void acceptEarly(int **, genericArr **, int, int *, int *, State *, TerminalNode **, float);
void acceptLate(int **, genericArr **, int, int *, int *, State *, TerminalNode **, float);
void reject(int **, genericArr **, int , int *, int *, State *, TerminalNode **, float);

//report results
void reportAssignments(State *, int, int *);
void determineAssignment(State *, int, int, int *);

//free memory used during dp
void freeTerminalList(TerminalNode *);
void freeSolnList(State *);
void freeRealEstate(State *);

//no backtracking variant
int rescheduleNB(int **, int *, int *);
