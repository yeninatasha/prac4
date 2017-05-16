#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

using namespace std;

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
		cout << line << msg << endl;
	}
	virtual ~Err()
	{
		delete [] msg;
	}
};

class LexErr: public Err
{
	char c;
public:
	LexErr(const char *newmsg, int nline, char nc)
			: Err(newmsg, nline)
	{
		c = nc;
	}
	virtual void Print()
	{
		cout << line << msg << c << endl;
	}
	virtual ~LexErr()
	{
	}
};

enum
{
	line, number, lable, equate, constline, operation, error
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

void dellex(struct lexeme *l)
{
	struct lexeme *tmp;
	while (l)
	{
		tmp = l;
		l = l->next;
		if (tmp->str)
			delete [] tmp->str;
		delete tmp;
	}
}

struct lexeme *newlex(struct lexeme *newl, struct lexeme *oldl)
{
	if (!oldl)
	{
		oldl = newl;
	}
	else
	{
		struct lexeme *tmp;
		tmp = oldl;
		while (tmp->next)
		{
			tmp = tmp->next;
		}
		tmp->next = newl;
	}
	return oldl;
}

void printlex(struct lexeme *l)
{
	int cstr = -1;
	while (l)
	{
		if (cstr != l->strnumb)
		{
			cstr = l->strnumb;
			cout << "\n" << cstr << "    ";
		}
		cout << "*" << l->str << "* ";
		l = l->next;
	}
	cout << endl;
}

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
	char *buf, *ebuf;
	struct lexeme *lex;
	struct lexeme *nlex;
	int countq;
	void Begin();
	void Line();
	void Number();
	void Comment();
	void Lable();
	void ConstLine();
	void Error();
	void Add();
	bool Op();
	bool EndLex();
	void FinLex();
	void FinOp();
	void FinOdOp();
	void FinLable();
	void Finish();
public:
	Automat();
	void SetChar(int smd);
	struct lexeme *GiveLex();
	~Automat();
};

Automat::Automat()
{
	buf = 0;
	lex = 0;
	nlex = 0;
	strnumb = 1;
	finlex = incomplete;
	cnd = begin;
	countq = 0;
	ebuf = 0;
}

Automat::~Automat()
{
	if (lex)
	{
		dellex(lex);
	}
	if (nlex)
	{
		dellex(nlex);
	}
	if (buf)
		delete [] buf;
	if (ebuf)
		delete [] ebuf;
}

bool Automat::Op()
{
	const char *arr = "!+-/,<>&|;.#=()[]";
	int len = strlen(arr);
	for (int i = 0; i < len; i++)
		if (c == arr[i])
			return true;
	return false;
}

bool Automat::EndLex()
{
	return (c == ' ') || (c == '\n') || (c == '\t');
}

void Automat::SetChar(int smd)
{
	c = smd;
	if (smd == EOF)
	{
		if (cnd == constline)
		{
			cnd = error;
			strcpy(buf, "Wrong number of quoters");
			FinLex();
		}
	}
	else if (ebuf)
		Lable();
	else if (cnd == begin)
		Begin();
	else if (cnd == line)
		Line();
	else if (cnd == number)
		Number();
	else if (cnd == comment)
		Comment();
	else if (cnd == constline)
		ConstLine();
	else if (cnd == operation)
		FinLex();
	if (c == '\n')
		strnumb++;
}

void Automat::Begin()
{
	lex = 0;
	nlex = 0;
	finlex = incomplete;
	if (ebuf)
		delete [] ebuf;
	ebuf = 0;
	if (buf)
		buf[0] = '\0';
	if (EndLex())
		FinLex();
	else if ((c >= 'a' && c <='z') || (c >='A' && c <= 'Z'))
		Line();
	else if (c >= '0' && c <= '9')
		Number();
	else if (c == ':')
		Lable();
	else if (c == '"')
		ConstLine();
	else if (Op())
		FinLex();
	else
	{
		cnd = error;
		strcpy(buf, "Wrong symbol ");
		int len = strlen(buf);
		buf[len] = c;
		buf[len + 1] = '\0';
		FinLex();
	}
}

void Automat::Add()
{
	if (!buf)
	{
		buf = new char [length];
		buf[0] = '\0';
	}
	int len = strlen(buf);
	if (len+ + 2 > length)
	{
		cnd = error;
		strcpy(buf, "Long command\n");
		FinLex();
		return;
	}
	buf[len] = c;
	buf[len + 1] = '\0';
}

void Automat::Line()
{
	cnd = line;
	if (Op())
		FinLex();
	else if (EndLex())
		FinLex();
	else if (c == ':')
		Lable();
	else
		Add();
}

void Automat::Number()
{
	cnd = number;
	if (Op())
		FinLex();
	else if (EndLex())
		FinLex();
	else if ((c < '0') || (c > '9'))
		Line();
	else Add();
}

