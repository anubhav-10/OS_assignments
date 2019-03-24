#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include<time.h>

struct message {
	char type;
	int from;
	int to;
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

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Need input filename\n");
		exit(0);
	}

	int P, P1, P2, P3;
	int i, j, k;

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

	printf("%d %d %d %d\n", P, P1, P2, P3);

	int pipe_messages[P][2];
	int pipe_parent[2];
	int pipe_children[P][2];
	pipe(pipe_parent);
	for(int i = 0; i < P; i++) {
		pipe(pipe_messages[i]);
		pipe(pipe_children[i]);
	}

	int size = P2 + P3;
	int grants_count;

	for(k = 0; k < P; k++) {
		if (fork() == 0) {
			if (k >= P1) {
				for(i = 0; i < P; i++) {
					write(pipe_messages[i][1], (char*)&k, sizeof(int));
				}
			}

			// send to parent
			write(pipe_parent[1], (char*)&k, sizeof(int));
			// receive permission
			read(pipe_children[k][0], (char*)&k, sizeof(int));
		
			int request[size];
			for(i = 0; i < size; i++) {
				read(pipe_messages[k][0], (char*)&request[i], sizeof(int));
			}

			sort(request, size);

			grants_count = 0;
			for(i = 0; i < size; i++) {
				struct message grant;
				grant.type = 'G';
				grant.from = k;
				grant.to = request[i];
				int req = request[i];
				write(pipe_messages[req][1], (char*)&grant, sizeof(struct message));
				for(;;) {
					struct message get;
					read(pipe_messages[k][0], (char*)&get, sizeof(struct message));

					if(get.type == 'R') {
						break;
					}
					else if(get.type == 'G') {
						grants_count++;
						printf("%d\n", grants_count);
						if(grants_count == P) {
							printf("pid %d lock\n", k);
						}
						struct message release;
						release.type = 'R';
						release.from = k;
						for(j = 0; j < P; j++) {
							write(pipe_messages[j][1], (char*)&release, sizeof(struct message));
						}
						grants_count = 0;
					}
				}
			}
			exit(0);
		}
	}

	for(k = 0; k < P; k++) {
		int id;
		read(pipe_parent[0], (char*)&id, sizeof(int));
		sleep(0.5);
		write(pipe_children[id][1], (char*)&id, sizeof(int));
	}

	for(int k = 0; k < P; k++) 
		wait(0);

	return 0;
}