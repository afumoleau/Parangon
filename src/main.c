#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "arguments.h"
#include "commands.h"
#include "archiveOperations.h"

void usage();

int main(int argc, char* argv[])
{
	if (argc < 2)
		usage();

	programOptions po = programOptionsCreate();

	while (readArgument(argc, argv, po) == 0)
	{
	}

	executeCommand(po);

	if(programOptionsGetGZip(po))
	  GZip(po);

	programOptionsDestroy(po);

	return EXIT_SUCCESS;
}

void usage()
{
	printf("Usage: par <operation> [options]\n");
	printf("For more help, type : par -h \n");
	exit(EXIT_FAILURE);
}
