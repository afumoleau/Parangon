#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "arguments.h"
#include "commands.h"

void usage();

int main(int argc, char* argv[])
{
	if (argc < 2)
		usage();

	programOptions po = programOptionsCreate();

	while (readParam(argc, argv, po) == 0)
	{
	}

	printf("%d\n", programOptionsGetMode(po));

	executeCommand(po);

	programOptionsDestroy(po);

	/*
	 char* dirName = (argc < 2) ? "." : argv[1];

	 struct stat myStat;
	 stat(dirName, &myStat);

	 if(!S_ISDIR(myStat.st_mode))
	 EXIT_FAILURE;

	 DIR* dir = opendir(dirName);
	 struct dirent** namelist;
	 struct dirent* entry = readdir(dir);

	 while(entry != NULL)
	 {
	 printf("%s\n", (char*) entry->d_name);
	 entry = readdir(dir);
	 }

	 struct dirent** namelist;

	 int fileCount = scandir(dirName, &namelist, NULL, alphasort);

	 unsigned int i = 0;
	 for(; i < fileCount; ++i)
	 {
	 struct dirent* file = namelist[i];
	 printf("%s\n", (char*) file->d_name);
	 }
	 */

	return EXIT_SUCCESS;
}

void usage()
{
	printf("Usage : par\n");
	exit(EXIT_FAILURE);
}
