#include <math.h>
#include <stdlib.h>
#include "main.h"

void determineS(int *para, int **s, float delta, int *vl){
	*s = (int *)malloc(sizeof(int) * vl[0]);
	for(int i = 0; i < vl[0]; i++)
		(*s)[i] = (int)floor((double)para[RD] * pow((double)delta, (double)i));
}


