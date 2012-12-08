#include "programOptions.h"
#include <stdlib.h>
#include <stdio.h>

struct programOptions
{
	unsigned int mode;
	char* archiveFilename;
	char** filesNames;
	unsigned int filesCount;
	unsigned int verbose;
	unsigned int gzip;
	unsigned int sparse;
};

programOptions programOptionsCreate()
{
	programOptions instance = (programOptions) malloc(sizeof(struct programOptions));
	instance->mode = MODE_NONE;
	instance->verbose = 0;
	instance->archiveFilename = NULL;
	instance->filesNames = NULL;
	instance->filesCount = 0;
	instance->gzip = 0;
	instance->sparse = 0;
	return instance;
}

void programOptionsDestroy(programOptions po)
{
	free(po);
}

unsigned int programOptionsGetMode(programOptions po)
{
	return po->mode;
}

void programOptionsSetMode(programOptions po, unsigned int mode)
{
	po->mode = mode;
}

void programOptionsAddFile(programOptions po, char* filename)
{

	++po->filesCount;
	char** tmp = (char**) malloc(sizeof(char*) * po->filesCount);

	for(int i = 0; i < po->filesCount-1; ++i)
		tmp[i] = po->filesNames[i];
	tmp[po->filesCount-1] = filename;

	po->filesNames = tmp;
}

void programOptionsSetArchiveName(programOptions po, char* archiveName)
{
	po->archiveFilename = archiveName;
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
	return po->filesNames;
}

unsigned int programOptionsGetFilesCount(programOptions po)
{
	return po->filesCount;
}

unsigned int programOptionsGetVerbose(programOptions po)
{
	return po->verbose;
}

void programOptionsSetVerbose(programOptions po, unsigned int verbose)
{
	po->verbose = verbose;
}

void programOptionsSetGZip(programOptions po, unsigned int gzip)
{
	po->gzip = gzip;
}

unsigned int programOptionsGetGZip(programOptions po)
{
	return po->gzip;
}

void programOptionsSetSparse(programOptions po, unsigned int sparse)
{
	po->sparse = sparse;
}

void multipleCommandError()
{
	fprintf(stderr, "Multiple commands found\n");
	exit(EXIT_FAILURE);
}
