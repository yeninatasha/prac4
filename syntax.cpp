#include "syntax.hpp"

Syntax::Syntax()
{
	currlex = 0;
	rpnhead = rpntail = 0;
	sn = 0;
	type = lex_fin;
	str = 0;
}

RPNItem *Syntax::GetRPNList()
{
	return rpnhead;
}

void Syntax::Next()
{
	if (currlex->next)
	{
		currlex = currlex->next;
		sn = currlex->strnumb;
		type = currlex->type;
		str = currlex->str;
	}
	else if (type != lex_fin)
		throw SyntErr("List ended unexpectedly", sn);
}

void Syntax::ScanLex(struct lexeme *lexhead)
{
	currlex = lexhead;
	sn = currlex->strnumb;
	type = currlex->type;
	str = currlex->str;
	P();
	printf("This program is syntactially correct\n");
}

void Syntax::P()
{
	if (strcmp(str, tw[tw_program])) 
		throw SyntErr("There isn't a program", sn);
	Next();
	B();
	if (type != lex_fin)
		throw SyntErr("'@' wanted in the program's end", sn);
}

void Syntax::B()
{
	if (strcmp(str, tw[tw_begin]))
		throw SyntErr("'begin' wanted", sn);
	Next();
	S();
	while (str[0] == ';')
	{
		Next();
		S();
	}
	if (!strcmp(str, tw[tw_end]))
		Next();
	else
		throw SyntErr("'end' wanted", sn);
	if (str[0] == '@')
	{
		if (currlex->next)
			throw SyntErr("Symbol after '@' detected", sn);
	}
}

