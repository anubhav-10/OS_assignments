#include "types.h"
#include "user.h"
#include "date.h"

int
main(int argc ,char **argv)
{
	if(argc != 3){
		printf(3, "usage: add arg1 arg2...\n");
	}
	add(atoi(argv[1]), atoi(argv[2]));
	exit();
}