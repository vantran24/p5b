#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
	//int fd = open("foo", O_CREATE|O_CHECKED);
	//printf(1, "%d\n", fd);
	uint a = 0xff000000;

	a = a & 0xd4123456;
	a = a >> 24;// can get the checksum this way
	printf(1, "%x\n", a);

	exit();
}
