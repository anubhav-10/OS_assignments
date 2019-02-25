#include "types.h"
#include "stat.h"
#include "user.h"
#define THREADCOUNT 8

// char* toArray(int n){
// 	int l = 8;
// 	char num[8];
// 	for(int i=0;i<8;i++, n/=10){
// 		num[i] = 
// 	}
// }

int
main(int argc, char *argv[])
{
	if(argc< 2){
		printf(1,"Need type and input filename\n");
		exit();
	}
	char *filename;
	filename=argv[2];
	int type = atoi(argv[1]);
	printf(1,"Type is %d and filename is %s\n",type, filename);

	int tot_sum = 0;	
	float variance = 0.0;

	int size=1000;
	short arr[size];
	char c;
	int fd = open(filename, 0);
	for(int i=0; i<size; i++){
		read(fd, &c, 1);
		arr[i]=c-'0';
		read(fd, &c, 1);
	}	
  	close(fd);
  	// this is to supress warning
  	printf(1,"first elem %d\n", arr[0]);
  
  	//----FILL THE CODE HERE for unicast sum and multicast variance
  	
  	int parent_pid = getpid();
  	for(int i=0;i<THREADCOUNT;i++){
  		int cid = fork();
  		if(cid == 0){
  			int start_index = i * (size/THREADCOUNT);
  			int end_index = (i + 1) * (size/THREADCOUNT);
  			if (i == THREADCOUNT) end_index = size;

  			int local_sum = 0;
  			for(int j=start_index;j<end_index;j++)
  				local_sum += arr[j];
  			char m[8];
  			itoa(local_sum, m);
  			send(getpid(), parent_pid, &m);
  			exit();
		}
  	}

  	for(int i=0;i<THREADCOUNT;i++){
  		char msg[8];
  		recv(&msg);
  		tot_sum += atoi(msg);
  	}

  	int mean = tot_sum / size;

  	float temp = 0;
  	for(int i=0;i<size;i++)
  		temp += (arr[i] - mean) * (arr[i]  - mean);

  	variance = temp / size;





  	//------------------

  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename,tot_sum);
	}
	else{ //mulicast variance
		printf(1,"Variance of array for file %s is %d\n", filename,(int)variance);
	}
	exit();
}
