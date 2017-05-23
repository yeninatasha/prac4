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

struct TVar
{
	char *name;
	int type;
	int intvalue;
	float floatvalue;
	TVar *next;
	TVar()
	{
		name = 0;
		intvalue = floatvalue = 0;
		next = 0;
	}
};

class RPNElem;

struct RPNItem
{
	RPNElem *p;
	RPNItem *next;
};

class RPNElem
{
protected:
	static void Push(RPNItem **stack, RPNElem *elem);
	static RPNElem *Pop(RPNItem **stack);
public:
	virtual void Evaluate(RPNItem **stack, RPNItem **cur_cmd,
								TVar **table) const = 0;
	virtual void Print() const = 0;
	virtual ~RPNElem()
	{}
};

void RPNElem::Push(RPNItem **stack, RPNElem *elem)
{
	if (!*stack)
	{
		*stack = new RPNItem;
		(*stack)->p = elem;
		(*stack)->next = 0;
	}
	else
	{
		RPNItem *tmp;
		tmp = new RPNItem;
		tmp->p = elem;
		tmp->next = *stack;
		*stack = tmp;
	}
}

RPNElem *RPNElem::Pop(RPNItem **stack)
{
	if (*stack)
	{
		RPNElem *tmp;
		tmp = (*stack)->p;
		RPNItem *head;
		head = *stack;
		*stack = (*stack)->next;
		delete head;
		return tmp;
	}
	else
		throw RPNErr("Attempt to take from clean stack");
	return 0;
}

class RPNSimple: public RPNElem
{
public:
	virtual void Evaluate(RPNItem **stack, RPNItem **cur_cmd, 
								TVar **table) const;
	virtual void SimpleEval(RPNItem **stack,
								TVar **table) const = 0;
	virtual void Print() const = 0;
	virtual ~RPNSimple()
	{}
	
};

void RPNSimple::Evaluate(RPNItem **stack, RPNItem **cur_cmd, 
								TVar **table) const
{
	SimpleEval(stack, table);
	*cur_cmd = (*cur_cmd)->next;
}

class RPNConst: public RPNSimple
{
public:
	virtual void SimpleEval(RPNItem **stack, 
								TVar **table) const
	{
		Push(stack, Clone());
	}
	virtual RPNElem *Clone() const = 0;
	virtual void Print() const = 0;
	virtual ~RPNConst()
	{}
};

class RPNInt: public RPNConst
{
	int value;
public:
	RPNInt(int a)
	{
		value = a;
	}
	virtual ~RPNInt()
	{}
	virtual RPNElem *Clone() const
	{
		return new RPNInt(value);
	}
	int Get() const
	{
		return value;
	}
	virtual void Print() const
	{
		printf("%d", value);
	}
};

class RPNFloat: public RPNConst
{
	float value;
public:
	RPNFloat(float a)
	{
		value = a;
	}
	virtual ~RPNFloat()
	{}
	virtual RPNElem *Clone() const
	{
		return new RPNFloat(value);
	}
	float Get() const
	{
		return value;
	}
	virtual void Print() const
	{
		printf("%.3f", value);
	}
};

class RPNLabel: public RPNConst
{
	RPNItem *value;
public:
	RPNLabel(RPNItem *a)
	{
		value = a;
	}
	virtual ~RPNLabel()
	{}
	virtual RPNElem *Clone() const
	{
		return new RPNLabel(value);
	}
	RPNItem *Get() const
	{
		return value;
	}
	void Set(RPNItem *a)
	{
		value = a;
	}
	virtual void Print() const
	{
		printf("lable");
	}
};

class RPNNop: public RPNConst
{
public:
	RPNNop()
	{}
	virtual ~RPNNop()
	{}
	virtual RPNElem *Clone() const
	{
		return new RPNNop();
	}
	virtual void Print() const
	{
		printf("NOP");
	}
};

