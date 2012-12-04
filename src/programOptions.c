#include "programOptions.h"
#include <stdlib.h>
#include <stdio.h>

struct programOptions
{
	int verbose;
	int mode;
	char* archiveFilename;
	char** filesName;
	int gzip;
	int sparse;
};

programOptions programOptionsCreate()
{
	programOptions instance = (programOptions) malloc(sizeof(struct programOptions));
	instance->mode = MODE_NONE;
	instance->verbose = 0;
	instance->archiveFilename = NULL;
	instance->gzip = 0;
	instance->sparse = 0;
	return instance;
}

void programOptionsDestroy(programOptions po)
{
	if(po->archiveFilename != NULL)
		free(po->archiveFilename);
	free(po);
}

int programOptionsGetMode(programOptions po)
{
	return po->mode;
}

void programOptionsSetMode(programOptions po, int mode)
{
	po->mode = mode;
}

void multipleCommandError()
{
	fprintf(stderr, "Multiple commands found\n");
	exit(EXIT_FAILURE);
}

char* programOptionsGetArchiveName(programOptions po)
{
	if(po->archiveFilename == NULL)
	{
		po->archiveFilename = (char*) malloc(100*sizeof(char));
		scanf("%s", po->archiveFilename);
	}
	return po->archiveFilename;
}

char** programOptionsGetFilesName(programOptions po)
{
	return po->filesName;
}

void programOptionsSetVerbose(programOptions po, int verbose)
{
	po->verbose = verbose;
}

void programOptionsSetGZip(programOptions po, int gzip)
{
	po->gzip = gzip;
}

void programOptionsSetSparse(programOptions po, int sparse)
{
	po->sparse = sparse;
}
