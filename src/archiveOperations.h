#ifndef ARCHIVEOPERATIONS_H
#define ARCHIVEOPERATIONS_H

#include "programOptions.h"

void addFile(FILE* archiveFile, 
	     char* newFiles, 
	     unsigned int verbose, 
	     char* archiveFileName) ;

void deleteFile(unsigned int verbose, 
		char* oldFiles, 
		char* archiveFilename, 
		FILE* archiveFile);

void extractFile(FILE* archiveFile, char* extractFile, char* newName);

void GZip(programOptions po);

int difference(FILE* archiveFile);

#endif //ARCHIVEOPERATIONS_H