class RPNString: public RPNConst
{
	char *value;
public:
	RPNString(char *a)
	{
		int len = strlen(a);
		value = new char [len + 1];
		for (int i = 0; i <= len; i++)
			value[i] = a[i];
	}
	virtual ~RPNString()
	{
		delete [] value;
	}
	virtual RPNElem *Clone() const
	{
		return new RPNString(value);
	}
	char *Get() const
	{
		return value;
	}
	virtual void Print() const
	{
		printf("%s", value);
	}
};

class RPNVar: public RPNConst
{
	char *name;
public:
	RPNVar(char *a)
	{
		int len = strlen(a);
		name = new char[len + 1];
		for (int i = 0; i <= len; i++)
			name[i] = a[i];
	}
	virtual ~RPNVar()
	{
		delete [] name;
	}
	virtual RPNElem *Clone() const
	{
		return new RPNVar(name);
	}
	char *Get() const
	{
		return name;
	}
	virtual void Print() const
	{
		printf("%s", name);
	}
	void Set(char *newname)
	{
		int len = strlen(newname);
		name = new char[len + 1];
		for (int i = 0; i <= len; i++)
			name[i] = newname[i];
	}
};

class RPNEx
{
protected:
	RPNElem *e;
public:
	virtual void Print() const = 0;
	virtual ~RPNEx()
	{}
};

class RPNExNotVar: public RPNEx
{
public:
	RPNExNotVar(RPNElem *ne)
	{
		e = ne;
	}
	void Print() const
	{
		printf("Wrong variable used\n");
	}
};

class RPNExNotNumber: public RPNEx
{
public:
	RPNExNotNumber(RPNElem *ne)
	{
		e = ne;
	}
	void Print() const
	{
		printf("Wrong number used\n");
	}
};

class RPNExNotLabel: public RPNEx
{
public:
	RPNExNotLabel(RPNElem *ne)
	{
		e = ne;
	}
	void Print() const
	{
		printf("Wrong lable used\n");
	}
};

class RPNExNotString: public RPNEx
{
public:
	RPNExNotString(RPNElem *ne)
	{
		e = ne;
	}
	void Print() const
	{
		printf("Wrong string used\n");
	}
};

class RPNExNotVarAddr: public RPNEx
{
public:
	RPNExNotVarAddr(RPNElem *ne)
	{
		e = ne;
	}
	void Print() const
	{
		printf("Wrong variable addres used\n");
	}
};

void newvar(TVar **table, TVar *var)
{
	if (!(*table))
	{
		*table = new TVar;
		*table = var;
	}
	else
	{
		TVar *curr;
		curr = *table;
		while (curr->next)
			curr = curr->next;
		curr->next = var;
	}
}

TVar *findvar(TVar *head, RPNVar *var)
{
	if (!head)
		return 0;
	while (head)
	{
		if (!strcmp(head->name, var->Get()))
			return head;
		head = head->next;
	}
	return 0;
}

void setvar(TVar **table, RPNVar *var, RPNElem *n)
{
	TVar *curr;
	curr = findvar(*table, var);
	if (curr->type == lex_int)
	{
		RPNInt *i = dynamic_cast<RPNInt*>(n);
		if (i)
			curr->intvalue = (int)(i->Get());
		else
		{
			RPNFloat *f = dynamic_cast<RPNFloat*>(n);
			if (!f)
				throw RPNExNotNumber(n);
			curr->intvalue = (int)(f->Get());
		}
	}
	else 
	{
		RPNInt *i = dynamic_cast<RPNInt*>(n);
		if (i)
			curr->floatvalue = (float)(i->Get());
		else
		{
			RPNFloat *f = dynamic_cast<RPNFloat*>(n);
			if (!f)
				throw RPNExNotNumber(n);
			curr->floatvalue = (float)(f->Get());
		}
	}
}

class RPNFunction: public RPNSimple
{
public:
	virtual RPNElem *EvaluateFun(RPNItem **stack,
							TVar **table) const = 0;
	virtual void SimpleEval(RPNItem **stack,
							TVar **table) const;
	virtual void Print() const = 0;
};

