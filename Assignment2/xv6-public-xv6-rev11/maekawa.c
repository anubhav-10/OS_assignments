#include "types.h"
#include "stat.h"
#include "user.h"

struct msg {
	char type;
	int id;
};
void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 
void sort(int *arr, int n) {
	for (int i = 0; i < n-1; i++) 
    { 
        int min_idx = i; 
        for (int j = i+1; j < n; j++) 
          if (arr[j] < arr[min_idx]) 
            min_idx = j; 
  
        swap(&arr[min_idx], &arr[i]); 
    } 
}

void critical(int pid, int ti) {
	printf(1, "%d acquired the lock at time %d \n", pid, uptime());
	sleep(ti);
	printf(1, "%d released the lock at time %d \n", pid, uptime());
}

int main(int argc, char *argv[]) {
	if(argc< 2){
		printf(1, "Need type and input filename\n");
		exit();
	}

	int P, P1, P2, P3;
	P = P1 = P2 = P3 = 0;
	char *filename;
	filename = argv[1];
	int fd = open(filename, 0);
	char c;
	int x=0;
	for(int j = 0; j < 4; j++){
		x = 0;
		for(int i=0;;i++){
			read(fd, &c, 1);
			if(c == '\n') break;
			else if(c==' ') continue;
			else {
				x*=10;
				x+= (int)(c - '0');
			}
		}
		if (j == 0)
			P = x;
		else if (j == 1)
			P1 = x;
		else if (j == 2)
			P2 = x;
		else 
			P3 = x;
	}

	// printf(1, "%d %d %d %d\n", P, P1, P2, P3);

	int pipe_req[P][2];
	int pipe_new[P][2];
	int pipe_parent[2];
	int pipe_children[P][2];
	pipe(pipe_parent);
	for(int i = 0; i < P; i++){
		pipe(pipe_req[i]);
		pipe(pipe_new[i]);
		pipe(pipe_children[i]);
	}

	int size = P2 + P3;
	int grants;
	int i, j, k;
	for(k = 0; k < P; k++) {
		if (fork() == 0) {
			int m = k;
			if (k >= P1) {
				for(int z = 0; z < P; z++) {
					write(pipe_req[z][1], (char*)&m, sizeof(int));
				}
			}
			// send to parent
			write(pipe_parent[1], (char*)&m, sizeof(int));
 			// recv permission
 			read(pipe_children[k][0], (char*)&m, sizeof(int));
 			int request[size];
			for(i = 0; i < size; i++) {
				int id;
				read(pipe_req[k][0], (char*)&id, sizeof(int));
				request[i] = id;
			}
			sort(request, size);

			// printf(1, "pid %d \n", k);
			// for(int i = 0; i < size; i++)
			// 	printf(1, "pid %d : %d ",k, request[i]);
			// printf(1, "\n");

			grants = 0;
			for(i = 0; i < size; i++) {
				struct msg ms;
				ms.type = 'G';
				ms.id = m;

				write(pipe_new[request[i]][1], (char*)&ms, sizeof(struct msg));
				for(;;) {
					struct msg ret;
					read(pipe_new[k][0], (char*)&ret, sizeof(struct msg));
					// printf(1, "pid %d type %c grant id %d\n",k, ret.type, ret.id);
					// for(int z=0;z<10;z++)
					// 	printf(1, "pid: %d request: %c %d\n", k, ret.type, ret.id);
					// sleep(1);
					if (ret.type == 'R'){
						// printf(1, "pid %d received release from %d\n", k, ret.id);
						break;
					}
					else if (ret.type == 'G') {
						// printf(1, "pid %d received grant from %d\n", k, ret.id);
						// int flag = 1;
						grants++;
						// printf(1, "pid %d grants %d\n", k, grants);
						if (grants == P) {
							int time = 0;
							if(k >= (P1 + P2)) time = 2;
							// printf(1, "%d Entered critical \n", k);
							// printf(1, "%d Exited critical \n", k);
							// if(k >= P1)
							critical(getpid(), time);
							struct msg release;
							release.type = 'R';
							release.id = m;
							for(j = 0; j < P; j++) {
								write(pipe_new[j][1], (char*)&release, sizeof(struct msg));
								// sleep(0.5);
							}
							grants = 0;
						}
					}
				}
				// printf(1, "pid %d iter %d\n", k, i);
			}
			// printf(1, "pid %d exited\n", k);
			exit();
		}
	}

	for(int k = 0; k < P; k++) {
		int id;
		read(pipe_parent[0], (char*)&id, sizeof(int));
		sleep(0.5);
		write(pipe_children[id][1], (char*)&id, sizeof(int));
		// printf(1, "%d\n", id);
	}

	for(int k = 0; k < P; k++)
		wait();

	exit();
}
