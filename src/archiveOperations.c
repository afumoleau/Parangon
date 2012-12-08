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

#include "archiveOperations.h"
#include "fileOperations.h"
#include "fileHeader.h"

#define COPYBUFFERSIZE 1024

int addFile(FILE* archiveFile, char* newFile, unsigned int verbose, char* archiveFilename)
{

	fseek(archiveFile, 0, SEEK_SET);

	unsigned int fileCount = 0;
	fread(&fileCount, sizeof(unsigned int), 1, archiveFile);

	// Shift data to reserve some place for the new headers
	shiftData(archiveFile, sizeof(unsigned int) + fileCount * sizeof(fileHeader), sizeof(unsigned int) + (fileCount + 1) * sizeof(fileHeader));
	fseek(archiveFile, 0, SEEK_SET);

	// Update data offset for each header
	size_t dataOffset = sizeof(unsigned int) + (fileCount + 1) * sizeof(fileHeader);
	fseek(archiveFile, sizeof(unsigned int), SEEK_SET);
	for (int i = 0; i < fileCount; ++i)
	{
		fileHeader header;
		if (fread(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
		{
			fprintf(stderr, "Error reading\n");
			return 1;
		}

		header.data = dataOffset;
		dataOffset += header.size;

		fseek(archiveFile, -sizeof(fileHeader), SEEK_CUR);

		if (fwrite(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
		{
			fprintf(stderr, "Error writing\n");
			return 1;
		}
	}

	size_t newFilesDataOffset = dataOffset;

	// Write new files headers

	// Verbose Log
	if (verbose)
		printf("Adding file %s to archive %s\n", newFile, archiveFilename);

	// Get file data
	struct stat myStat;
	stat(newFile, &myStat);

	// Create header and fill it with data
	fileHeader header;
	strcpy(header.name, newFile);
	header.mode = myStat.st_mode;
	header.group = myStat.st_gid;
	header.owner = myStat.st_uid;
	header.size = myStat.st_size;
	header.data = dataOffset;
	header.mtime = myStat.st_mtime;

	// Write header
	if (fwrite(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
	{
		fprintf(stderr, "Error writing\n");
		return 1;
	}

	// Add file size to the data offset
	dataOffset += header.size;

	// Write new files data
	fseek(archiveFile, newFilesDataOffset, SEEK_SET);

	// Open file
	FILE* file = fopen(newFile, "r");

	// Get file size
	fseek(file, 0, SEEK_END);
	long int leftToRead = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Copy loop
	char copyBuffer[COPYBUFFERSIZE];
	while (leftToRead > 0)
	{
		unsigned int readSize = leftToRead < COPYBUFFERSIZE ? leftToRead : COPYBUFFERSIZE;
		if (fread(&copyBuffer, readSize, 1, file) <= 0)
		{
			fprintf(stderr, "Error reading\n");
			return 1;
		}
		if (fwrite(&copyBuffer, readSize, 1, archiveFile) <= 0)
		{
			fprintf(stderr, "Error writing\n");
			return 1;
		}
		leftToRead -= readSize;
	}

	// Close file
	fclose(file);

	// Update file count
	fseek(archiveFile, 0, SEEK_SET);
	unsigned int totalFileCount = fileCount + 1;
	if (fwrite(&totalFileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		fprintf(stderr, "Error writing\n");
		return 1;
	}

	return 0;

}

int deleteFile(unsigned int verbose, char* oldFiles, char* archiveFilename, FILE* archiveFile)
{

	fseek(archiveFile, 0, SEEK_SET);

	unsigned int fileCount = 0;
	if (fread(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		fprintf(stderr, "Error reading\n");
		return 1;
	}

	// Verbose Log
	if (verbose)
		printf("Deleting file %s to archive %s\n", oldFiles, archiveFilename);

	for (int j = 0; j < fileCount; j++)
	{
		size_t headerPos = ftell(archiveFile);
		fileHeader header;

		if (fread(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
		{
			fprintf(stderr, "Error reading\n");
			return 1;
		}

		if (strcmp(header.name, oldFiles) == 0)
		{
			size_t dataOffset = header.data;
			size_t dataSize = header.size;

			fseek(archiveFile, sizeof(unsigned int), SEEK_SET);

			// Update data offset for each header
			fileHeader header;
			for (int k = 0; k < fileCount; k++)
			{
				if (fread(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
				{
					fprintf(stderr, "Error reading\n");
					return 1;
				}

				header.data -= sizeof(fileHeader);
				if (k > j)
					header.data -= dataSize;
				fseek(archiveFile, -sizeof(fileHeader), SEEK_CUR);

				if (fwrite(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
				{
					fprintf(stderr, "Error writing\n");
					return 1;
				}
			}

			// Shift data to erase the file and its header
			shiftData(archiveFile, dataOffset + dataSize, dataOffset);
			shiftData(archiveFile, headerPos + sizeof(fileHeader), headerPos);

			fseek(archiveFile, headerPos, SEEK_SET);
			break;
		}
	}

	// Update file count
	fseek(archiveFile, 0, SEEK_SET);
	int totalFileCount = fileCount - 1;
	if (fwrite(&totalFileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		fprintf(stderr, "Error writing\n");
		return 1;
	}

	return 0;
}

int GZip(programOptions po)
{
	if (fork() == 0)
	{
		execlp("gzip", "gzip", programOptionsGetArchiveName(po), NULL);
		perror("Compression failed.\n");
		return 1;
	}
	else
	{
		if (programOptionsGetVerbose(po))
			printf("Compressing.\n");

		int status;
		waitpid(-1, &status, 0);

		if (!WIFEXITED(status))
		{
			perror("Compression failed.\n");
			return 1;
		}
		else if (programOptionsGetVerbose(po))
			printf("%s", "Compression done.\n");
	}

	return 0;
}

int extractFile(FILE* archiveFile, char* extractFile, char* newName)
{
	fseek(archiveFile, 0, SEEK_SET);

	unsigned int fileCount = 0;
	if (fread(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		fprintf(stderr, "Error reading\n");
		return 1;
	}

	for (int j = 0; j < fileCount; j++)
	{
		fileHeader header;

		if (fread(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
		{
			fprintf(stderr, "Error reading\n");
			return 1;
		}

		size_t headerPos = ftell(archiveFile);

		if (strcmp(header.name, extractFile) == 0)
		{
			size_t dataOffset = header.data;
			size_t dataSize = header.size;
			char buffer[dataSize];

			fseek(archiveFile, dataOffset, SEEK_SET);

			if (fread(&buffer, dataSize, 1, archiveFile) <= 0)
			{
				fprintf(stderr, "Error reading\n");
				return 1;
			}

			fseek(archiveFile, headerPos, SEEK_SET);

			int svguid = geteuid();
			seteuid(header.owner);

			int svggid = getegid();
			setegid(header.group);

			FILE* newFile = fopen(newName, "w+");
			fseek(newFile, 0, SEEK_SET);

			if (fwrite(&buffer, dataSize, 1, newFile) <= 0)
			{
				fprintf(stderr, "Error writing\n");
				return 1;
			}

			chmod(header.name, header.mode);

			seteuid(svguid);
			setegid(svggid);

			fclose(newFile);

			struct utimbuf time;
			time.actime = header.mtime;
			time.modtime = header.mtime;
			utime(header.name, &time);

			fseek(archiveFile, sizeof(unsigned int), SEEK_SET);
			break;
		}
	}
	return 0;
}

int difference(FILE* archiveFile)
{

	// Read file count
	unsigned int fileCount = 0;
	if (fread(&fileCount, sizeof(unsigned int), 1, archiveFile) <= 0)
	{
		fprintf(stderr, "Error writing\n");
		return 1;
	}

	fileHeader header;
	for (int i = 0; i < fileCount; ++i)
	{
		size_t readSize = fread(&header, sizeof(fileHeader), 1, archiveFile);
		size_t headerPos = ftell(archiveFile);
		if (readSize > 0)
		{
			struct stat buf;
			stat(header.name, &buf);

			struct utimbuf time;
			time.actime = buf.st_atime;
			time.modtime = buf.st_mtime;
			if (header.mtime != buf.st_mtime) // show difference only if the last modification time are different
			{
				printf("%s is different on the disk :\n", header.name);
				extractFile(archiveFile, header.name, "/tmp/tmpDiff");
				if (fork() == 0)
				{
					execlp("diff", "diff", "/tmp/tmpDiff", header.name, NULL);
					printf("An error occured during diff.\n");
				}
				else
				{
					int status;
					waitpid(-1, &status, 0);
					if (!WIFEXITED(status))
						perror("Difference failed.\n");

					if (fork() == 0)
					{
						execlp("rm", "rm", "/tmp/tmpDiff", NULL);
						return 1;
					}
					else
					{
						int status;
						waitpid(-1, &status, 0);
						if (!WIFEXITED(status))
							perror("Difference failed.\n");

						utime(header.name, &time); // restore the last access time
					}

					fseek(archiveFile, headerPos, SEEK_SET);
				}
			}
		}
		else
		{
			// Close archive file
			fclose(archiveFile);
			return 1;
		}
	}

	return 0;

}