void RPNFunction::SimpleEval(RPNItem **stack, TVar **table) const
{
	RPNElem *res = EvaluateFun(stack, table);
	if (res)
		Push(stack, res);
}

class RPNFunPlus: public RPNFunction
{
public:
	RPNFunPlus()
	{}
	virtual ~RPNFunPlus()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int f1 = 0, f2 = 0;
		int resint;
		float resfloat;
		RPNInt *int1, *int2;
		RPNFloat *float1, *float2;
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1) 
		{
			float1 = dynamic_cast<RPNFloat*>(operand1);
			if (!float1)
				throw RPNExNotNumber(operand1);
			f1 = 1;
		}
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2)
		{
			float2 = dynamic_cast<RPNFloat*>(operand2);
			if (!float2)
				throw RPNExNotNumber(operand2);
			f2 = 1;
		}
		if (!f1 && !f2)
			resint = int1->Get() + int2->Get();
		else
		{
			if (f1 && !f2)
				resfloat = float1->Get() + int2->Get();
			else if (!f1 && f2)
				resfloat = int1->Get() + float2->Get();
			else
				resfloat = float1->Get() + float2->Get();
		}
		delete operand1;
		delete operand2;
		if (!f1 && !f2)
			return new RPNInt(resint);
		return new RPNFloat(resfloat);
	}
	virtual void Print() const
	{
		printf("+");
	}
};

class RPNFunMinus: public RPNFunction
{
public:
	RPNFunMinus()
	{}
	virtual ~RPNFunMinus()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int f1 = 0, f2 = 0;
		int resint;
		float resfloat;
		RPNInt *int1, *int2;
		RPNFloat *float1, *float2;
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1) 
		{
			float1 = dynamic_cast<RPNFloat*>(operand1);
			if (!float1)
				throw RPNExNotNumber(operand1);
			f1 = 1;
		}
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2)
		{
			float2 = dynamic_cast<RPNFloat*>(operand2);
			if (!float2)
				throw RPNExNotNumber(operand2);
			f2 = 1;
		}
		if (!f1 && !f2)
			resint = int2->Get() - int1->Get();
		else
		{
			if (!f1 && f2)
				resfloat = float2->Get() - int1->Get();
			else if (f1 && !f2)
				resfloat = int2->Get() - float1->Get();
			else
				resfloat = float2->Get() - float1->Get();
		}
		delete operand1;
		delete operand2;
		if (!f1 && !f2)
			return new RPNInt(resint);
		return new RPNFloat(resfloat);
	}
	virtual void Print() const
	{
		printf("-");
	}
};

class RPNFunMultiply: public RPNFunction
{
public:
	RPNFunMultiply()
	{}
	virtual ~RPNFunMultiply()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int f1 = 0, f2 = 0;
		int resint;
		float resfloat;
		RPNInt *int1, *int2;
		RPNFloat *float1, *float2;
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1) 
		{
			
			float1 = dynamic_cast<RPNFloat*>(operand1);
			if (!float1)
				throw RPNExNotNumber(operand1);
			f1 = 1;
		}
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2)
		{
			float2 = dynamic_cast<RPNFloat*>(operand2);
			if (!float2)
				throw RPNExNotNumber(operand2);
			f2 = 1;
		}
		if (!f1 && !f2)
			resint = int1->Get() * int2->Get();
		else
		{
			if (f1 && !f2)
				resfloat = float1->Get() * int2->Get();
			else if (!f1 && f2)
				resfloat = int1->Get() * float2->Get();
			else
				resfloat = float1->Get() * float2->Get();
		}
		delete operand1;
		delete operand2;
		if (!f1 && !f2)
			return new RPNInt(resint);
		return new RPNFloat(resfloat);
	}
	virtual void Print() const
	{
		printf("*");
	}
};