void Syntax::S()
{
	if (type == lex_id)
	{
		I();
		EIDX();
		if (type != lex_assign)
			throw SyntErr("Wrong assign statement", sn);
		Next();
		E();
	// '=' to RPNlist
		RPNItem *tmp;
		RPNFunAssign *a;
		a = new RPNFunAssign();
		tmp = new RPNItem;
		tmp->p = a;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
	}
	else if (!strcmp(str, tw[tw_if]))
	{
		Next();
		E();

		RPNItem *tmp1;
		RPNLabel *l1;
		l1 = new RPNLabel(0);
		tmp1 = new RPNItem;
		tmp1->p = l1;
		tmp1->next = 0;
		newrpn(&rpnhead, &rpntail, tmp1);

		RPNItem *tmp2;
		RPNCondBranch *cb;
		cb = new RPNCondBranch();
		tmp2 = new RPNItem;
		tmp2->p = cb;
		tmp2->next = 0;
		newrpn(&rpnhead, &rpntail, tmp2);

		if (strcmp(str, tw[tw_then]))
			throw SyntErr("'then' wanted in conditional statement",sn);
		Next();
		S();

		RPNItem *tmp5;
		RPNNop *n;
		n = new RPNNop();
		tmp5 = new RPNItem;
		tmp5->p = n;
		tmp5->next = 0;
		newrpn(&rpnhead, &rpntail, tmp5);
		l1->Set(tmp5);
	
		if (!strcmp(str, tw[tw_else]))
		{

			RPNItem *tmp3;
			RPNLabel *l2;
			l2 = new RPNLabel(0);
			tmp3 = new RPNItem;
			tmp3->p = l2;
			tmp3->next = 0;
			newrpn(&rpnhead, &rpntail, tmp3);
	
			RPNItem *tmp4;
			RPNBranch *b;
			b = new RPNBranch();
			tmp4 = new RPNItem;
			tmp4->p = b;
			tmp4->next = 0;
			newrpn(&rpnhead, &rpntail, tmp4);

			Next();
			S();

			RPNItem *tmp6;
			RPNNop *n2;
			n2 = new RPNNop();
			tmp6 = new RPNItem;
			tmp6->p = n2;
			tmp6->next = 0;
			newrpn(&rpnhead, &rpntail, tmp6);
			l2->Set(tmp6);
		}
	}
	else if (!strcmp(str, tw[tw_while]))
	{
		RPNItem *tmp0;
		RPNNop *n0;
		n0 = new RPNNop();
		tmp0 = new RPNItem;
		tmp0->p = n0;
		tmp0->next = 0;
		newrpn(&rpnhead, &rpntail, tmp0);

		Next();
		E();

		RPNItem *tmp1;
		RPNLabel *a;
		a = new RPNLabel(0);
		tmp1 = new RPNItem;
		tmp1->p = a;
		tmp1->next = 0;
		newrpn(&rpnhead, &rpntail, tmp1);

		RPNItem *tmp2;
		RPNCondBranch *cb;
		cb = new RPNCondBranch();
		tmp2 = new RPNItem;
		tmp2->p = cb;
		tmp2->next = 0;
		newrpn(&rpnhead, &rpntail, tmp2);
	
		if (strcmp(str, tw[tw_do]))
			throw SyntErr("'do' wanted in while-cycle", sn);
		Next();
		S();

		RPNItem *tmp3;
		RPNLabel *l2;
		l2 = new RPNLabel(tmp0);
		tmp3 = new RPNItem;
		tmp3->p = l2;
		tmp3->next = 0;
		newrpn(&rpnhead, &rpntail, tmp3);

		RPNItem *tmp4;
		RPNBranch *b;
		b = new RPNBranch();
		tmp4 = new RPNItem;
		tmp4->p = b;
		tmp4->next = 0;
		newrpn(&rpnhead, &rpntail, tmp4);

		RPNItem *tmp5;
		RPNNop *n;
		n = new RPNNop();
		tmp5 = new RPNItem;
		tmp5->p = n;
		tmp5->next = 0;
		newrpn(&rpnhead, &rpntail, tmp5);
		a->Set(tmp5);
	}
	else if (!strcmp(str, tw[tw_begin]))
		B();
	else if (!strcmp(str, tw[tw_print]))
	{
		Next(); 
		LE();
		while (str[0] == ',')
		{
			Next();
			LE();
		}

		RPNItem *tmp5;
		RPNFunPrint *n;
		n = new RPNFunPrint();
		tmp5 = new RPNItem;
		tmp5->p = n;
		tmp5->next = 0;
		newrpn(&rpnhead, &rpntail, tmp5);
	}
	else if (str[0] == ';')
		return;
	else if (!strcmp(str, tw[tw_end]))
		return;
	else
		throw SyntErr("Wrong statement", sn);

}

void Syntax::EIDX()
{
	if (str[0] == '[')
	{
		Next();
		E();
		if (str[0] != ']')
			throw SyntErr("Wrong index", sn);
		Next();
	
		// [] to RPNlist
		RPNItem *tmp;
		RPNFunIndex *index;
		index = new RPNFunIndex();
		tmp = new RPNItem;
		tmp->p = index;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
	}
}

void Syntax::LE()
{
	if (type != lex_strlit)
		E();
	else
	{
		RPNItem *tmp;
		RPNString *n;
		n = new RPNString(str);
		tmp = new RPNItem;
		tmp->p = n;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
		Next();
	}
}

void Syntax::E()
{
	E1();
	E2();
}

void Syntax::E2()
{
	if (type == lex_eq || type == lex_less || type == lex_more)
	{
		int tmptype = type;
		Next();
		E1();
		// Operation to RPNlist
		RPNItem *tmp;
		RPNFunction *op;
		if (tmptype == lex_eq)
			op = new RPNFunEq();
		else if (tmptype == lex_less)
			op = new RPNFunLess();
		else 
			op = new RPNFunMore();
		tmp = new RPNItem;
		tmp->p = op;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
	}
}

void Syntax::E1()
{
	T();
	while (str[0] == '+' || str[0] == '-' || str[0] == '|')
	{
		char tmpc = str[0];
		Next();
		T();
		//Operation to RPNlist
		RPNItem *tmp;
		RPNFunction *op;
		if (tmpc == '+')
			op = new RPNFunPlus();
		else if (tmpc == '-')
			op = new RPNFunMinus();
		else
			op = new RPNFunOr();
		tmp = new RPNItem;
		tmp->p = op;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
	}
}

