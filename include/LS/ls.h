#ifndef _LSYSTEM_H_
#define _LSYSTEM_H

/* macros */
typedef unsigned char Symbol;
typedef Symbol Axiom;

/* struct declarators */
typedef struct _LSystem {
	size_t active;
	size_t axiomLength;
	Axiom *w;
} LSystem;

typedef struct _Sequence {
	Symbol symbol;
	struct _Sequence *next;
	struct _Sequence *prev;
} SequenceNode, Sequence;

/* type definitions */
typedef void (*PFnProductionRules)(SequenceNode **);

/* function prototypes */
void lsGenLSystem(LSystem *ls, size_t axiomLength, PFnProductionRules pfnProductionRules);
void lsGenSequence(LSystem *ls, unsigned int iterations, Sequence **seq);
unsigned char lsSequenceNodeSymbol(SequenceNode *node);
size_t lsSequenceSize(void);
void lsSequenceString(Sequence *seq, size_t sequenceSize, char *sequenceString);
void lsAddSymbol(SequenceNode **ppNode, unsigned char symbol);
void lsReplaceSymbol(SequenceNode **ppNode, unsigned char symbol);
void lsCreateSequence(Sequence **seq, size_t axiomLength, Symbol *vals);
void lsDestroySequence(Sequence **seq);
void lsDestroyLSystem(LSystem *ls);

#endif  // _LSYSTEM_H
