#include "types.h"
#include "stat.h"
#include "user.h"


#define N 11
#define E 0.00001
#define T 100.0
#define P 6
#define L 20000
#define NUMTHREADS 8

float fabsm(float a){
	if(a<0)
	return -1*a;
return a;
}
int main(int argc, char *argv[])
{
	float diff;
	int i,j;
	// float mean;

	int count = 0;
	// mean = (3 * T) / 4;

	int pipe_above[NUMTHREADS - 1][2];
	int pipe_below[NUMTHREADS - 1][2];
	int pipe_parent[NUMTHREADS][2];
	int children[NUMTHREADS];

	for(i = 0; i < NUMTHREADS - 1; i++) {
		pipe(pipe_above[i]);
		pipe(pipe_below[i]);
		pipe(pipe_parent[i]);
	}

	pipe(pipe_parent[NUMTHREADS - 1]);

	int parent_pid = getpid();
	for(int k = 0; k < NUMTHREADS; k++) {
		children[k] = fork();
				// printf(1,"children : %d  \n",children[k]);

		if (children[k] == 0) {

			float local_diff = 0.0;
			int size = N / NUMTHREADS;
			if (k == N - 1) size += N % NUMTHREADS;
			if (k == 0 || k == N - 1) size += 1;
			else size += 2;
			float u[size][N];
			float w[size][N];
			// printf(1, "inside1\n");
			// initialization of u's
			if (k == 0) {
				for(i = 0; i < N; i++) 
					u[0][i] = T;
				for(i = 1; i < size - 1; i++) 
					u[i][0] = u[i][N - 1] = T;
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						u[i][j] = (3 * T) / 4;
					}
				}
			}
			// printf(1, "inside2\n");
			else if (k == NUMTHREADS - 1) {
				// printf(1,"%d: ", k);
				for(i = 0; i < N; i++) 
					u[size - 1][i] = 0.0;
				for(i = 1; i < size - 1; i++) 
					u[i][0] = u[i][N - 1] = T;
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						u[i][j] = (3 * T) / 4;
					}
				}
				// printf(1,"%d: ", k);
				
			}
			// printf(1, "inside3\n");
			else {
				for(i = 1; i < size - 1; i++) 
					u[i][0] = u[i][N - 1] = T;
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						u[i][j] = (3 * T) / 4;
					}
				}
			}
			// printf(1, "inside4\n");
			int count = 0;
			for(;;) {
				// write to processes and read
				count++;

				// printf(1,"pid : %d count : %d \n",k, count);
				if (k == 0) {
					for(i = 0; i < N; i++) {
						// write to below
						write(pipe_below[0][1], (char*)&u[size - 2][i], sizeof(float));
					}
					for(i = 0; i < N; i++) {
						// read from below
						read(pipe_above[0][0], (char*)&u[size - 1][i], sizeof(float));
					}
				}

				else if (k == NUMTHREADS - 1) {
					for(i = 0; i < N; i++) {
						// write to above
						write(pipe_above[NUMTHREADS - 2][1], (char*)&u[1][i], sizeof(float));
					}
					for(i = 0; i < N; i++) {
						// read from above
						read(pipe_below[NUMTHREADS - 2][0], (char*)&u[0][i], sizeof(float));
				printf(1,"pid : %d iter : %d count : %d \n",k, i, count);
					}
				}

				else {
					for(i = 0; i < N; i++) {
						// write to below
						write(pipe_below[k][1], (char*)&u[size - 2][i], sizeof(float));
						// write to above
						write(pipe_above[k - 1][1], (char*)&u[1][i], sizeof(float));
					}
					for(i = 0; i < N; i++) {
						// read from below
						read(pipe_above[k][0], (char*)&u[size - 1][i], sizeof(float));
						// read from above
						read(pipe_below[k - 1][0], (char*)&u[0][i], sizeof(float));
				// printf(1,"pid : %d iter : %d count : %d \n",k, i, count);
					}
				printf(1,"pid : %d iter : %d count : %d \n",k, i, count);
				}

				printf(1,"here%d \n ", k);
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						w[i][j] = ( u[i - 1][j] + u[i + 1][j] + u[i][j - 1] + u[i][j + 1]) / 4.0;
						if(fabsm(w[i][j] - u[i][j]) > local_diff )
							local_diff = fabsm(w[i][j]- u[i][j]);
					}
				}

				
				send(getpid(), parent_pid, (char*)&local_diff);
				int signal;
				recv((char*)&signal);
				if(signal == 1) {
					for(i = 1; i < size - 1; i++) {
						for(j = 0; j < N; j++) {
							write(pipe_parent[k][1], (char*)&u[i][j], sizeof(float));
						}
					}		
					exit();
				}

				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						u[i][j] = w[i][j];
					}
				}
			}
		}

	}

	for(;;) {
		diff = 0.0;
		count++;
		// printf(1,"%d ",((int)count));
		for(i = 0; i < NUMTHREADS; i++) {
			float d;
			recv((char*)&d);
			if(diff > d)
				diff = d;
		}
		if(diff <= E || count > L) {
			for(i = 0; i < NUMTHREADS; i++) 
				send(getpid(), children[i], (char*)1);
			break;
		}
	}

	for(int k = 0; k < NUMTHREADS; k++) {
		float val;
		for(i =0; i < N; i++) {
			for(j = 0; j < N; j++) {
				read(pipe_parent[k][0], (char*)&val, sizeof(float));
				// printf(1,"%d ",((int)val));
			}
			printf(1,"\n");
		}
	}
	exit();

}
