/*
 * filestat.c
 *
 *  Created on: Dec 12, 2013
 *      Author: van
 */

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf(1, "Wrong number of args");
    exit();
  }
 struct stat st;
 char * pathname;
 pathname = argv[1];
 //call stat() in user.h
 if (stat(pathname, &st) == -1){
	 printf(1, "something wrong in stat\n");
	 exit();
 }
 printf(1, "File: %s\n", pathname);
 printf(1, "Size: %d\n", st.size);
 printf(1, "Type: %d\n", st.type);
 printf(1, "Checksum: %s\n", st.checksum);

 //print everything

  exit();
}
