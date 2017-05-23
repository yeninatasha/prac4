#ifndef _RPN_H_
#define _RPN_H_

#include "lex.hpp"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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

void newvar(TVar **table, TVar *var);

TVar *findvar(TVar *head, RPNVar *var);

void setvar(TVar **table, RPNVar *var, RPNElem *n);

class RPNFunction: public RPNSimple
{
public:
	virtual RPNElem *EvaluateFun(RPNItem **stack,
							TVar **table) const = 0;
	virtual void SimpleEval(RPNItem **stack,
							TVar **table) const;
	virtual void Print() const = 0;
};

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


void delrpnlist(RPNItem *head);

void newrpn(RPNItem **head, RPNItem **tail, RPNItem *newelem);

#endif
