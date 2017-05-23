#ifndef _LEX_H_
#define _LEX_H_

// #include "synt.h"
#include "err.hpp"
// #include "rpn.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

enum
{
	first_1, tw_begin,  tw_do, tw_else, tw_end,
	tw_float, tw_if, tw_int, tw_print, tw_program,
	tw_then, tw_while,
	line, number, label, equate, constline, operation, error, fl,
	funct, lex_float, lex_word, lex_int, lex_id, lex_label,
	lex_assign, lex_funct, lex_strlit, lex_less, lex_more, lex_eq,
	lex_game, lex_function, lex_null, lex_fin, lex_error
};


static const char *tw[] =
{
	"",			//0
	"begin",	//1
	"do",		//4
	"else",		//5
	"end",		//6
	"float",	//8
	"if",		//9
	"int",		//10
	"print",	//11
	"program",	//13
	"then",		//15
	"while",
	0	//17
};

struct lexeme
{
	int type;
	int strnumb;
	char *str;
	struct lexeme *next;
	lexeme()
	{
		strnumb = 0;
		str = 0;
		next = 0;
	}
};

void plwn(struct lexeme *l);

void printlex(struct lexeme *l);

struct lexeme *newlex(struct lexeme *newl, struct lexeme *oldl);

void dellex(struct lexeme *l);

class Automat
{
	enum
	{
		begin = 100, comment = 101,
		complete, incomplete, length = 256
	};
	char c;
	int cnd;
	int strnumb;
	int finlex;
	char *buf;
	struct lexeme *lex;
	struct lexeme *nlex;
	int countq;
	void Begin();
	void Line();
	void Number();
	void Comment();
	void ConstLine();
	void Error();
	void Float();
	void Function();

	void Add();
	bool Op();
	bool EndLex();
	void FinLex();
	void FinOp();
	void FinOdOp();
	void Finish();
public:
	Automat();
	void SetChar(int smb);
	struct lexeme *GiveLex();
	~Automat();
};

class LexAnalysis
{
	Automat a;
	struct lexeme *lexhead, *lextail, *plex;
	int p;
public:
	struct lexeme *GetLexList();
	LexAnalysis();
};

#endif