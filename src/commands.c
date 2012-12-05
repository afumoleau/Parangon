#include "commands.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "programOptions.h"
#include "fileHeader.h"
#include "fileOperations.h"

#define COPYBUFFERSIZE 1024

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
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r+");

	// Check file existence
	if(archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Get file counts
	char** newFiles = programOptionsGetFilesName(po);
	unsigned int newFilesCount = programOptionsGetFilesCount(po);
	unsigned int fileCount = 0;
	fread(&fileCount, sizeof(unsigned int), 1, archiveFile);
	unsigned int totalFileCount = fileCount + newFilesCount;

	if( fileCount != 0 )
	// Shift data to reserve some place for the new headers
		shiftData(archiveFile, sizeof(unsigned int) + fileCount * sizeof(fileHeader), sizeof(unsigned int) + totalFileCount * sizeof(fileHeader));
	fseek(archiveFile, 0, SEEK_SET);

	// Update data offset for each header
	size_t dataOffset = sizeof(unsigned int) + totalFileCount * sizeof(fileHeader);
	fseek(archiveFile, sizeof(unsigned int), SEEK_SET);
	for(int i = 0; i < fileCount; ++i)
	{
		fileHeader header;
		printf("reading at %d\n", ftell(archiveFile));
		if(fread(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
			fprintf(stderr, "Error reading");

		header.data = dataOffset;
		dataOffset += header.size;

		printf("Updating %s\n", header.name);

		printf("seeking at %d\n", ftell(archiveFile));
		fseek(archiveFile, -sizeof(fileHeader), SEEK_CUR);


		printf("writing at %d\n", ftell(archiveFile));
		fwrite(&header, sizeof(fileHeader), 1, archiveFile);
		printf("end at %d\n", ftell(archiveFile));
	}

	size_t newFilesDataOffset = dataOffset;

	// Write new files headers
	for(int i = 0; i < newFilesCount; ++i)
	{
		char* file = newFiles[i];

		// Verbose Log
		if(programOptionsGetVerbose(po))
			printf("Adding file %s to archive %s\n", file, archiveFilename);

		// Get file data
		struct stat myStat;
		stat(file, &myStat);

		// Create header and fill it with data
		fileHeader header;
		strcpy(header.name, file);
		header.mode = myStat.st_mode;
		header.owner = myStat.st_gid;
		header.group = myStat.st_uid;
		header.size = myStat.st_size;
		header.data = dataOffset;
		header.mtime = myStat.st_mtime;

		// Write header
		fwrite(&header, sizeof(fileHeader), 1, archiveFile);

		// Add file size to the data offset
		dataOffset += header.size;
	}

	// Write new files data
	fseek(archiveFile, newFilesDataOffset, SEEK_SET);

	for(int i = 0; i < newFilesCount; ++i)
	{
		// Open file
		FILE* file = fopen(newFiles[i], "r");

		// Get file size
		fseek(file, 0, SEEK_END);
		long int leftToRead = ftell(file);
		fseek(file, 0, SEEK_SET);

		// Copy loop
		char copyBuffer[COPYBUFFERSIZE];
		while(leftToRead > 0)
		{
			unsigned int readSize = leftToRead < COPYBUFFERSIZE ? leftToRead : COPYBUFFERSIZE;
			fread(&copyBuffer, readSize, 1, file);
			fwrite(&copyBuffer, readSize, 1, archiveFile);
			leftToRead -= readSize;
		}

		// Close file
		fclose(file);
	}

	// Update file count
	fseek(archiveFile, 0, SEEK_SET);
	fwrite(&totalFileCount, sizeof(unsigned int), 1, archiveFile);

	// Close archive file
	fclose(archiveFile);
	return 0;
}

int commandDelete(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r+");

	// Check file existence
	if(archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Get file counts
	char** oldFiles = programOptionsGetFilesName(po);
	unsigned int oldFilesCount = programOptionsGetFilesCount(po);
	unsigned int fileCount = 0;
	fread(&fileCount, sizeof(unsigned int), 1, archiveFile);

	unsigned int totalFileCount = fileCount;

	for(int i = 0; i < oldFilesCount; i++)
	{
		// Verbose Log
		if(programOptionsGetVerbose(po))
			printf("Deleting file %s to archive %s\n", oldFiles[i], archiveFilename);

		for(int j = 0; j < fileCount; j++)
		{
			size_t headerPos = ftell(archiveFile);
			fileHeader header ;
			fread(&header, sizeof(fileHeader), 1, archiveFile);

			if(strcmp(header.name,oldFiles[i]) == 0)
			{
				size_t dataOffset = header.data;
				size_t dataSize = header.size;

				// Shift data to erase the file and its header
				shiftData(archiveFile, dataOffset + dataSize, dataOffset);
				shiftData(archiveFile, ftell(archiveFile), headerPos);
				fseek(archiveFile, headerPos, SEEK_SET);

				// Update data offset for each header
				fileHeader header;
				for(int k = j+1; k < fileCount; k++)
				{
					fread(&header, sizeof(fileHeader), 1, archiveFile);
					header.data -= dataSize ;
					fseek(archiveFile, -sizeof(fileHeader), SEEK_CUR);
					fwrite(&header, sizeof(fileHeader), 1, archiveFile);
				}


				totalFileCount--;
				break ;
			}
		}
	}

	// Update file count
	fseek(archiveFile, 0, SEEK_SET);
	fwrite(&totalFileCount, sizeof(unsigned int), 1, archiveFile);

	// Close archive file
	fclose(archiveFile);
	return 0;

	return 0;
}

int commandExtract(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r+");

	// Check file existence
	if(archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Get file counts
	char** extractFiles = programOptionsGetFilesName(po);
	unsigned int extractFilesCount = programOptionsGetFilesCount(po);
	unsigned int fileCount = 0;
	fread(&fileCount, sizeof(unsigned int), 1, archiveFile);

	for(int i = 0; i < extractFilesCount; i++)
	{
		// Verbose Log
		if(programOptionsGetVerbose(po))
			printf("Extracting file %s to archive %s\n", extractFiles[i], archiveFilename);

		for(int j = 0; j < fileCount; j++)
		{
			fileHeader header ;
			fread(&header, sizeof(fileHeader), 1, archiveFile);
			size_t headerPos = ftell(archiveFile);

			if(strcmp(header.name,extractFiles[i]) == 0)
			{
				size_t dataOffset = header.data;
				size_t dataSize = header.size;
				char buffer[dataSize];

				fseek(archiveFile, dataOffset, SEEK_SET);
				fread(&buffer,dataSize,1,archiveFile);
				fseek(archiveFile, headerPos, SEEK_SET);

				int svguid = geteuid() ;
				seteuid(header.owner);

				int svggid = getegid();
				setegid(header.group);

				FILE* newFile = fopen(header.name, "w+");
				fseek(newFile,0,SEEK_SET);
				fwrite(&buffer,dataSize,1,newFile);
				chmod(header.name, header.mode);

				seteuid(svguid);
				setegid(svggid);

				//TO DO change date of last modification.

				fclose(newFile);

				break ;
			}
		}
	}

	// Close archive file
	fclose(archiveFile);
	return 0;
}

int commandUpdate(programOptions po)
{
	return 0;
}

int commandCreate(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "w");

	// Verbose Log
	if(programOptionsGetVerbose(po))
		printf("Creating archive %s\n", archiveFilename);

	// Write file count
	unsigned int fileCount = programOptionsGetFilesCount(po);
	fwrite(&fileCount, sizeof(unsigned int), 1, archiveFile);

	// Compute initial data offset
	size_t dataOffset = sizeof(unsigned int) + fileCount * sizeof(fileHeader);

	// Get files to add to the archive
	char** files = programOptionsGetFilesName(po);

	// Write headers
	for(int i = 0; i < fileCount; ++i)
	{
		// Get file data
		struct stat myStat;
		stat(files[i], &myStat);

		// Create header and fill it with data
		fileHeader header;
		strcpy(header.name, files[i]);
		header.mode = myStat.st_mode;
		header.owner = myStat.st_gid;
		header.group = myStat.st_uid;
		header.size = myStat.st_size;
		header.data = dataOffset;
		header.mtime = myStat.st_mtime;

		// Write header
		fwrite(&header, sizeof(fileHeader), 1, archiveFile);

		// Add file size to the data offset
		dataOffset += header.size;
	}

	// Write data
	for(int i = 0; i < fileCount; ++i)
	{
		// Open file
		FILE* file = fopen(files[i], "r");

		// Get file size
		fseek(file, 0, SEEK_END);
		long int leftToRead = ftell(file);
		fseek(file, 0, SEEK_SET);

		// Copy loop
		char copyBuffer[COPYBUFFERSIZE];
		while(leftToRead > 0)
		{
			unsigned int readSize = leftToRead < COPYBUFFERSIZE ? leftToRead : COPYBUFFERSIZE;
			fread(&copyBuffer, readSize, 1, file);
			fwrite(&copyBuffer, readSize, 1, archiveFile);
			leftToRead -= readSize;
		}

		// Close file
		fclose(file);
	}

	// Close archive file
	fclose(archiveFile);

	return 0;
}

int commandList(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r");

	// Check file existence
	if(archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Verbose Log
	if(programOptionsGetVerbose(po))
		printf("Listing files in %s\n", archiveFilename);

	// Read file count
	unsigned int fileCount = 0;
	fread(&fileCount, sizeof(unsigned int), 1, archiveFile);

	// Read headers
	fileHeader header;
	for(int i = 0; i < fileCount; ++i)
	{
		size_t readSize = fread(&header, sizeof(fileHeader), 1, archiveFile);
		if(readSize > 0)
			printf("%s\n", header.name);
		else
		{
			// Close archive file
			fclose(archiveFile);
			return 1;
		}
	}

	// Close archive file
	fclose(archiveFile);
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
