#include "commands.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <utime.h>
#include <sys/wait.h>

#include "programOptions.h"
#include "fileHeader.h"
#include "fileOperations.h"
#include "archiveOperations.h"

#include <fcntl.h>

int executeCommand(programOptions po)
{
	switch (programOptionsGetMode(po))
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
	default:
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
	if (archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Get file counts
	char** newFiles = programOptionsGetFilesName(po);
	unsigned int newFilesCount = programOptionsGetFilesCount(po);

	for (int i = 0; i < newFilesCount; i++)
		if (addFile(archiveFile, newFiles[i], programOptionsGetVerbose(po), archiveFilename) != 0)
		{
			fprintf(stderr, "Error adding %s.\n", newFiles[i]);
			return 1;
		}

	// Close archive file
	fflush(archiveFile);
	fclose(archiveFile);
	return 0;
}

int commandDelete(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r+");

	// Check file existence
	if (archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Get file counts
	char** oldFiles = programOptionsGetFilesName(po);
	unsigned int oldFilesCount = programOptionsGetFilesCount(po);
	unsigned int fileCount = 0;

	if (fread(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		printf("Error reading\n");
		return 1;
	}

	for (int i = 0; i < oldFilesCount; i++)
		if (deleteFile(programOptionsGetVerbose(po), oldFiles[i], archiveFilename, archiveFile) != 0)
		{
			fprintf(stderr, "Error deleting %s", oldFiles[i]);
			return 1;
		}

	// Close archive file
	fflush(archiveFile);
	fclose(archiveFile);

	return 0;
}

int commandExtract(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r+");

	// Check file existence
	if (archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Get file counts
	char** extractFiles = programOptionsGetFilesName(po);
	unsigned int extractFilesCount = programOptionsGetFilesCount(po);

	if (extractFilesCount == 0)
	{
		// Read file count
		unsigned int fileCount = 0;
		if (fread(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
		{
			perror("Error reading.\n");
			return 1;
		}

		// Read headers
		fileHeader header;
		for (int i = 0; i < fileCount; ++i)
		{
			size_t readSize = fread(&header, sizeof(fileHeader), 1, archiveFile);
			if (readSize > 0)
			{
				// Verbose Log
				if (programOptionsGetVerbose(po))
					printf("Extracting file %s from archive %s\n", header.name, archiveFilename);
				if (extractFile(archiveFile, header.name, header.name) != 0)
				{
					fprintf(stderr, "Error extracting %s.\n", header.name);
					return 1;
				}
			}
			else
			{
				// Close archive file
				fclose(archiveFile);
				return 1;
			}
		}
	}
	for (int i = 0; i < extractFilesCount; i++)
	{
		// Verbose Log
		if (programOptionsGetVerbose(po))
			printf("Extracting file %s from archive %s\n", extractFiles[i], archiveFilename);
		if (extractFile(archiveFile, extractFiles[i], extractFiles[i]) != 0)
		{
			fprintf(stderr, "Error extracting %s.\n", extractFiles[i]);
			return 1;
		}
	}

	// Close archive file
	fflush(archiveFile);
	fclose(archiveFile);
	return 0;
}

int commandUpdate(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r+");

	// Check file existence
	if (archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Verbose Log
	if (programOptionsGetVerbose(po))
		printf("Searching files to update in %s\n", archiveFilename);

	// Read file count
	unsigned int fileCount = 0;

	if (fread(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		printf("Error reading.\n");
		return 1;
	}

	char** changedFiles;
	changedFiles = (char**) malloc(sizeof(char*) * fileCount);
	for (int i = 0; i < fileCount; i++)
		changedFiles[i] = (char*) malloc(sizeof(char) * 100);
	int cpt = 0;

	// Read headers
	fileHeader header;
	for (int i = 0; i < fileCount; ++i)
	{
		size_t readSize = fread(&header, sizeof(fileHeader), 1, archiveFile);
		if (readSize > 0)
		{
			struct stat buf;
			stat(header.name, &buf);
			if (header.mtime != buf.st_mtime)
			{
				if (programOptionsGetVerbose(po))
					printf("Updating %s.\n", header.name);
				strcpy(changedFiles[cpt], header.name);
				cpt++;
			}
		}
		else
		{
			// Close archive file
			fflush(archiveFile);
			fclose(archiveFile);
			return 1;
		}
	}
	for (int i = 0; i < cpt; i++)
	{
		if ((deleteFile(programOptionsGetVerbose(po), changedFiles[i], archiveFilename, archiveFile) != 0) || (addFile(archiveFile, changedFiles[i], programOptionsGetVerbose(po), archiveFilename) != 0))
		{
			fprintf(stderr, "Error updating %s", changedFiles[i]);
			return 1;
		}
	}

	// Close archive file
	fflush(archiveFile);
	fclose(archiveFile);
	return 0;
}

int commandCreate(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "w+");

	// Verbose Log
	if (programOptionsGetVerbose(po))
		printf("Creating archive %s\n", archiveFilename);

	// Write file count
	unsigned int fileCount = 0;
	if (fwrite(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		perror("Error writing.\n");
		return 1;
	}

	// Get files to add to the archive
	char** files = programOptionsGetFilesName(po);
	fileCount = programOptionsGetFilesCount(po);

	for (int i = 0; i < fileCount; i++)
		if (addFile(archiveFile, files[i], programOptionsGetVerbose(po), archiveFilename) != 0)
		{
			fprintf(stderr, "Error adding %s to %s.\n", files[i], archiveFilename);
			return 1;
		}

	// Close archive file
	fflush(archiveFile);
	fclose(archiveFile);

	return 0;
}

int commandList(programOptions po)
{
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r");

	// Check file existence
	if (archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Verbose Log
	if (programOptionsGetVerbose(po))
		printf("Listing files in %s\n", archiveFilename);

	// Read file count
	unsigned int fileCount = 0;
	if (fread(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		perror("Error reading.\n");
		return 1;
	}

	// Read headers
	fileHeader header;
	for (int i = 0; i < fileCount; ++i)
	{
		size_t readSize = fread(&header, sizeof(fileHeader), 1, archiveFile);
		if (readSize > 0)
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
	// Open archive file
	char* archiveFilename = programOptionsGetArchiveName(po);
	FILE* archiveFile = fopen(archiveFilename, "r");

	// Check file existence
	if (archiveFile == NULL)
	{
		fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
		return 1;
	}

	// Verbose Log
	if (programOptionsGetVerbose(po))
		printf("Serching differences between the files in %s and the files on the disk.\n", archiveFilename);

	if (difference(archiveFile) != 0)
		return 1;

	// Close archive file
	fclose(archiveFile);
	return 0;
}

int commandHelp(programOptions po)
{
	printf("Usage: \npar <operation> [options]\n");
	printf("\n");
	printf("Operations :\n");
	printf("-h \t display the help\n\n");
	printf("-t \t list the contents from an archive\n\n");
	printf("-r <files | directories> \t append files (or directories) to the end of an archive\n\n");
	printf("-c <files | directories> \t create an archive from files (and directories)\n\n");
	printf("-u \t only append files that are newer than the existing in archive\n\n");
	printf("-x \t extract files from an archive\n\n");
	printf("-d <file> \t delete file from the archive (not for use on magnetic tapes !)\n\n");
	printf("-sparse \t \n\n");
	printf("-m \t find differences between archive and file system\n");
	printf("\n");
	printf("Options :\n");
	printf("-f <file> \t use archive file\n\n");
	printf("-v \t verbosely list files processed\n\n");
	printf("-z \t filter the archive through gzip\n\n");

	return 0;

}