class RPNFunDivide: public RPNFunction
{
public:
	RPNFunDivide()
	{}
	virtual ~RPNFunDivide()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int f1 = 0, f2 = 0;
		float resfloat;
		RPNInt *int1, *int2;
		RPNFloat *float1, *float2;
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1) 
		{
			
			float1 = dynamic_cast<RPNFloat*>(operand1);
			if (!float1)
				throw RPNExNotNumber(operand1);
			f1 = 1;
		}
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2)
		{
			float2 = dynamic_cast<RPNFloat*>(operand2);
			if (!float2)
				throw RPNExNotNumber(operand2);
			f2 = 1;
		}
		if (!f1 && !f2)
			resfloat = int2->Get() / int1->Get();
		else
		{
			if (!f1 && f2)
				resfloat = float2->Get() / int1->Get();
			else if (f1 && !f2)
				resfloat = int2->Get() / float1->Get();
			else
				resfloat = float2->Get() / float1->Get();
		}
		delete operand1;
		delete operand2;
		return new RPNFloat(resfloat);
	}
	virtual void Print() const
	{
		printf("/");
	}
};

class RPNFunIndex: public RPNFunction
{
public:
	RPNFunIndex()
	{}
	virtual ~RPNFunIndex()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		RPNElem *operand1 = Pop(stack);
		RPNInt *i = dynamic_cast<RPNInt*>(operand1);
		if (!i)
			throw RPNExNotNumber(operand1);
		RPNElem *operand2 = Pop(stack);
		RPNVar *v = dynamic_cast<RPNVar*>(operand2);
		if (!v)
			throw RPNExNotVar(operand2);
		char index[40];
		sprintf(index, "%d", i->Get());
		char *name;
		name = new char [strlen(v->Get()) + strlen(index) + 3];
		strcpy(name, v->Get());
		int len = strlen(name);
		name[len] = '[';
		strcpy(name + len + 1, index);
		len = strlen(name);
		name[len] = ']';
		name[len + 1] = '\0';
		v->Set(name);
		return v;
	}
	virtual void Print() const
	{
		printf("[]");
	}
};

class RPNFunAssign: public RPNFunction
{
public:
	RPNFunAssign()
	{}
	virtual ~RPNFunAssign()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		RPNInt *i;
		RPNFloat *f;
		int fl = 0;
		RPNElem *operand1 = Pop(stack);
		i = dynamic_cast<RPNInt*>(operand1);
		if (!i)
		{
			f = dynamic_cast<RPNFloat*>(operand1);
			if (!f)
				throw RPNExNotNumber(operand1);
			fl = 1;
		}
		RPNElem *operand2 = Pop(stack);
		RPNVar *v = dynamic_cast<RPNVar*>(operand2);
		if (!v)
			throw RPNExNotVar(operand2);
		TVar *var;
		var = findvar(*table, v);
		if (!var)
		{
			TVar *tmp;
			tmp = new TVar;
			tmp->next = 0;
			tmp->name = new char[strlen(v->Get() + 1)];
			strcpy(tmp->name, v->Get());
			if (fl)
			{
				tmp->type = lex_float;
				tmp->floatvalue = f->Get();
			}
			else
			{
				tmp->type = lex_int;
				tmp->intvalue = i->Get();
			}
			newvar(table, tmp);
		}
		else
			setvar(table, v, operand1);
		return 0;
	}
	virtual void Print() const
	{
		printf("=");
	}
};

class RPNFunGetVal: public RPNFunction
{
public: 
	RPNFunGetVal()
	{}
	virtual ~RPNFunGetVal()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		RPNVar *v;
		RPNElem *operand1 = Pop(stack);
		v = dynamic_cast<RPNVar*>(operand1);
		if (!v)
			throw RPNExNotVar(operand1);
		TVar *curr = findvar(*table, v);
		if (!curr)
//			throw RPNErr("Variable didn't declare");
			throw RPNExNotVar(operand1);
		if (curr->type == lex_float)
			return new RPNFloat(curr->floatvalue);
		return new RPNInt(curr->intvalue);
	}
	virtual void Print() const
	{
		printf("$");
	}
};

