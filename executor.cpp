#include "executor.hpp"

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