/* header files */
// standard headers
#include<stdio.h>

// malloc
#include<memory.h>
#include<malloc.h>

// L-System
#include"../../include/LS/ls.h"

/* file-restricted global variables */
static PFnProductionRules fnProductionRules = NULL;
static size_t sequenceSize = 0;

/* initialize the L-System */
void lsGenLSystem(LSystem *ls, size_t axiomLength, PFnProductionRules pfuncProductionRules)
{
	// code
	ls->axiomLength = axiomLength;
	ls->w = (Axiom *)malloc(sizeof(Axiom) * axiomLength);

	fnProductionRules = pfuncProductionRules;

	// activate L-System
	ls->active = 1;
}

unsigned int lsActiveLSystem(LSystem *ls)
{
	// code
	return ls->active ? 1 : 0;
}

/* apply production rules */
void lsGenSequence(LSystem *ls, unsigned int iterations, Sequence **seq)
{
	// variable declarations
	SequenceNode *head = NULL;
	SequenceNode *node = NULL;

	// code
	if (*seq == NULL)
		lsCreateSequence(seq, ls->axiomLength, (Symbol *)ls->w);  /* first, load the axiom */

	head = (SequenceNode *)(*seq);

	for (int i = 0; i < iterations; i++)
	{
		node = head;

		do
		{
			fnProductionRules(&node);
			node = node->next;

		} while (node != head);
	}
}

unsigned char lsSequenceNodeSymbol(SequenceNode *node)
{
	// code
	return (unsigned char)(node->symbol);
}

/* create a sequence from an L-System */
void lsCreateSequence(Sequence **seq, size_t axiomLength, Symbol *vals)
{
	// variable declarations
	SequenceNode *head = NULL;
	SequenceNode *curr = NULL;
	SequenceNode *next = NULL;

	// code
	sequenceSize = axiomLength;

	head = (SequenceNode *)malloc(sizeof(SequenceNode));
	head->next = head;
	head->prev = head;

	*seq = head;
	(*seq)->symbol = vals[0];

	//printf("axiom: %c", (*seq)->symbol);

	//if (axiomLength == 1)
	//{
	//	printf("\n");
	//	return;
	//}
	
	curr = *seq;

	for (int i = 1; i < axiomLength; i++)
	{
		next = (SequenceNode *)malloc(sizeof(SequenceNode));
		next->symbol = vals[i];

		curr->next = next;
		next->prev = curr;

		next->next = head;
		head->prev = next;

		curr = next;

		//printf("%c", curr->symbol);
	}

	//printf("\n");
}

void lsAddSymbol(SequenceNode **ppNode, unsigned char symbol)
{
	// variable declarations
	SequenceNode *newNode = NULL;

	// code
	newNode = (SequenceNode *)malloc(sizeof(SequenceNode));

	// insert
	newNode->symbol = (Symbol)symbol;
	
	newNode->next = (*ppNode)->next;
	(*ppNode)->next->prev = newNode;
	(*ppNode)->next = newNode;
	newNode->prev = *ppNode;

	// advance
	*ppNode = newNode;

	// adding to sequence size
	sequenceSize = sequenceSize + 1U;
}

void lsReplaceSymbol(SequenceNode **ppNode, unsigned char symbol)
{
	// code
	(*ppNode)->symbol = (Symbol)symbol;
}

size_t lsSequenceSize(void)
{
	// code
	return sequenceSize;
}

void lsSequenceString(Sequence *seq, size_t sequenceSize, char *sequenceString)
{
	// variable declarations
	SequenceNode *head = (SequenceNode *)seq;
	SequenceNode *node = head;
	int i = 0;

	// code
	do
	{
		sprintf(sequenceString + i, "%c", (unsigned char)node->symbol);
		node = node->next;
		i++;

	} while (i < sequenceSize && node != head);

	*(sequenceString + i) = '\0';
}

/* uninitialize a sequence */
void lsDestroySequence(Sequence **seq)
{
	// variable declarations
	SequenceNode *curr = NULL;
	SequenceNode *next = NULL;

	// code
	curr = *seq;
	
	while (curr != NULL)
	{
		curr->symbol = (Symbol)0;

		if (curr->next == curr)
		{
			next = NULL;
		}
		else
		{
			curr->prev->next = curr->next;
			curr->next->prev = curr->prev;

			next = curr->next;
		}

		curr->prev = NULL;
		curr->next = NULL;

		free(curr);

		curr = next;
	}

	*seq = NULL;
}

/* uninitialize an L-System */
void lsDestroyLSystem(LSystem *ls)
{
	// code
	// deactivate L-System
	ls->active = 0;
	
	if (ls->axiomLength == 1)
	{
		ls->w[0] = (Symbol)0;
	}
	else
	{
		for (int i = 0; i < ls->axiomLength; i++)
		{
			ls->w[i] = (Symbol)0;
		}
	}

	free(ls->w);
	ls->w = NULL;
	ls->axiomLength = 0;
}