class RPNFunMore: public RPNFunction
{
public:
	RPNFunMore()
	{}
	virtual ~RPNFunMore()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int f1 = 0, f2 = 0;
		int resint;
		RPNInt *int1, *int2;
		RPNFloat *float1, *float2;
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2) 
		{
			float2 = dynamic_cast<RPNFloat*>(operand2);
			if (!float2)
				throw RPNExNotNumber(operand2);
			f2 = 1;
		}
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1)
		{
			float1 = dynamic_cast<RPNFloat*>(operand1);
			if (!float1)
				throw RPNExNotNumber(operand1);
			f1 = 1;
		}
		if (!f1 && !f2)
		{
			if (int1->Get() > int2->Get())
				resint = 1;
			else
				resint = 0;
		}
		else
		{
			if (f1 && !f2)
			{
				if (float1->Get() > int2->Get())
					resint = 1;
				else 
					resint = 0;
			}
			else if (!f1 && f2)
			{
				if (int1->Get() > float2->Get())
					resint = 1;
				else
					resint = 0;
			}
			else
			{
				if (float1->Get() > float2->Get())
					resint = 1;
				else 
					resint = 0;
			}
		}
		delete operand1;
		delete operand2;
		return new RPNInt(resint);
	}
	virtual void Print() const
	{
		printf(">");
	}
};

class RPNFunLess: public RPNFunction
{
public:
	RPNFunLess()
	{}
	virtual ~RPNFunLess()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int f1 = 0, f2 = 0;
		int resint;
		RPNInt *int1, *int2;
		RPNFloat *float1, *float2;
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2) 
		{
			float2 = dynamic_cast<RPNFloat*>(operand2);
			if (!float2)
				throw RPNExNotNumber(operand2);
			f2 = 1;
		}
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1)
		{
			float1 = dynamic_cast<RPNFloat*>(operand1);
			if (!float1)
				throw RPNExNotNumber(operand1);
			f1 = 1;
		}
		if (!f1 && !f2)
		{
			if (int1->Get() < int2->Get())
				resint = 1;
			else
				resint = 0;
		}
		else
		{
			if (f1 && !f2)
			{
				if (float1->Get() < int2->Get())
					resint = 1;
				else 
					resint = 0;
			}
			else if (!f1 && f2)
			{
				if (int1->Get() < float2->Get())
					resint = 1;
				else
					resint = 0;
			}
			else
			{
				if (float1->Get() < float2->Get())
					resint = 1;
				else 
					resint = 0;
			}
		}
		delete operand1;
		delete operand2;
		return new RPNInt(resint);
	}
	virtual void Print() const
	{
		printf("<");
	}
};

class RPNFunEq: public RPNFunction
{
public:
	RPNFunEq()
	{}
	virtual ~RPNFunEq()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int f1 = 0, f2 = 0;
		int resint;
		RPNInt *int1, *int2;
		RPNFloat *float1, *float2;
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2) 
		{
			
			float2 = dynamic_cast<RPNFloat*>(operand2);
			if (!float2)
				throw RPNExNotNumber(operand2);
			f2 = 1;
		}
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1)
		{
			float1 = dynamic_cast<RPNFloat*>(operand1);
			if (!float1)
				throw RPNExNotNumber(operand1);
			f1 = 1;
		}
		if (!f1 && !f2)
		{
			if (int1->Get() == int2->Get())
				resint = 1;
			else
				resint = 0;
		}
		else
		{
			if (f1 && !f2)
			{
				if (float1->Get() == int2->Get())
					resint = 1;
				else 
					resint = 0;
			}
			else if (!f1 && f2)
			{
				if (int1->Get() == float2->Get())
					resint = 1;
				else
					resint = 0;
			}
			else
			{
				if (float1->Get() == float2->Get())
					resint = 1;
				else 
					resint = 0;
			}
		}
		delete operand1;
		delete operand2;
		return new RPNInt(resint);
	}
	virtual void Print() const
	{
		printf("~");
	}
};

