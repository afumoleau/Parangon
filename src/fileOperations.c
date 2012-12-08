#include "fileOperations.h"

#define SHIFTBUFFERSIZE 1024

void shiftData(FILE* file, off_t src, off_t dst)
{
	if (dst > src)
	{
		// Shift right

		size_t shiftOffset = dst - src;
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);

		for (int i = fileSize - 1; i >= src; --i)
		{
			char tmp;
			fseek(file, i, SEEK_SET);
			fread(&tmp, sizeof(char), 1, file);
			fseek(file, i + shiftOffset, SEEK_SET);
			fwrite(&tmp, sizeof(char), 1, file);
			tmp = 0;
			fseek(file, i, SEEK_SET);
			fwrite(&tmp, sizeof(char), 1, file);
		}
	}
	else
	{
		// Shift left
		size_t shiftOffset = dst - src;
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);

		for (int i = src; i < fileSize; ++i)
		{
			char tmp;
			fseek(file, i, SEEK_SET);
			fread(&tmp, sizeof(char), 1, file);
			fseek(file, i + shiftOffset, SEEK_SET);
			fwrite(&tmp, sizeof(char), 1, file);
			tmp = 0;
			fseek(file, i, SEEK_SET);
			fwrite(&tmp, sizeof(char), 1, file);
		}
	}
}
