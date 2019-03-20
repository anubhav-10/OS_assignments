#include "types.h"
#include "user.h"
#include "date.h"

int
main(int argc, char **argv)
{
	if(argc != 3){
		printf(2, "usage: add arg1 arg2...\n");
		exit();
	}
	add(atoi(argv[1]), atoi(argv[2]));
	// for(int i=1;i<argc;i++)
	// 	add(atoi(argv[i]));
	exit();
}