#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

#include "syntax.hpp"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

class Executor
{
	RPNItem *curr;
	RPNItem *stack;
	TVar *table;

public:
	Executor(Syntax sa);
	void Execute();
	void Print() const;
};
#endif