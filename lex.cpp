#include "lex.hpp"

void ListOfLex::print()
{
	int cstr = -1;
	while (first)
	{
		if (cstr != first->strnumb)
		{
			cstr = first->strnumb;
			printf("\n%d	", cstr);
		}
		printf("%s (%d) ", first->str, first->type);
		first = first->next;
	}
	putchar('\n');
	putchar('\n');
}

void ListOfLex::add(lexeme *newl)
{
	if (!first)
	{
		first = newl;
	}
	else
	{
		struct lexeme *tmp;
		tmp = first;

		while (tmp->next)
		{
			tmp = tmp->next;
		}
		tmp->next = newl;
	}
}

void ListOfLex::del()
{
	lexeme *tmp;
	while (first)
	{
		tmp = first;
		first = first->next;
		if (tmp->str)
			delete [] tmp->str;
		delete tmp;
	}
}

Automat::Automat()
{
	buf = 0;
	lex = 0;
	nlex = 0;
	strnumb = 1;
	finlex = incomplete;
	cnd = begin;
	countq = 0;
}

Automat::~Automat()
{
	if (buf)
		delete [] buf;
}

bool Automat::Op()
{
	const char *arr = "!+-/*&|;<>~=()[]$@";
	int len = strlen(arr);
	for (int i = 0; i < len; i++)
		if (c == arr[i])
			return 1;
	return 0;
}

bool Automat::EndLex()
{
	return (c == ' ') || (c == '\n') || (c == '\t');
}

void  Automat::SetChar(int smb)
{
	c = smb;

	if (smb == EOF)
	{
		if (cnd == constline)
		{
			cnd = error;
			strcpy(buf, "Wrong number of quoters");
			FinLex();
		}
		else
		{
			c = ' ';
			FinLex();
		}
	}
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
	else if (cnd == fl)
		Float();
	if (c == '\n')
		strnumb++;
}

void Automat::Begin()
{
	lex = 0;
	nlex = 0;
	finlex = incomplete;

	if (buf)
		buf[0] = '\0';

	if (EndLex())
		FinLex();
	else if ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z'))
		Line();
	else if ((c >= '0') && (c <= '9'))
		Number();
	else if (c == '{')
		Comment();
	else if (c == '"')
		ConstLine();
	else if (Op())
		FinLex();
	else
	{
	//	throw LexErr("Wrong symbol!", strnumb, c);
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
	if (len + 2 > length)
	{
		cnd = error;
		strcpy(buf, "Long command\n");
		FinLex();
		return;
	}
	buf[len] = c;
	buf[len + 1] = '\0';
}

void Automat::Float()
{
	cnd = fl;
	if (Op() || EndLex())
		FinLex();
	else if (cnd != number && cnd != fl)
	{
		cnd = error;
		strcpy(buf, "Point is used by float only\n");
		FinLex();
//		return;
	}
	else
		Add();
}

void Automat::Line()
{
	cnd = line;
	if (Op())
		FinLex();
	else if (EndLex())
		FinLex();
	else if ((c >= 'a' && c <= 'z') || (c >='A' && c <= 'Z') ||
				(c >= '0' && c <= '9'))
		Add();
	else
	{
		cnd = error;
		strcpy(buf, "Wrong symbol in string ");
		int len = strlen(buf);
		buf[len] = c;
		buf[len + 1] = '\0';
		FinLex();
	}
}

void Automat::Number()
{
	cnd = number;
	if (c == '.')
		Float();
	else if (Op())
		FinLex();
	else if (EndLex())
		FinLex();
	else if ((c < '0') || (c > '9'))
	{
		cnd = error;
		strcpy(buf, "Wrong symbol in number ");
		int len = strlen(buf);
		buf[len] = c;
		buf[len + 1] = '\0';
		FinLex();
	}
	else
		Add();
}

void Automat::Comment()
{
	cnd = comment;
	if (c == '}')
		cnd = begin;
}

void Automat::ConstLine()
{
	cnd = constline;
	if (c == '"')
	{
		countq++;
		if (!(countq % 2))
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
	if (c == '<')
		nlex->type = lex_less;
	else if (c == '>')
		nlex->type = lex_more;
	else if (c == '~')
		nlex->type = lex_eq;
	else if (c =='=')
		nlex->type = lex_assign;
	else if (c == '@')
		nlex->type = lex_fin;
	else
		nlex->type = operation;
	nlex->str[0] = c;
	nlex->str[1] = '\0';
	nlex->strnumb = strnumb;
	nlex->next = 0;
	lex->next = nlex;
}

void Automat::FinOdOp()
{
	if (c == '<')
		lex->type = lex_less;
	else if (c == '>')
		lex->type = lex_more;
	else if (c == '~')
		lex->type = lex_eq;
	else if (c =='=')
		lex->type = lex_assign;
	else if (c == '@')
		lex->type = lex_fin;
	else
		lex->type = operation;
	lex->str[0] = c;
	lex->str[1] = '\0';
	lex->strnumb = strnumb;
	lex->next = 0;

}

void Automat::Finish()
{
	if (cnd == line)
	{
		int i = 1;
		while (tw[i])
		{
			if (!strcmp(buf, tw[i]))
			{
				lex->type = lex_word;
				break;
			}
			i++;
		}
		if (lex->type != lex_word)
			lex->type = lex_id;
	}
	else if (cnd == number)
		lex->type = lex_int;
	else if (cnd == fl)
		lex->type = lex_float;
	else if (cnd == constline)
		lex->type = lex_strlit;
	else if (cnd == operation )
	{
		if (c == '<')
			lex->type = lex_less;
		else if (c == '>')
			lex->type = lex_more;
		else if (c == '~')
			lex->type = lex_eq;
		else if (c =='=')
			lex->type = lex_assign;
		else if (c == '@')
			lex->type = lex_fin;
		else
			lex->type = operation;
	}
	else if (cnd == error)
		lex->type = lex_error;
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
		lex = new lexeme();
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
	if (EndLex() || cnd == constline)
	{
		Finish();
		lex->next = 0;
	}
	cnd = begin;
	finlex = complete;
}

lexeme *Automat::GiveLex()
{
	if (finlex == complete)
	{
		finlex = incomplete;
		return lex;
	}
	else
		return 0;
}

LexAnalysis::LexAnalysis()
{
	p = 0;
	plex = 0;
}

lexeme *LexAnalysis::GetLexList()
{
	int er = 0;
	while (p != EOF)
	{
		p = getchar();
		a.SetChar(p);
		plex = a.GiveLex();
		if (plex)
		{
			if (plex->type == lex_error)
			{
				er = 1;
				break;
			}
			list.add(plex);
		}
	}
	if (er)
	{
		list.print();
		return 0;
	}
	list.print();
	return list.first;
}