class RPNFunAnd: public RPNFunction
{
public:
	RPNFunAnd()
	{}
	virtual ~RPNFunAnd()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int resint;
		RPNInt *int1, *int2;
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2) 
			throw RPNExNotNumber(operand2);
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1)
			throw RPNExNotNumber(operand1);
		if (int1->Get() && int2->Get())
			resint = 1;
		else
			resint = 0;
		delete operand1;
		delete operand2;
		return new RPNInt(resint);
	}
	virtual void Print() const
	{
		printf("&");
	}
};

class RPNFunOr: public RPNFunction
{
public:
	RPNFunOr()
	{}
	virtual ~RPNFunOr()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int resint;
		RPNInt *int1, *int2;
		RPNElem *operand2 = Pop(stack);
		int2 = dynamic_cast<RPNInt*>(operand2);
		if (!int2) 
			throw RPNExNotNumber(operand2);
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1)
			throw RPNExNotNumber(operand1);
		if (int1->Get() || int2->Get())
			resint = 1;
		else
			resint = 0;
		delete operand1;
		delete operand2;
		return new RPNInt(resint);
	}
	virtual void Print() const
	{
		printf("|");
	}
};

class RPNFunNot: public RPNFunction
{
public:
	RPNFunNot()
	{}
	virtual ~RPNFunNot()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		int resint = 0;
		RPNInt *int1;
		RPNElem *operand1 = Pop(stack);
		int1 = dynamic_cast<RPNInt*>(operand1);
		if (!int1)
			throw RPNExNotNumber(operand1);
		if (int1->Get())
			resint = 1;;
		delete operand1;
		return new RPNInt(resint);
	}
	virtual void Print() const
	{
		printf("not");
	}
};

class RPNFunParOp: public RPNFunction
{
public:
	RPNFunParOp()
	{}
	virtual ~RPNFunParOp()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		return new RPNFunParOp();
	}
	virtual void Print() const
	{
		printf("(");
	}
};

class RPNFunParCl: public RPNFunction
{
public:
	RPNFunParCl()
	{}
	virtual ~RPNFunParCl()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		RPNItem *tail, *head;
		tail = head = 0;
		RPNElem *operand1 = Pop(stack);
		RPNFunParOp *po = dynamic_cast<RPNFunParOp*>(operand1);
		while (!po)
		{
			RPNItem *curr;
			curr = new RPNItem;
			curr->p = operand1;
			curr->next = 0;
			if (!head)
			{
				head = curr;
				tail = head;
				tail = tail->next;
			}
			else
			{
				tail->next = curr;
				tail = tail->next;
			}
			operand1 = Pop(stack);
			po = dynamic_cast<RPNFunParOp*>(operand1);
		}
		if (!head)
			throw RPNExNotNumber((*stack)->p);
		return head->p;
	}
	virtual void Print() const
	{
		printf(")");
	}
};

class RPNFunPrint: public RPNFunction
{
public:
	RPNFunPrint()
	{}
	virtual ~RPNFunPrint()
	{}
	RPNElem *EvaluateFun(RPNItem **stack, TVar **table) const
	{
		RPNString *s;
		RPNInt *i;
		RPNFloat *f;
		RPNElem *operand2 = Pop(stack);
		s = dynamic_cast<RPNString*>(operand2);
		if (!s) 
		{
			i = dynamic_cast<RPNInt*>(operand2);
			if (!i)
			{
				f = dynamic_cast<RPNFloat*>(operand2);
				if (!f)
					throw RPNExNotNumber(operand2);
				f->Print();
			}
			else
				i->Print();
		}
		else
			s->Print();
		delete operand2;
		return 0;
	}
	virtual void Print() const
	{
		printf("print");
	}
};

class RPNBranch: public RPNElem
{
public:
	RPNBranch()
	{}
	virtual ~RPNBranch()
	{}
	void Evaluate(RPNItem **stack, RPNItem **cur_cmd,
										TVar **table) const
	{
		RPNElem *operand1 = Pop(stack);
		RPNLabel *lab = dynamic_cast<RPNLabel *>(operand1);
		if (!lab)
			throw RPNExNotLabel(operand1);
		RPNItem *addr = lab->Get();
		*cur_cmd = addr;
		delete operand1;
	}
	virtual void Print() const
	{
		printf("!");
	}
};