void Syntax::T()
{
	F();
	while (str[0] == '*' || str[0] == '/' || str[0] == '&')
	{
		char tmpc = str[0];
		Next();
		F();
		//Operation to RPNlist
		RPNItem *tmp;
		RPNFunction *op;
		if (tmpc == '*')
			op = new RPNFunMultiply();
		else if (tmpc == '/')
			op = new RPNFunDivide();
		else
			op = new RPNFunAnd();
		tmp = new RPNItem;
		tmp->p = op;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
	}
}

void Syntax::F()
{
	if (type == lex_id)
	{
		RPNItem *tmp;
		RPNVar *ident;
		ident = new RPNVar(str);
		tmp = new RPNItem;
		tmp->p = ident;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);

		Next();
		EIDX();
		
		if (str[0] == '$')
		{
			RPNItem *tmp2;
			RPNFunGetVal *gv;
			gv = new RPNFunGetVal();
			tmp2 = new RPNItem;
			tmp2->p = gv;
			tmp2->next = 0;
			newrpn(&rpnhead, &rpntail, tmp2);
			Next();
		}
		else
			throw SyntErr("'$' expected after ident in expression"
									, sn);
	}
	else if (type == lex_int)
	{
		RPNItem *tmp;
		RPNInt *i;
		i = new RPNInt(atoi(str));
		tmp = new RPNItem;
		tmp->p = i;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
		Next();
	}
	else if (type == lex_float)
	{
		RPNItem *tmp;
		RPNFloat *f;
		float tmpf;
		sscanf(str, "%f", &tmpf);
		f = new RPNFloat(tmpf);
		tmp = new RPNItem;
		tmp->p = f;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
		Next();
	}
	else if (str[0] == '!')
	{
		Next();
		F();

		RPNItem *tmp;
		RPNFunNot *n;
		n = new RPNFunNot();
		tmp = new RPNItem;
		tmp->p = n;
		tmp->next = 0;
		newrpn(&rpnhead, &rpntail, tmp);
	}
	else if (str[0] == '-')
	{
		RPNItem *tmp1;
		RPNInt *i;
		i = new RPNInt(0);
		tmp1 = new RPNItem;
		tmp1->p = i;
		tmp1->next = 0;
		newrpn(&rpnhead, &rpntail, tmp1);

		Next();
		F();

		RPNItem *tmp2;
		RPNFunMinus *m;
		m = new RPNFunMinus();
		tmp2 = new RPNItem;
		tmp2->p = m;
		tmp2->next = 0;
		newrpn(&rpnhead, &rpntail, tmp2);
	}
	else if (str[0] == '(')
	{
		RPNItem *tmp1;
		RPNFunParOp *po;
		po = new RPNFunParOp();
		tmp1 = new RPNItem;
		tmp1->p = po;
		tmp1->next = 0;
		newrpn(&rpnhead, &rpntail, tmp1);

		Next();
		E();
		if (str[0] != ')')
			throw SyntErr("')' wanted", sn);
		
		RPNItem *tmp2;
		RPNFunParCl *pc;
		pc = new RPNFunParCl();
		tmp2 = new RPNItem;
		tmp2->p = pc;
		tmp2->next = 0;
		newrpn(&rpnhead, &rpntail, tmp2);

		Next();
	}
	else
		throw SyntErr("Wrong expression", sn);
}

void Syntax::I()
{
	if (type != lex_id)
		throw SyntErr("Wrong identifier", sn);
	RPNItem *tmp;
	RPNVar *ident;
	ident = new RPNVar(str);
	tmp = new RPNItem;
	tmp->p = ident;
	tmp->next = 0;
	newrpn(&rpnhead, &rpntail, tmp);
	Next();
}