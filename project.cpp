#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "executor.hpp"

int redirect(char *name)
{
	int fd = -1;
	fd = open(name, O_RDONLY);
	if (fd == -1)
	{
		printf("Error in file opening\n");
		return -1;
	}
	else
	{
		dup2(fd, 0);
		if (close(fd) != 0)
		{
			printf("Error in file closing\n");
			return -1;
		}
	}
	return fd;
}

int main (int argc, char **argv)
{
	ListOfLex lexlist;
	LexAnalysis la;
	Syntax sa;

	if (argc != 2)
	{
		printf("Wrong number of arguments\n");
		return 1;
	}
	if (redirect(argv[1]) == -1)
		return 1;
	lexlist.first = la.GetLexList();
	if (!lexlist.first)
		return 1;
	try
	{
		sa.ScanLex(lexlist.first);
		Executor ex(sa);
		ex.Print();
		ex.Execute();
		printf("This program ended sucsessfully\n");
	}
	catch(SyntErr &e)
	{
		e.Print();
	}
	catch(RPNErr &er)
	{
		er.Print();
	}
	catch (RPNEx &err)
	{
		err.Print();
	}

	return 0;
}