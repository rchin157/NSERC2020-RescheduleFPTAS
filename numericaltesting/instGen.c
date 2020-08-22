#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(){
	srand(time(NULL));
	struct stat st = {0};

	if(stat("./instances", &st) == -1){
		mkdir("./instances", 0777);
	}

	int m[5] = {2, 3, 4, 6, 8};
	int numJobs[17] = {4, 6, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 30, 40, 50};

	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 17; j++){
			if(numJobs[j] < m[i])
				continue;
			char dirname[25];
			sprintf(dirname, "./instances/%d_%d", m[i], numJobs[j]);
			if(stat(dirname, &st) == -1){
				mkdir(dirname, 0777);
			}
			for(int k = 0; k < 20; k++){
				char newjobs[60];
				sprintf(newjobs, "./instances/%d_%d/jobs%d.txt", m[i], numJobs[j], k);
				FILE *jobFile = fopen(newjobs, "w");
				fprintf(jobFile, "%d\n", numJobs[j]);
				int psum = 0;
				int esum = 0;
				for(int l = 0; l < numJobs[j]; l++){
					int pl = (rand() % 30) + 1;
					int el = ((rand() % 10) + 1) * pl;
					psum += pl;
					esum += el;
					fprintf(jobFile, "%d %d\n", pl, el);
				}
				fclose(jobFile);
				
				char newinst[40];
				sprintf(newinst, "./instances/%d_%d/instance%d.txt", m[i], numJobs[j], k);
				FILE *instFile = fopen(newinst, "w");
				//set rd
				int scale = rand() % 3;
				int rd;
				if(scale == 0)
					rd = psum / 3;
				else if(scale == 1)
					rd = psum / 2;
				else
					rd = (psum * 2) / 3;
				fprintf(instFile, "%d\n", rd);
				//set number of delayed jobs
				scale = rand() % 3;
				int d;
				if(scale == 0)
					d = numJobs[j] / 10;
				else if(scale == 1)
					d = numJobs[j] / 5;
				else
					d = numJobs[j] / 2;
				fprintf(instFile, "%d\n", d);
				//set delayed jobs
				int tempSet[d];
				for(int h = 0; h < d; h++){
					tempSet[h] = (rand() % numJobs[j]) + 1;
					int unique = 0;
					while(!unique){
						unique = 1;
						for(int hh = 0; hh < h; hh++){
							if(tempSet[hh] == tempSet[h])
								unique = 0;
						}
						if(!unique)
							tempSet[h] = (rand() % numJobs[j]) + 1;
					}
				}
				for(int h = 0; h < d; h++)
					fprintf(instFile, "%d ", tempSet[h]);
				fprintf(instFile, "\n");
				//set total rejection cost
				scale = rand() % 3;
				int h;
				if(scale == 0)
					h = esum / 3;
				else if(scale == 1)
					h = esum / 2;
				else
					h = (esum * 2) / 3;
				fprintf(instFile, "%d\n", h);
				//set scaling factor
				fprintf(instFile, "3\n");
				//set number of machines
				fprintf(instFile, "%d\n", m[i]);
				//set epsilon
				fprintf(instFile, "10.0\n");
				//set filename
				sprintf(newjobs, "./numericaltesting/instances/%d_%d/jobs%d.txt", m[i], numJobs[j], k);
				fprintf(instFile, "%s\n", newjobs);
				fclose(instFile);
			}

		}
	}

}
