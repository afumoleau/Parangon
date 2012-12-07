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

#include <fcntl.h>

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

  for(int i = 0; i < newFilesCount; i++)
    addFile(archiveFile, fileCount+i, newFiles[i], programOptionsGetVerbose(po), archiveFilename);

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

  for(int i = 0; i < oldFilesCount; i++)
    deleteFile(programOptionsGetVerbose(po), oldFiles[i], archiveFilename, fileCount-i, archiveFile);

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

	      fclose(newFile);


	      struct utimbuf time;
	      time.actime = header.mtime;
	      time.modtime = header.mtime;
	      utime(header.name, &time);

	      fseek(archiveFile,sizeof(unsigned int),SEEK_SET);
	      break ;
	    }
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
  if(archiveFile == NULL)
    {
      fprintf(stderr, "No such file or directory : %s\n", archiveFilename);
      return 1;
    }

  // Verbose Log
  if(programOptionsGetVerbose(po))
    printf("Searching files to update in %s\n", archiveFilename);

  // Read file count
  unsigned int fileCount = 0;
  fread(&fileCount, sizeof(unsigned int), 1, archiveFile);

  char** changedFiles;
  changedFiles = (char**) malloc(sizeof(char*)*fileCount);
  for(int i = 0; i<fileCount; i++)
    changedFiles[i] = (char*) malloc(sizeof(char)*100);
  int cpt = 0;

  // Read headers
  fileHeader header;
  for(int i = 0; i < fileCount; ++i)
    {
      size_t readSize = fread(&header, sizeof(fileHeader), 1, archiveFile);
      if(readSize > 0)
	{
	  struct stat buf ;
	  stat(header.name,&buf);
	  if(header.mtime != buf.st_mtime){
	    if(programOptionsGetVerbose(po))
	      printf("Updating %s.\n",header.name);
	    strcpy(changedFiles[cpt],header.name);
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
  for(int i = 0; i < cpt; i++)
    {
      deleteFile(1,changedFiles[i],archiveFilename,fileCount-i,archiveFile);
      addFile(archiveFile, fileCount-1, changedFiles[i], 1, archiveFilename);
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
      header.group = myStat.st_gid;
      header.owner = myStat.st_uid;
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
    printf("Serching differences between the files in %s and the files on the disk.\n", archiveFilename);

  // Read file count
  unsigned int fileCount = 0;
  fread(&fileCount, sizeof(unsigned int), 1, archiveFile);

  // Read headers
  fileHeader header;
  for(int i = 0; i < fileCount; ++i)
    {
      size_t readSize = fread(&header, sizeof(fileHeader), 1, archiveFile);
      if(readSize > 0)
	{
	  struct stat buf ;
	  stat(header.name,&buf);
	  if(header.mtime != buf.st_mtime)
	    printf("%s is different on the disk.\n",header.name);
	}
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

void addFile(FILE* archiveFile, int fileCount, char* newFile, unsigned int verbose, char* archiveFilename){

  fseek(archiveFile,sizeof(unsigned int),SEEK_SET);

  // Shift data to reserve some place for the new headers
  shiftData(archiveFile, sizeof(unsigned int) + fileCount * sizeof(fileHeader), sizeof(unsigned int) + (fileCount+1) * sizeof(fileHeader));
  fseek(archiveFile, 0, SEEK_SET);

  // Update data offset for each header
  size_t dataOffset = sizeof(unsigned int) + (fileCount+1) * sizeof(fileHeader);
  fseek(archiveFile, sizeof(unsigned int), SEEK_SET);
  for(int i = 0; i < fileCount; ++i)
    {
      fileHeader header;
      if(fread(&header, sizeof(fileHeader), 1, archiveFile) <= 0)
	fprintf(stderr, "Error reading");

      header.data = dataOffset;
      dataOffset += header.size;

      fseek(archiveFile, -sizeof(fileHeader), SEEK_CUR);

      fwrite(&header, sizeof(fileHeader), 1, archiveFile);
    }

  size_t newFilesDataOffset = dataOffset;

  // Write new files headers

  // Verbose Log
  if(verbose)
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
  fwrite(&header, sizeof(fileHeader), 1, archiveFile);

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
  while(leftToRead > 0)
    {
      unsigned int readSize = leftToRead < COPYBUFFERSIZE ? leftToRead : COPYBUFFERSIZE;
      fread(&copyBuffer, readSize, 1, file);
      fwrite(&copyBuffer, readSize, 1, archiveFile);
      leftToRead -= readSize;
    }

  // Close file
  fclose(file);

  // Update file count
  fseek(archiveFile, 0, SEEK_SET);
  unsigned int totalFileCount = fileCount + 1;
  fwrite(&totalFileCount, sizeof(unsigned int), 1, archiveFile);
}

void deleteFile(unsigned int verbose, char* oldFiles, char* archiveFilename, int fileCount, FILE* archiveFile){
  fseek(archiveFile,sizeof(unsigned int),SEEK_SET);
  // Verbose Log
  if(verbose)
    printf("Deleting file %s to archive %s\n", oldFiles, archiveFilename);

  for(int j = 0; j < fileCount; j++)
    {
      size_t headerPos = ftell(archiveFile);
      fileHeader header ;
      fread(&header, sizeof(fileHeader), 1, archiveFile);
      if(strcmp(header.name,oldFiles) == 0)
	{ 
	  size_t dataOffset = header.data;
	  size_t dataSize = header.size;

	  fseek(archiveFile, sizeof(unsigned int), SEEK_SET);

	  // Update data offset for each header
	  fileHeader header;
	  for(int k = 0; k < fileCount; k++)
	    {
	      fread(&header, sizeof(fileHeader), 1, archiveFile);
	      header.data -= sizeof(fileHeader) ;
	      if(k>j)
		header.data -= dataSize ;
	      fseek(archiveFile, -sizeof(fileHeader), SEEK_CUR);
	      fwrite(&header, sizeof(fileHeader), 1, archiveFile);
	    }

	  // Shift data to erase the file and its header
	  shiftData(archiveFile, dataOffset + dataSize, dataOffset);
	  shiftData(archiveFile, headerPos + sizeof(fileHeader), headerPos);

	  fseek(archiveFile,headerPos,SEEK_SET);
	  break ;
	}
    }

  // Update file count
  fseek(archiveFile, 0, SEEK_SET);
  int totalFileCount = fileCount-1; 
  fwrite(&totalFileCount, sizeof(unsigned int), 1, archiveFile);
}

void GZip(programOptions po){
  if(fork() == 0)
    {
      execlp("gzip","gzip",programOptionsGetArchiveName(po),NULL);
      perror("Compression failed.\n");
    }	    
  else
    {
      if(programOptionsGetVerbose(po))
	printf("Compressing.\n");

      int status;
      waitpid(-1,&status,0);

      if(!WIFEXITED(status))
	perror("Compression failed.\n");
      else if(programOptionsGetVerbose(po))
	printf("%s","Compression done.\n");

    }
}