void Automat::Comment()
{
	cnd = comment;
	if (c == '}')
		cnd = begin;
}

void Automat::Lable()
{
	if (c == ':')
	{
		if (!ebuf)
		{
		 	ebuf = new char [length];
			ebuf[0] = '\0';
		}
		ebuf[0] = c;
		ebuf[1] = '\0';
	}
	else if (c == '=')
	{
		ebuf[1] = '=';
		ebuf[2] = '\0';
		FinLex();
	}
	else
		FinLex();
}

void Automat::ConstLine()
{
	cnd = constline;
	if (c == '"')
		countq++;
	if ((c == '"') && (!(countq % 2)))
	{
		Add();
		FinLex();
	}
	else
		Add();
}

void Automat::FinOp()
{
	if (!nlex)
	{
		nlex = new struct lexeme();
		nlex->str = new char [length];
	}
	nlex->type = operation;
	nlex->str[0] = c;
	nlex->str[1] = '\0';
	nlex->strnumb = strnumb; 
	nlex->next = 0;
	lex->next = nlex;
}

void Automat::FinOdOp()
{
	lex->type = operation;
	lex->str[0] = c;
	lex->str[1] = '\0';
	lex->strnumb = strnumb; 
	lex->next = 0;

}

void Automat::FinLable()
{
	if (cnd != begin)
	{
		Finish();
		lex->strnumb = strnumb;
		lex->next = 0;
	}
	if (strlen(ebuf) == 1)
	{
		FinOp();
		nlex->type = lable;
		nlex->str[0] = ':';
		nlex->str[1] = '\0';
	}
	else
	{
		if (!nlex)
		{
			nlex = new struct lexeme();
			nlex->str = new char [length];
		}
		nlex->type = equate;
		int len = strlen(ebuf);
		for ( int i = 0; i <= len; i++)
			nlex->str[i] = ebuf[i];
		nlex->strnumb = strnumb; 
		nlex->next = 0;
		lex->next = nlex;
	}	
}

void Automat::Finish()
{
	lex->type = cnd;
	for (unsigned int i = 0; i <= strlen(buf); i++)
		lex->str[i] = buf[i];	
	lex->strnumb = strnumb;

}

void Automat::FinLex()
{
	if ((cnd == begin && !Op()) || cnd == comment)
		return;
	if (!lex)
	{
		lex = new struct lexeme();
		lex->str = new char [length];
	}
	if (Op())
	{
		if (cnd != begin)
		{
			Finish();
			FinOp();
		}
		else
			FinOdOp();
	}
	if (cnd == error)
		Finish();
	if (ebuf)
		FinLable();
	if (EndLex() || cnd == constline)
	{
		Finish();
		if (!ebuf)
			lex->next = 0;
	}
	cnd = begin;
	finlex = complete;
	if (ebuf)
		delete [] ebuf;
	ebuf = 0;
}

struct lexeme *Automat::GiveLex()
{
	if (finlex == complete)
	{
		finlex = incomplete;
		return lex;
	}
	else
		return 0;
}

int redirect(char *name)
{
	int fd = -1;
	fd = open(name, O_RDONLY);
	if (fd == -1)
	{
		cout << "Error in file opening" << endl;
		return -1;
	}
	else
	{
		dup2(fd, 0);
		if (close(fd) != 0)
		{
			cout << "Error in file closing" << endl;
			return -1;
		}
	}
	return fd;
}

class LexAnalysis
{
	Automat a;
	struct lexeme *lexhead, *lextail, *plex;
	int p;
public:
	struct lexeme *GetLexList();
	LexAnalysis();
	~LexAnalysis();
};

LexAnalysis::LexAnalysis()
{
	p=0;
	lexhead = lextail = plex = 0;
}

LexAnalysis::~LexAnalysis()
{
	if (lextail)
		dellex(lextail);
	if (lexhead)
		dellex(lexhead);
//	if (plex)
//		dellex(plex);
}

struct lexeme *LexAnalysis::GetLexList()
{
	int er = 0;
	while (p != EOF)
	{
		p = getchar();
		a.SetChar(p);
		plex = a.GiveLex();
		if (plex)
		{
			if (plex->type == error)
			{
				er = 1;
				break;
			}
			lexhead = newlex(plex, lexhead);
		}
	}
	if (er)
	{
		cout << plex->str << endl;
		return NULL;
	}
	printlex(lexhead);
	return lexhead;
}

int main (int argc, char **argv)
{
	struct lexeme *lexlist;
	LexAnalysis la;

	if (argc != 2)
	{
		cout << "Wrong number of arguments" << endl;
		return 1;
	}
	if (redirect(argv[1]) == -1)
		return 1;
	lexlist = la.GetLexList();
	if (!lexlist)
		return 1;
	return 0;
}