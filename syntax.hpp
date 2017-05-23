#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#include "rpn.hpp"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

class Syntax
{
	struct lexeme *currlex;
	RPNItem *rpnhead;
	RPNItem *rpntail;
	int sn;
	int type;
	char *str;

	void P();
	void D1();
	void D();
	void TP();
	void DIDX();
	void B();
	void S();
	void EIDX();
	void LE();
	void E();
	void E1();
	void E2();
	void T();
	void F();
	void I();

	void Next();
public:
	Syntax();
	void ScanLex(struct lexeme *leahead);
	RPNItem *GetRPNList();
};

#endif