#ifndef COMMANDS_H
#define COMMANDS_H

#include "programOptions.h"
#include <stdio.h>

int executeCommand(programOptions po);

int commandAdd(programOptions po);

int commandDelete(programOptions po);

int commandExtract(programOptions po);

int commandUpdate(programOptions po);

int commandCreate(programOptions po);

int commandList(programOptions po);

int commandDiff(programOptions po);

int commandHelp(programOptions po);

#endif // COMMANDS_H
