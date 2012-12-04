#include "commands.h"
#include "programOptions.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int executeCommand(programOptions po)
{
	switch(programOptionsGetMode(po))
	{
	case MODE_ADD:
		return commandAdd(po);
		break;
	case MODE_DELETE:
		return commandDelete(po);
		break;
	case MODE_EXTRACT:
		return commandExtract(po);
		break;
	case MODE_UPDATE:
		return commandUpdate(po);
		break;
	case MODE_CREATE:
		return commandCreate(po);
		break;
	case MODE_LIST:
		return commandList(po);
		break;
	case MODE_DIFF:
		return commandDiff(po);
		break;
	case MODE_HELP:
		return commandHelp(po);
		break;
	default :
		break;
	}
	return 0;
}

int commandAdd(programOptions po)
{
	return 0;
}
int commandDelete(programOptions po)
{
	return 0;
}
int commandExtract(programOptions po)
{
	return 0;
}
int commandUpdate(programOptions po)
{
	return 0;
}

struct tarHeader
{
	char name[100];
	mode_t mode;
	unsigned long owner;
	unsigned long group;
	off_t size;
	time_t mtime;
	unsigned long checksum;
	char type;
	char linkedName[100];
};
typedef struct tarHeader tarHeader;

int commandCreate(programOptions po)
{
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "w");

	printf("Creating %s\n", archiveFilename);

	char** files = programOptionsGetFilesName(po);
	for(int i = 0; i < programOptionsGetFilesCount(po); ++i)
	{
		struct stat myStat;
		stat(files[i], &myStat);

		tarHeader header;
		strcpy(header.name, files[i]);
		header.mode = myStat.st_mode;
		header.owner = myStat.st_gid;
		header.group = myStat.st_uid;
		header.size = myStat.st_size;
		header.mtime = myStat.st_mtime;
		/*
		header.checksum = ;
		header.type = ;
		header.linkedName = ;
		*/
	}
	//fwrite(archiveFile, headers);

	fclose(archiveFile);

	return 0;
}
int commandList(programOptions po)
{
	return 0;
}
int commandDiff(programOptions po)
{
	return 0;
}
int commandHelp(programOptions po)
{
	return 0;
}
