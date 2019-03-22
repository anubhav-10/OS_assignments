#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 

#define N 11
#define E 0.00001
#define T 100.0
#define P 6
#define L 20000
#define NUMTHREADS 3

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
	int pipe_signal[NUMTHREADS][2];
	int pipe_diff[NUMTHREADS][2];
	int children[NUMTHREADS];

	for(i = 0; i < NUMTHREADS - 1; i++) {
		pipe(pipe_above[i]);
		pipe(pipe_below[i]);
		pipe(pipe_parent[i]);
		pipe(pipe_signal[i]);
		pipe(pipe_diff[i]);
	}

	pipe(pipe_parent[NUMTHREADS - 1]);
	pipe(pipe_signal[NUMTHREADS - 1]);
	pipe(pipe_diff[NUMTHREADS - 1]);

	// int parent_pid = getpid();
	for(int k = 0; k < NUMTHREADS; k++) {
		children[k] = fork();
				// printf("children : %d  \n",children[k]);

		if (children[k] == 0) {

			float local_diff = 0.0;
			int size = N / NUMTHREADS;
			if (k == NUMTHREADS - 1) size += N % NUMTHREADS;
			if (k == 0 || k == NUMTHREADS - 1) size += 1;
			else size += 2;
			float u[size][N];
			float w[size][N];
			// printf( "inside1\n");
			// initialization of u's
			if (k == 0 && NUMTHREADS == 1) {
				size = N;
				for(i = 0; i < N; i++) {
					u[0][i] = u[i][0] = u[i][N - 1] = T;
					u[N - 1][i] = 0.0;
				}
					
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						u[i][j] = (3 * T) / 4;
					}
				}
			}
			else if (k == 0) {
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
			// printf( "inside2\n");
			else if (k == NUMTHREADS - 1) {
				// printf("%d: ", k);
				u[size - 1][0] = T;
				for(i = 1; i < N; i++) 
					u[size - 1][i] = 0.0;
				for(i = 1; i < size - 1; i++) 
					u[i][0] = u[i][N - 1] = T;
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						u[i][j] = (3 * T) / 4;
					}
				}
				// printf("%d: ", k);
				
			}
			// printf( "inside3\n");
			else {
				for(i = 1; i < size - 1; i++) 
					u[i][0] = u[i][N - 1] = T;
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						u[i][j] = (3 * T) / 4;
					}
				}
			}
			// printf( "inside4\n");
			int count = 0;
			for(;;) {
				// write to processes and read
				count++;

				if (k == 0 && NUMTHREADS == 1) {

				}
				else if (k == 0 && NUMTHREADS != 1) {
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
				// printf("pid : %d count : %d \n",k, count);
					}
					for(i = 0; i < N; i++) {
						// read from above
						read(pipe_below[NUMTHREADS - 2][0], (char*)&u[0][i], sizeof(float));
				// printf("pid : %d iter : %d count : %d \n",k, i, count);
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
						// if (k==1) printf( "GOing to read\n");
						read(pipe_above[k][0], (char*)&u[size - 1][i], sizeof(float));
						// read from above

						// if (k==1) printf( "read1\n");
						read(pipe_below[k - 1][0], (char*)&u[0][i], sizeof(float));

						// if (k==1) printf( "read2\n");
				// printf("pid : %d iter : %d count : %d \n",k, i, count);
					}
				// printf("pid : %d iter : %d count : %d \n",k, i, count);
				}

				// printf("here%d \n ", k);
				for(i = 1; i < size - 1; i++) {
					for(j = 1; j < N - 1; j++) {
						w[i][j] = ( u[i - 1][j] + u[i + 1][j] + u[i][j - 1] + u[i][j + 1]) / 4.0;
						if(fabsm(w[i][j] - u[i][j]) > local_diff )
							local_diff = fabsm(w[i][j]- u[i][j]);
					}
				}

				
				// send(getpid(), parent_pid, (char*)&local_diff);
				write(pipe_diff[k][1], (char*)&local_diff, sizeof(float));
				int signal;
				// printf("before receiving signal%d \n ", k);
				// recv((char*)&signal);
				read(pipe_signal[k][0], (char*)&signal, sizeof(int));
				// printf("after receiving signal pid:%d signal: %d\n ", k, signal);

				if(signal == 1) {
					if (k == 0) {
						if (NUMTHREADS == 1) {
							for(i = 0; i < N; i++) {
								for(j = 0; j < N; j++) {
									write(pipe_parent[k][1], (char*)&u[i][j], sizeof(float));
								}
							}
						}
						else {
							for(i = 0; i < size - 1; i++) {
								for(j = 0; j < N; j++) {
									write(pipe_parent[k][1], (char*)&u[i][j], sizeof(float));
								}
							}
						}
					}		
					else if (k == NUMTHREADS - 1) {
						for(i = 1; i < size; i++) {
							for(j = 0; j < N; j++) {
								write(pipe_parent[k][1], (char*)&u[i][j], sizeof(float));
							}
						}
					}		
					else {
						for(i = 1; i < size - 1; i++) {
							for(j = 0; j < N; j++) {
								write(pipe_parent[k][1], (char*)&u[i][j], sizeof(float));
							}
						}
					}		
					exit(0);
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
		for(i = 0; i < NUMTHREADS; i++) {
			float d;
			// recv((char*)&d);
			read(pipe_diff[i][0], (char*)&d, sizeof(float));
			// printf("local diff : %d ",((int)d));
			if(diff < d)
				diff = d;
		}
		if(diff <= E || count > L) {
			int sig = 1;
			// printf( "%d %d %d %d\n", (int)diff, (int)E, count, L);
			for(i = 0; i < NUMTHREADS; i++) 
				write(pipe_signal[i][1], (char*)&sig, sizeof(int));
				// send(getpid(), children[i], (char*)&sig);
			break;
		}
		else {
			int sig = 0;
			for(i = 0; i < NUMTHREADS; i++) { 
				write(pipe_signal[i][1], (char*)&sig, sizeof(int));
				// send(getpid(), children[i], (char*)&sig);			
				// printf("ibefor sig %d ccount: %d\n", sig, count);
			}
		}
	}

	for(int k = 0; k < NUMTHREADS; k++) {
		int size = N / NUMTHREADS;
		if (k == NUMTHREADS - 1) size += (N % NUMTHREADS);
		if (k == 0 || k == NUMTHREADS - 1) size += 1;
		else size += 2;
		float val;
		// if (k == 0 && NUMTHREADS == 1) size = N;
		if (k == 0) {
			for(i =0; i < size - 1; i++) {
				for(j = 0; j < N; j++) {
					read(pipe_parent[k][0], (char*)&val, sizeof(float));
					printf("%d ",((int)val));
				}
				printf("\n");
			}
		}
		else if (k == NUMTHREADS - 1) {
			// printf("%d\n", size);
			for(i =1; i < size; i++) {
				for(j = 0; j < N; j++) {
					read(pipe_parent[k][0], (char*)&val, sizeof(float));
					printf("%d ",((int)val));
				}
				printf("\n");
			}
		}
		else {
			for(i =1; i < size-1; i++) {
				for(j = 0; j < N; j++) {
					read(pipe_parent[k][0], (char*)&val, sizeof(float));
					printf("%d ",((int)val));
				}
				printf("\n");
			}			
		}
	}
	exit(0);

}
