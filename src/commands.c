#include "commands.h"
#include "programOptions.h"
#include <stdlib.h>
#include <stdio.h>

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
int commandCreate(programOptions po)
{
	char* archiveFilename = programOptionsGetArchiveName(po);
	if(archiveFilename == NULL)
	{
		archiveFilename = (char*) malloc(100*sizeof(char));
		scanf("%s", archiveFilename);
	}

	FILE* archiveFile = fopen(archiveFilename, "w");


	//TODO
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
