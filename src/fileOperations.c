#include "fileOperations.h"

#define SHIFTBUFFERSIZE 1024

void shiftData(FILE* file, off_t src, off_t dst)
{
	if(dst > src)
	{
		// Shift right
		char* buffer[SHIFTBUFFERSIZE];

		// Compute shift offset and file size
		size_t shiftOffset = dst-src;
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		fseek(file, shiftOffset, SEEK_CUR);

		// Start one buffer before the end of the file
		size_t readSize = SHIFTBUFFERSIZE < src ? SHIFTBUFFERSIZE : src;
		size_t readOffset = fileSize - readSize;
		while(readOffset >= src)
		{
			readSize = SHIFTBUFFERSIZE < readOffset-src ? SHIFTBUFFERSIZE : readOffset-src;
			fseek(file, readOffset, SEEK_SET);
			fread(&buffer, readSize, 1, file);
			fseek(file, readOffset + shiftOffset, SEEK_SET);
			fwrite(&buffer, readSize, 1, file);

			// Go one buffer to the left
			readOffset -= src;
		}
	}
	else
	{
		//TODO Shift left
	}
}
