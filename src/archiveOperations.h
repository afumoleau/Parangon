#ifndef ARCHIVEOPERATIONS_H
#define ARCHIVEOPERATIONS_H

#include "programOptions.h"

int addFile(FILE* archiveFile, char* newFiles, unsigned int verbose, char* archiveFileName);

int deleteFile(unsigned int verbose, char* oldFiles, char* archiveFilename, FILE* archiveFile);

int extractFile(FILE* archiveFile, char* extractFile, char* newName);

int GZip(programOptions po);

int difference(FILE* archiveFile);

#endif //ARCHIVEOPERATIONS_H
