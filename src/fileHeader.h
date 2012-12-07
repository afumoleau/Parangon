#ifndef FILEHEADER_H
#define FILEHEADER_H

struct fileHeader
{
	char	name[100];
	mode_t	mode;
	char	owner;
	char	group;
	size_t	size;
	size_t	data;
	time_t	mtime;
};
typedef struct fileHeader fileHeader;

#define ISSPARSE 1
#define ISNOTSPARSE 0

#endif // FILEHEADER_H
