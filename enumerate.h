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

//enumerate on s values
State *enumerate(int **, int *, int *, float, int *);
void runCombinations(int *, int [], int, int, int, int, int **, int *, State **, int *, int *, float);

