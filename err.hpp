#ifndef _ERR_H_
#define _ERR_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

class Err
{
protected:
	char *msg;
	int line;
public:
	Err(const char *newmsg, int nline)
	{
		int len = strlen(newmsg);
		msg = new char [len + 1];
		for (int i = 0; i <= len; i++)
			msg[i] = newmsg[i];
		line = nline;
	}
	virtual void Print()
	{
		printf("%d	%s\n", line, msg);
	}
	virtual ~Err()
	{
		delete [] msg;
	}
};

class SyntErr: public Err
{
public:
	SyntErr(const char *newmsg, int nline)
			: Err(newmsg, nline)
	{}	
	virtual void Print()
	{
		printf("Syntax error:	%s, str %d\n\n", msg, line);
	}
	virtual ~SyntErr()
	{}
};

class RPNErr: public Err
{
public:
	RPNErr(const char *newmsg)
			: Err(newmsg, 0)
	{}	
	virtual void Print()
	{
		printf("\nRPN error:	%s\n\n", msg);
	}
	virtual ~RPNErr()
	{}
};

#endif