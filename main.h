#define RD 0
#define REJECTIONCOSTBOUND 1
#define MAXTARDINESS 2
#define SCALINGFACTOR 3
#define NUMMACHINES 4
#define NUMDELAYED 5
#define NUMJOBS 6

#ifndef STATE
typedef struct State{
	int jobNum;
	int *schedules; //early/later for each machine, size will be 2m ints
	int tCompTime; //total completion time
	int maxTardy;
	int sumRejectCost;
	struct State *parent;
} State;
#endif

