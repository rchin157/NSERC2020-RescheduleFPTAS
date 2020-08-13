#define RD 0
#define REJECTIONCOSTBOUND 1
#define SCALINGFACTOR 2
#define NUMMACHINES 3
#define NUMDELAYED 4
#define NUMJOBS 5

#ifndef STATE
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
#endif

