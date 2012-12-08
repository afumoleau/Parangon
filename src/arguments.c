#include "arguments.h"
#include <string.h>
#include <stdio.h>

int readArgument(int argc, char* argv[], programOptions po)
{
	static int argumentIndex = 1;

	if (argumentIndex >= argc)
		return 1;

	char* argument = argv[argumentIndex];

	int j = 0;
	if (argument[j] == '-')
	{
		j++;
		do
		{
			char option = argument[j++];

			switch (option)
			{
			case 'h':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_HELP);
				break;

			case 'v':
				programOptionsSetVerbose(po, 1);
				break;

			case 'c':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_CREATE);
				break;

			case 't':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_LIST);
				break;

			case 'r':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_ADD);
				break;

			case 'u':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_UPDATE);
				break;

			case 'x':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_EXTRACT);
				break;

			case 'f':
				programOptionsSetArchiveName(po, argv[++argumentIndex]);
				break;

			case 'z':
				programOptionsSetGZip(po, 1);
				break;

			case 'd':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_DELETE);
				break;

			case 'm':
				if (programOptionsGetMode(po) != MODE_NONE)
					multipleCommandError();
				programOptionsSetMode(po, MODE_DIFF);
				break;

			case 's':
				if (strcmp(argument, "-sparse"))
					programOptionsSetSparse(po, 1);
				break;

			default:
				break;
			}

		} while (argument[j] != '\0');
	}
	else
	{
		programOptionsAddFile(po, argument);
	}

	return (++argumentIndex < argc) ? 0 : argc;
}
