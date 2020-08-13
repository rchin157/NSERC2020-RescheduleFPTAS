typedef struct genericarray{
	void *arr;
} genericArr;

//get info from user
void getParams(int *, int **, int **, float *);
void getSValues(int **, int);
//get jobs from file
void getJobs(int ***, int *, int *, int *);
//perform quicksort on jobs
void sortJobs(int **, int, int, int);
int partitionJobs(int **, int, int, int);
//free various things from main
void cleanupMem(int *, int **, int, int *);
//handle program args
int processArgs(int, char *[], int *);
void reportBadArgs();
//print jobs in readable format
void printJobs(int **, int);
//set completion times accordingly
void calculateCj(int **, int, int);
//generic quicksort implementation
void quickSort(int *, int, int);
int partition(int *, int, int);
//sparsness
//long double calculateProblemSparsness(int *, int **);
//fptas numbers
void calcDeltaV0(int **, int *, float *, float *, int **);
void calcVl(int **, int *, float, int *, int *);
double myLog(double, double);
int pMax(int **, int *);
//ndim array stuff
genericArr *buildArr(int *, int, int);
int arrGet(genericArr *, int *, int);
void arrSet(genericArr *, int *, int, int);
void freeArr(genericArr *, int *, int, int);
