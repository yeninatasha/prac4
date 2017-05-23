#include "rpn.hpp"

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

void RPNSimple::Evaluate(RPNItem **stack, RPNItem **cur_cmd, 
								TVar **table) const
{
	SimpleEval(stack, table);
	*cur_cmd = (*cur_cmd)->next;
}

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

void RPNFunction::SimpleEval(RPNItem **stack, TVar **table) const
{
	RPNElem *res = EvaluateFun(stack, table);
	if (res)
		Push(stack, res);
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