#define STATE //prevent double definitions

typedef struct State{
	int jobNum;
	int *schedules; //early/later for each machine, size will be 2m ints
	int tCompTime; //total completion time
	int maxTardy;
	int sumRejectCost;
	int children;
	short solnID;
	struct State *parent;
} State;

//for creating traversable lists of states
typedef struct TerminalNode{
	State *st;
	struct TerminalNode *next;
} TerminalNode;

//responsible for running the exact algorithm
State *reschedule(int **, int *, int *, int *, float);
void generateChildren(int **, int, int *, int *, State *, State **);
State *prepass(int **, int *);
void markSolution(State *);
int checkValidSoln(State *, int *, int *);

//handle a job based on its annotation
void acceptEarly(int **, int, int *, int *, State *, State **);
void acceptLate(int **, int, int *, int *, State *, State **);
void reject(int **, int , int *, int *, State *, State **);

//report results
void reportAssignments(State *, int, int *);
void determineAssignment(State *, int, int, int *);

//free memory used during dp
void freeTerminalList(TerminalNode *);
void freeDP(TerminalNode *);
void freeSolnList(State *);
void freeRealEstate(State *);
void freeOldSoln(State *);

//no backtracking variant
int rescheduleNB(int **, int *, int *);