class RPNCondBranch: public RPNElem
{
public:
	RPNCondBranch()
	{}
	virtual ~RPNCondBranch()
	{}
	void Evaluate(RPNItem **stack, RPNItem **cur_cmd,
										TVar **table) const
	{
		RPNElem *operand1 = Pop(stack);
		RPNLabel *lab = dynamic_cast<RPNLabel *>(operand1);
		if (!lab)
			throw RPNExNotLabel(operand1);
		RPNElem *operand2 = Pop(stack);
		RPNInt *i = dynamic_cast<RPNInt *>(operand2);
		if (!i)
			throw RPNExNotNumber(operand2);
		if (!i->Get())
		{
			RPNItem *addr = lab->Get();
			*cur_cmd = addr;
		}
		else
			*cur_cmd = (*cur_cmd)->next;
		delete operand1;
		delete operand2;
	}
	virtual void Print() const
	{
		printf("!f");
	}
};

const char *tw[] =
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
	"while",	//17
	0
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

void plwn(struct lexeme *l)
{
	while (l)
	{
		printf("%s - %d ", l->str, l->type);
		l = l->next;
	}
	putchar('\n');
}

void printlex(struct lexeme *l)
{
	int cstr = -1;
	while (l)
	{
		if (cstr != l->strnumb)
		{
			cstr = l->strnumb;
			printf("\n%d	", cstr);
		}
		printf("%s (%d) ", l->str, l->type);
		l = l->next;
	}
	putchar('\n');
	putchar('\n');
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
	if (lex)
		dellex(lex);
	if (nlex)
		dellex(nlex);
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
	if (EndLex() || cnd == constline)
	{
		Finish();
		lex->next = 0;
	}
	cnd = begin;
	finlex = complete;
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

class LexAnalysis
{
	Automat a;
	struct lexeme *lexhead, *lextail, *plex;
	int p;
public:
	struct lexeme *GetLexList();
	LexAnalysis();
};

LexAnalysis::LexAnalysis()
{
	p = 0;
	lexhead = lextail = plex = 0;
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
			if (plex->type == lex_error)
			{
				er = 1;
				break;
			}
			lexhead = newlex(plex, lexhead);
		}
	}
	if (er)
	{
		printlex(plex);
		return 0;
	}
	printlex(lexhead);
	return lexhead;
}

void delrpnlist(RPNItem *head)
{
	while (head)
	{
		RPNItem *tmp;
		tmp = head;
		head = head->next;
		delete tmp->p;
		delete tmp;
	}
}

void newrpn(RPNItem **head, RPNItem **tail, RPNItem *newelem)
{
	if (!(*head))
	{
		*head = newelem;
		*tail = *head;
	}
	else
	{
		(*tail)->next = newelem;
		(*tail) = (*tail)->next;
	}
}

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
		else if (tmptype == lex_more)
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

Executor::Executor(Syntax sa)
{
	curr = sa.GetRPNList();
	table = 0;
	stack = 0;
}

void Executor::Execute()
{
	while (curr)
		curr->p->Evaluate(&stack, &curr, &table);
}

void Executor::Print() const
{
	RPNItem *tmp;
	tmp = curr;
	putchar('\n');
	while (tmp)
	{
		tmp->p->Print();
		putchar(' ');
		tmp = tmp->next;
	}
	putchar('\n');
	putchar('\n');
}

int main (int argc, char **argv)
{
	struct lexeme *lexlist;
	LexAnalysis la;
	Syntax sa;

	if (argc != 2)
	{
		printf("Wrong number of arguments\n");
		return 1;
	}
	if (redirect(argv[1]) == -1)
		return 1;
	lexlist = la.GetLexList();
	if (!lexlist)
		return 1;
	try
	{
		sa.ScanLex(lexlist);
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

