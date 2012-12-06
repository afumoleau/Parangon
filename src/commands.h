#ifndef COMMANDS_H
#define COMMANDS_H

#include "programOptions.h"
#include <stdio.h>

int executeCommand(programOptions po);

int commandAdd(programOptions po);
void addFile(FILE* archiveFile, 
	     int fileCount, 
	     char* newFiles, 
	     unsigned int verbose, 
	     char* archiveFileName) ;

int commandDelete(programOptions po);
void deleteFile(unsigned int verbose, 
		char* oldFiles, 
		char* archiveFilename, 
		int fileCount, 
		FILE* archiveFile);

int commandExtract(programOptions po);

int commandUpdate(programOptions po);

int commandCreate(programOptions po);

int commandList(programOptions po);

int commandDiff(programOptions po);

int commandHelp(programOptions po);

#endif // COMMANDS_H
