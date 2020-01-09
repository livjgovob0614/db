/*
 * regexp.c: generic and extensible Regular Expression engine
 *
 * Basically designed with the purpose of compiling regexps for
 * the variety of validation/shemas mechanisms now available in
 * XML related specifications these include:
 *  - XML-1.0 DTD validation
 *  - XML Schemas structure part 1
 *  - XML Schemas Datatypes part 2 especially Appendix F
 *  - RELAX-NG/TREX i.e. the counter proposal
 *
 * See Copyright for the status of this software.
 *
 * Daniel Veillard <veillard@redhat.com>
 */
#define IN_LIBXML
#include "libxml.h"
#pragma hdrstop

#ifdef LIBXML_REGEXP_ENABLED
// #define DEBUG_ERR
//#include <libxml/xmlunicode.h>
#ifndef INT_MAX
	#define INT_MAX 123456789 /* easy to flag and big enough for our needs */
#endif

/* #define DEBUG_REGEXP_GRAPH */
/* #define DEBUG_REGEXP_EXEC */
/* #define DEBUG_PUSH */
/* #define DEBUG_COMPACTION */

#define MAX_PUSH 10000000

#ifdef ERROR
	#undef ERROR
#endif
#define ERROR(str) ctxt->error = XML_REGEXP_COMPILE_ERROR; xmlRegexpErrCompile(ctxt, str);
#define NEXT ctxt->cur++
#define CUR (*(ctxt->cur))
#define NXT(index) (ctxt->cur[index])

#define CUR_SCHAR(s, l) xmlStringCurrentChar(NULL, s, &l)
#define NEXTL(l) ctxt->cur += l;
#define XML_REG_STRING_SEPARATOR '|'
/*
 * Need PREV to check on a '-' within a Character Group. May only be used
 * when it's guaranteed that cur is not at the beginning of ctxt->string!
 */
#define PREV (ctxt->cur[-1])

/**
 * @todo 
 *
 * macro to flag unimplemented blocks
 */
#define TODO xmlGenericError(0, "Unimplemented block at %s:%d\n", __FILE__, __LINE__);
//
// Datatypes and structures
//
/*
 * Note: the order of the enums below is significant, do not shuffle
 */
typedef enum {
	XML_REGEXP_EPSILON = 1,
	XML_REGEXP_CHARVAL,
	XML_REGEXP_RANGES,
	XML_REGEXP_SUBREG, /* used for () sub regexps */
	XML_REGEXP_STRING,
	XML_REGEXP_ANYCHAR, /* . */
	XML_REGEXP_ANYSPACE, /* \s */
	XML_REGEXP_NOTSPACE, /* \S */
	XML_REGEXP_INITNAME, /* \l */
	XML_REGEXP_NOTINITNAME, /* \L */
	XML_REGEXP_NAMECHAR, /* \c */
	XML_REGEXP_NOTNAMECHAR, /* \C */
	XML_REGEXP_DECIMAL, /* \d */
	XML_REGEXP_NOTDECIMAL, /* \D */
	XML_REGEXP_REALCHAR, /* \w */
	XML_REGEXP_NOTREALCHAR, /* \W */
	XML_REGEXP_LETTER = 100,
	XML_REGEXP_LETTER_UPPERCASE,
	XML_REGEXP_LETTER_LOWERCASE,
	XML_REGEXP_LETTER_TITLECASE,
	XML_REGEXP_LETTER_MODIFIER,
	XML_REGEXP_LETTER_OTHERS,
	XML_REGEXP_MARK,
	XML_REGEXP_MARK_NONSPACING,
	XML_REGEXP_MARK_SPACECOMBINING,
	XML_REGEXP_MARK_ENCLOSING,
	XML_REGEXP_NUMBER,
	XML_REGEXP_NUMBER_DECIMAL,
	XML_REGEXP_NUMBER_LETTER,
	XML_REGEXP_NUMBER_OTHERS,
	XML_REGEXP_PUNCT,
	XML_REGEXP_PUNCT_CONNECTOR,
	XML_REGEXP_PUNCT_DASH,
	XML_REGEXP_PUNCT_OPEN,
	XML_REGEXP_PUNCT_CLOSE,
	XML_REGEXP_PUNCT_INITQUOTE,
	XML_REGEXP_PUNCT_FINQUOTE,
	XML_REGEXP_PUNCT_OTHERS,
	XML_REGEXP_SEPAR,
	XML_REGEXP_SEPAR_SPACE,
	XML_REGEXP_SEPAR_LINE,
	XML_REGEXP_SEPAR_PARA,
	XML_REGEXP_SYMBOL,
	XML_REGEXP_SYMBOL_MATH,
	XML_REGEXP_SYMBOL_CURRENCY,
	XML_REGEXP_SYMBOL_MODIFIER,
	XML_REGEXP_SYMBOL_OTHERS,
	XML_REGEXP_OTHER,
	XML_REGEXP_OTHER_CONTROL,
	XML_REGEXP_OTHER_FORMAT,
	XML_REGEXP_OTHER_PRIVATE,
	XML_REGEXP_OTHER_NA,
	XML_REGEXP_BLOCK_NAME
} xmlRegAtomType;

typedef enum {
	XML_REGEXP_QUANT_EPSILON = 1,
	XML_REGEXP_QUANT_ONCE,
	XML_REGEXP_QUANT_OPT,
	XML_REGEXP_QUANT_MULT,
	XML_REGEXP_QUANT_PLUS,
	XML_REGEXP_QUANT_ONCEONLY,
	XML_REGEXP_QUANT_ALL,
	XML_REGEXP_QUANT_RANGE
} xmlRegQuantType;

typedef enum {
	XML_REGEXP_START_STATE = 1,
	XML_REGEXP_FINAL_STATE,
	XML_REGEXP_TRANS_STATE,
	XML_REGEXP_SINK_STATE,
	XML_REGEXP_UNREACH_STATE
} xmlRegStateType;

typedef enum {
	XML_REGEXP_MARK_NORMAL = 0,
	XML_REGEXP_MARK_START,
	XML_REGEXP_MARK_VISITED
} xmlRegMarkedType;

typedef struct _xmlRegRange xmlRegRange;
typedef xmlRegRange * xmlRegRangePtr;

struct _xmlRegRange {
	int neg;        /* 0 normal, 1 not, 2 exclude */
	xmlRegAtomType type;
	int start;
	int end;
	xmlChar * blockName;
};

typedef xmlAutomataState xmlRegState;
typedef xmlRegState * xmlRegStatePtr;

struct xmlRegAtom {
	int    no;
	xmlRegAtomType type;
	xmlRegQuantType quant;
	int    min;
	int    max;
	void * valuep;
	void * valuep2;
	int    neg;
	int    codepoint;
	xmlRegState * start;
	xmlRegState * start0;
	xmlRegState * stop;
	int    maxRanges;
	int    nbRanges;
	xmlRegRange ** ranges;
	void * data;
};

struct xmlRegCounter {
	int    min;
	int    max;
};

struct xmlRegTrans {
	xmlRegAtom * atom;
	int    to;
	int    counter;
	int    count;
	int    nd;
};

typedef xmlRegTrans * xmlRegTransPtr;

struct xmlAutomataState {
	xmlRegStateType type;
	xmlRegMarkedType mark;
	xmlRegMarkedType markd;
	xmlRegMarkedType reached;
	int no;
	int maxTrans;
	int nbTrans;
	xmlRegTrans * trans;
	/*  knowing states ponting to us can speed things up */
	int maxTransTo;
	int nbTransTo;
	int * transTo;
};

typedef struct xmlAutomata xmlRegParserCtxt;
//typedef xmlRegParserCtxt * xmlRegParserCtxtPtr;

#define AM_AUTOMATA_RNG 1

struct xmlAutomata {
	xmlChar * string;
	xmlChar * cur;
	int error;
	int neg;
	xmlRegStatePtr start;
	xmlRegStatePtr end;
	xmlRegStatePtr state;
	xmlRegAtom * atom;
	int maxAtoms;
	int nbAtoms;
	xmlRegAtom ** atoms;
	int maxStates;
	int nbStates;
	xmlRegStatePtr * states;
	int maxCounters;
	int nbCounters;
	xmlRegCounter * counters;
	int determinist;
	int negs;
	int flags;
};

struct xmlRegexp {
	xmlChar * string;
	int nbStates;
	xmlRegStatePtr * states;
	int nbAtoms;
	xmlRegAtom ** atoms;
	int nbCounters;
	xmlRegCounter * counters;
	int determinist;
	int flags;
	/*
	 * That's the compact form for determinists automatas
	 */
	int nbstates;
	int * compact;
	void ** transdata;
	int nbstrings;
	xmlChar ** stringMap;
};

typedef struct _xmlRegExecRollback xmlRegExecRollback;
typedef xmlRegExecRollback * xmlRegExecRollbackPtr;

struct _xmlRegExecRollback {
	xmlRegStatePtr state; /* the current state */
	int index;      /* the index in the input stack */
	int nextbranch; /* the next transition to explore in that state */
	int * counts;   /* save the automata state if it has some */
};

typedef struct _xmlRegInputToken xmlRegInputToken;
typedef xmlRegInputToken * xmlRegInputTokenPtr;

struct _xmlRegInputToken {
	xmlChar * value;
	void * data;
};

struct _xmlRegExecCtxt {
	int status;     /* execution status != 0 indicate an error */
	int determinist; /* did we find an indeterministic behaviour */
	xmlRegexp * comp; /* the compiled regexp */
	xmlRegExecCallbacks callback;
	void * data;
	xmlRegState * state; /* the current state */
	int transno;    /* the current transition on that state */
	int transcount; /* the number of chars in char counted transitions */
	/*
	 * A stack of rollback states
	 */
	int maxRollbacks;
	int nbRollbacks;
	xmlRegExecRollback * rollbacks;
	/*
	 * The state of the automata if any
	 */
	int * counts;
	/*
	 * The input stack
	 */
	int inputStackMax;
	int inputStackNr;
	int index;
	int * charStack;
	const xmlChar * inputString; /* when operating on characters */
	xmlRegInputTokenPtr inputStack; /* when operating on strings */
	/*
	 * error handling
	 */
	int errStateNo;         /* the error state number */
	xmlRegStatePtr errState; /* the error state */
	xmlChar * errString;    /* the string raising the error */
	int * errCounts;        /* counters at the error state */
	int nbPush;
};

#define REGEXP_ALL_COUNTER      0x123456
#define REGEXP_ALL_LAX_COUNTER  0x123457

static void xmlFAParseRegExp(xmlRegParserCtxt * ctxt, int top);
static void FASTCALL xmlRegFreeState(xmlRegStatePtr state);
static void FASTCALL xmlRegFreeAtom(xmlRegAtom * atom);
static int xmlRegStrEqualWildcard(const xmlChar * expStr, const xmlChar * valStr);
static int xmlRegCheckCharacter(xmlRegAtom * atom, int codepoint);
static int FASTCALL xmlRegCheckCharacterRange(xmlRegAtomType type, int codepoint, int neg, int start, int end, const xmlChar * blockName);

void xmlAutomataSetFlags(xmlAutomataPtr am, int flags);
//
// Regexp memory error handler
//
/**
 * xmlRegexpErrMemory:
 * @extra:  extra information
 *
 * Handle an out of memory condition
 */
static void FASTCALL xmlRegexpErrMemory(xmlRegParserCtxt * ctxt, const char * extra)
{
	const char * regexp = NULL;
	if(ctxt) {
		regexp = (const char *)ctxt->string;
		ctxt->error = XML_ERR_NO_MEMORY;
	}
	__xmlRaiseError(0, 0, 0, 0, 0, XML_FROM_REGEXP, XML_ERR_NO_MEMORY, XML_ERR_FATAL, NULL, 0, extra, regexp, NULL, 0, 0, "Memory allocation failed : %s\n", extra);
}
/**
 * xmlRegexpErrCompile:
 * @extra:  extra information
 *
 * Handle a compilation failure
 */
static void xmlRegexpErrCompile(xmlRegParserCtxt * ctxt, const char * extra)
{
	const char * regexp = NULL;
	int    idx = 0;
	if(ctxt) {
		regexp = (const char *)ctxt->string;
		idx = (int)(ctxt->cur - ctxt->string);
		ctxt->error = XML_REGEXP_COMPILE_ERROR;
	}
	__xmlRaiseError(0, 0, 0, 0, 0, XML_FROM_REGEXP, XML_REGEXP_COMPILE_ERROR, XML_ERR_FATAL, NULL, 0, extra,
	    regexp, NULL, idx, 0, "failed to compile: %s\n", extra);
}
//
// Allocation/Deallocation
//
static int xmlFAComputesDeterminism(xmlRegParserCtxt * ctxt);
/**
 * xmlRegEpxFromParse:
 * @ctxt:  the parser context used to build it
 *
 * Allocate a new regexp and fill it with the result from the parser
 *
 * Returns the new regexp or NULL in case of error
 */
static xmlRegexp * xmlRegEpxFromParse(xmlRegParserCtxt * ctxt)
{
	xmlRegexp * ret = static_cast<xmlRegexp *>(SAlloc::M(sizeof(xmlRegexp)));
	if(!ret) {
		xmlRegexpErrMemory(ctxt, "compiling regexp");
		return 0;
	}
	memzero(ret, sizeof(xmlRegexp));
	ret->string = ctxt->string;
	ret->nbStates = ctxt->nbStates;
	ret->states = ctxt->states;
	ret->nbAtoms = ctxt->nbAtoms;
	ret->atoms = ctxt->atoms;
	ret->nbCounters = ctxt->nbCounters;
	ret->counters = ctxt->counters;
	ret->determinist = ctxt->determinist;
	ret->flags = ctxt->flags;
	if(ret->determinist == -1) {
		xmlRegexpIsDeterminist(ret);
	}
	if(ret->determinist && (ret->nbCounters == 0) && (ctxt->negs == 0) && ret->atoms && ret->atoms[0] && (ret->atoms[0]->type == XML_REGEXP_STRING)) {
		int i, j, nbstates = 0, nbatoms = 0;
		int * stateRemap;
		int * stringRemap;
		int * transitions;
		void ** transdata;
		xmlChar ** stringMap;
		xmlChar * value;
		/*
		 * Switch to a compact representation
		 * 1/ counting the effective number of states left
		 * 2/ counting the unique number of atoms, and check that
		 *  they are all of the string type
		 * 3/ build a table state x atom for the transitions
		 */
		stateRemap = (int *)SAlloc::M(ret->nbStates * sizeof(int));
		if(stateRemap == NULL) {
			xmlRegexpErrMemory(ctxt, "compiling regexp");
			SAlloc::F(ret);
			return 0;
		}
		for(i = 0; i < ret->nbStates; i++) {
			if(ret->states[i]) {
				stateRemap[i] = nbstates;
				nbstates++;
			}
			else {
				stateRemap[i] = -1;
			}
		}
#ifdef DEBUG_COMPACTION
		printf("Final: %d states\n", nbstates);
#endif
		stringMap = static_cast<xmlChar **>(SAlloc::M(ret->nbAtoms * sizeof(char *)));
		if(stringMap == NULL) {
			xmlRegexpErrMemory(ctxt, "compiling regexp");
			SAlloc::F(stateRemap);
			SAlloc::F(ret);
			return 0;
		}
		stringRemap = static_cast<int *>(SAlloc::M(ret->nbAtoms * sizeof(int)));
		if(stringRemap == NULL) {
			xmlRegexpErrMemory(ctxt, "compiling regexp");
			SAlloc::F(stringMap);
			SAlloc::F(stateRemap);
			SAlloc::F(ret);
			return 0;
		}
		for(i = 0; i < ret->nbAtoms; i++) {
			if((ret->atoms[i]->type == XML_REGEXP_STRING) && (ret->atoms[i]->quant == XML_REGEXP_QUANT_ONCE)) {
				value = static_cast<xmlChar *>(ret->atoms[i]->valuep);
				for(j = 0; j < nbatoms; j++) {
					if(sstreq(stringMap[j], value)) {
						stringRemap[i] = j;
						break;
					}
				}
				if(j >= nbatoms) {
					stringRemap[i] = nbatoms;
					stringMap[nbatoms] = sstrdup(value);
					if(stringMap[nbatoms] == NULL) {
						for(i = 0; i < nbatoms; i++)
							SAlloc::F(stringMap[i]);
						SAlloc::F(stringRemap);
						SAlloc::F(stringMap);
						SAlloc::F(stateRemap);
						SAlloc::F(ret);
						return 0;
					}
					nbatoms++;
				}
			}
			else {
				SAlloc::F(stateRemap);
				SAlloc::F(stringRemap);
				for(i = 0; i < nbatoms; i++)
					SAlloc::F(stringMap[i]);
				SAlloc::F(stringMap);
				SAlloc::F(ret);
				return 0;
			}
		}
#ifdef DEBUG_COMPACTION
		printf("Final: %d atoms\n", nbatoms);
#endif
		transitions = static_cast<int *>(SAlloc::M((nbstates + 1) * (nbatoms + 1) * sizeof(int)));
		if(transitions == NULL) {
			SAlloc::F(stateRemap);
			SAlloc::F(stringRemap);
			SAlloc::F(stringMap);
			SAlloc::F(ret);
			return 0;
		}
		memzero(transitions, (nbstates + 1) * (nbatoms + 1) * sizeof(int));
		/*
		 * Allocate the transition table. The first entry for each state corresponds to the state type.
		 */
		transdata = NULL;
		for(i = 0; i < ret->nbStates; i++) {
			int atomno, targetno, prev;
			xmlRegStatePtr state;
			xmlRegTransPtr trans;
			int stateno = stateRemap[i];
			if(stateno == -1)
				continue;
			state = ret->states[i];
			transitions[stateno * (nbatoms + 1)] = state->type;
			for(j = 0; j < state->nbTrans; j++) {
				trans = &(state->trans[j]);
				if((trans->to == -1) || (trans->atom == NULL))
					continue;
				atomno = stringRemap[trans->atom->no];
				if(trans->atom->data && (transdata == NULL)) {
					transdata = static_cast<void **>(SAlloc::M(nbstates * nbatoms * sizeof(void *)));
					if(transdata)
						memzero(transdata, nbstates * nbatoms * sizeof(void *));
					else {
						xmlRegexpErrMemory(ctxt, "compiling regexp");
						break;
					}
				}
				targetno = stateRemap[trans->to];
				/*
				 * if the same atom can generate transitions to 2 different
				 * states then it means the automata is not determinist and
				 * the compact form can't be used !
				 */
				prev = transitions[stateno * (nbatoms + 1) + atomno + 1];
				if(prev != 0) {
					if(prev != targetno + 1) {
						ret->determinist = 0;
#ifdef DEBUG_COMPACTION
						printf("Indet: state %d trans %d, atom %d to %d : %d to %d\n", i, j, trans->atom->no, trans->to, atomno, targetno);
						printf("       previous to is %d\n", prev);
#endif
						SAlloc::F(transdata);
						SAlloc::F(transitions);
						SAlloc::F(stateRemap);
						SAlloc::F(stringRemap);
						for(i = 0; i < nbatoms; i++)
							SAlloc::F(stringMap[i]);
						SAlloc::F(stringMap);
						goto not_determ;
					}
				}
				else {
#if 0
					printf("State %d trans %d: atom %d to %d : %d to %d\n", i, j, trans->atom->no, trans->to, atomno, targetno);
#endif
					transitions[stateno * (nbatoms + 1) + atomno + 1] = targetno + 1; /* to avoid 0 */
					if(transdata)
						transdata[stateno * nbatoms + atomno] = trans->atom->data;
				}
			}
		}
		ret->determinist = 1;
#ifdef DEBUG_COMPACTION
		/*
		 * Debug
		 */
		for(i = 0; i < nbstates; i++) {
			for(j = 0; j < nbatoms + 1; j++) {
				printf("%02d ", transitions[i * (nbatoms + 1) + j]);
			}
			printf("\n");
		}
		printf("\n");
#endif
		/*
		 * Cleanup of the old data
		 */
		if(ret->states) {
			for(i = 0; i < ret->nbStates; i++)
				xmlRegFreeState(ret->states[i]);
			SAlloc::F(ret->states);
		}
		ret->states = NULL;
		ret->nbStates = 0;
		if(ret->atoms) {
			for(i = 0; i < ret->nbAtoms; i++)
				xmlRegFreeAtom(ret->atoms[i]);
			SAlloc::F(ret->atoms);
		}
		ret->atoms = NULL;
		ret->nbAtoms = 0;
		ret->compact = transitions;
		ret->transdata = transdata;
		ret->stringMap = stringMap;
		ret->nbstrings = nbatoms;
		ret->nbstates = nbstates;
		SAlloc::F(stateRemap);
		SAlloc::F(stringRemap);
	}
not_determ:
	ctxt->string = NULL;
	ctxt->nbStates = 0;
	ctxt->states = NULL;
	ctxt->nbAtoms = 0;
	ctxt->atoms = NULL;
	ctxt->nbCounters = 0;
	ctxt->counters = NULL;
	return ret;
}
/**
 * xmlRegNewParserCtxt:
 * @string:  the string to parse
 *
 * Allocate a new regexp parser context
 *
 * Returns the new context or NULL in case of error
 */
static xmlRegParserCtxt * xmlRegNewParserCtxt(const xmlChar * string)
{
	xmlRegParserCtxt * ret = static_cast<xmlRegParserCtxt *>(SAlloc::M(sizeof(xmlRegParserCtxt)));
	if(ret) {
		memzero(ret, sizeof(xmlRegParserCtxt));
		ret->string = sstrdup(string);
		ret->cur = ret->string;
		ret->neg = 0;
		ret->negs = 0;
		ret->error = 0;
		ret->determinist = -1;
	}
	return ret;
}
/**
 * xmlRegNewRange:
 * @ctxt:  the regexp parser context
 * @neg:  is that negative
 * @type:  the type of range
 * @start:  the start codepoint
 * @end:  the end codepoint
 *
 * Allocate a new regexp range
 *
 * Returns the new range or NULL in case of error
 */
static xmlRegRangePtr xmlRegNewRange(xmlRegParserCtxt * ctxt, int neg, xmlRegAtomType type, int start, int end)
{
	xmlRegRangePtr ret = static_cast<xmlRegRange *>(SAlloc::M(sizeof(xmlRegRange)));
	if(!ret) {
		xmlRegexpErrMemory(ctxt, "allocating range");
	}
	else {
		ret->neg = neg;
		ret->type = type;
		ret->start = start;
		ret->end = end;
	}
	return ret;
}
/**
 * xmlRegFreeRange:
 * @range:  the regexp range
 *
 * Free a regexp range
 */
static void FASTCALL xmlRegFreeRange(xmlRegRangePtr range)
{
	if(range) {
		SAlloc::F(range->blockName);
		SAlloc::F(range);
	}
}
/**
 * xmlRegCopyRange:
 * @range:  the regexp range
 *
 * Copy a regexp range
 *
 * Returns the new copy or NULL in case of error.
 */
static xmlRegRangePtr xmlRegCopyRange(xmlRegParserCtxt * ctxt, xmlRegRangePtr range) 
{
	xmlRegRangePtr ret;
	if(range == NULL)
		return 0;
	ret = xmlRegNewRange(ctxt, range->neg, range->type, range->start, range->end);
	if(!ret)
		return 0;
	if(range->blockName) {
		ret->blockName = sstrdup(range->blockName);
		if(ret->blockName == NULL) {
			xmlRegexpErrMemory(ctxt, "allocating range");
			xmlRegFreeRange(ret);
			return 0;
		}
	}
	return ret;
}
/**
 * xmlRegNewAtom:
 * @ctxt:  the regexp parser context
 * @type:  the type of atom
 *
 * Allocate a new atom
 *
 * Returns the new atom or NULL in case of error
 */
static xmlRegAtom * FASTCALL xmlRegNewAtom(xmlRegParserCtxt * ctxt, xmlRegAtomType type)
{
	xmlRegAtom * ret = static_cast<xmlRegAtom *>(SAlloc::M(sizeof(xmlRegAtom)));
	if(!ret) {
		xmlRegexpErrMemory(ctxt, "allocating atom");
	}
	else {
		memzero(ret, sizeof(xmlRegAtom));
		ret->type = type;
		ret->quant = XML_REGEXP_QUANT_ONCE;
		ret->min = 0;
		ret->max = 0;
	}
	return ret;
}
/**
 * xmlRegFreeAtom:
 * @atom:  the regexp atom
 *
 * Free a regexp atom
 */
static void FASTCALL xmlRegFreeAtom(xmlRegAtom * atom)
{
	if(atom) {
		for(int i = 0; i < atom->nbRanges; i++)
			xmlRegFreeRange(atom->ranges[i]);
		SAlloc::F(atom->ranges);
		if(atom->type == XML_REGEXP_STRING)
			SAlloc::F(atom->valuep);
		if(atom->type == XML_REGEXP_STRING)
			SAlloc::F(atom->valuep2);
		if(atom->type == XML_REGEXP_BLOCK_NAME)
			SAlloc::F(atom->valuep);
		SAlloc::F(atom);
	}
}
/**
 * xmlRegCopyAtom:
 * @ctxt:  the regexp parser context
 * @atom:  the oiginal atom
 *
 * Allocate a new regexp range
 *
 * Returns the new atom or NULL in case of error
 */
static xmlRegAtom * xmlRegCopyAtom(xmlRegParserCtxt * ctxt, xmlRegAtom * atom)
{
	xmlRegAtom * ret = static_cast<xmlRegAtom *>(SAlloc::M(sizeof(xmlRegAtom)));
	if(!ret) {
		xmlRegexpErrMemory(ctxt, "copying atom");
		return 0;
	}
	memzero(ret, sizeof(xmlRegAtom));
	ret->type = atom->type;
	ret->quant = atom->quant;
	ret->min = atom->min;
	ret->max = atom->max;
	if(atom->nbRanges > 0) {
		int i;
		ret->ranges = static_cast<xmlRegRange **>(SAlloc::M(sizeof(xmlRegRange *) * atom->nbRanges));
		if(ret->ranges == NULL) {
			xmlRegexpErrMemory(ctxt, "copying atom");
			goto error;
		}
		for(i = 0; i < atom->nbRanges; i++) {
			ret->ranges[i] = xmlRegCopyRange(ctxt, atom->ranges[i]);
			if(ret->ranges[i] == NULL)
				goto error;
			ret->nbRanges = i + 1;
		}
	}
	return ret;
error:
	xmlRegFreeAtom(ret);
	return 0;
}

static xmlRegStatePtr FASTCALL xmlRegNewState(xmlRegParserCtxt * ctxt)
{
	xmlRegStatePtr ret = static_cast<xmlRegStatePtr>(SAlloc::M(sizeof(xmlRegState)));
	if(!ret) {
		xmlRegexpErrMemory(ctxt, "allocating state");
	}
	else {
		memzero(ret, sizeof(xmlRegState));
		ret->type = XML_REGEXP_TRANS_STATE;
		ret->mark = XML_REGEXP_MARK_NORMAL;
	}
	return ret;
}
/**
 * xmlRegFreeState:
 * @state:  the regexp state
 *
 * Free a regexp state
 */
static void FASTCALL xmlRegFreeState(xmlRegStatePtr state)
{
	if(state) {
		SAlloc::F(state->trans);
		SAlloc::F(state->transTo);
		SAlloc::F(state);
	}
}
/**
 * xmlRegFreeParserCtxt:
 * @ctxt:  the regexp parser context
 *
 * Free a regexp parser context
 */
static void xmlRegFreeParserCtxt(xmlRegParserCtxt * ctxt)
{
	if(ctxt) {
		SAlloc::F(ctxt->string);
		if(ctxt->states) {
			for(int i = 0; i < ctxt->nbStates; i++)
				xmlRegFreeState(ctxt->states[i]);
			SAlloc::F(ctxt->states);
		}
		if(ctxt->atoms) {
			for(int i = 0; i < ctxt->nbAtoms; i++)
				xmlRegFreeAtom(ctxt->atoms[i]);
			SAlloc::F(ctxt->atoms);
		}
		SAlloc::F(ctxt->counters);
		SAlloc::F(ctxt);
	}
}
//
// Display of Data structures
//
static void xmlRegPrintAtomType(FILE * output, xmlRegAtomType type)
{
	switch(type) {
		case XML_REGEXP_EPSILON:             fprintf(output, "epsilon "); break;
		case XML_REGEXP_CHARVAL:             fprintf(output, "charval "); break;
		case XML_REGEXP_RANGES:              fprintf(output, "ranges "); break;
		case XML_REGEXP_SUBREG:              fprintf(output, "subexpr "); break;
		case XML_REGEXP_STRING:              fprintf(output, "string "); break;
		case XML_REGEXP_ANYCHAR:             fprintf(output, "anychar "); break;
		case XML_REGEXP_ANYSPACE:            fprintf(output, "anyspace "); break;
		case XML_REGEXP_NOTSPACE:            fprintf(output, "notspace "); break;
		case XML_REGEXP_INITNAME:            fprintf(output, "initname "); break;
		case XML_REGEXP_NOTINITNAME:         fprintf(output, "notinitname "); break;
		case XML_REGEXP_NAMECHAR:            fprintf(output, "namechar "); break;
		case XML_REGEXP_NOTNAMECHAR:         fprintf(output, "notnamechar "); break;
		case XML_REGEXP_DECIMAL:             fprintf(output, "decimal "); break;
		case XML_REGEXP_NOTDECIMAL:          fprintf(output, "notdecimal "); break;
		case XML_REGEXP_REALCHAR:            fprintf(output, "realchar "); break;
		case XML_REGEXP_NOTREALCHAR:         fprintf(output, "notrealchar "); break;
		case XML_REGEXP_LETTER:              fprintf(output, "LETTER "); break;
		case XML_REGEXP_LETTER_UPPERCASE:    fprintf(output, "LETTER_UPPERCASE "); break;
		case XML_REGEXP_LETTER_LOWERCASE:    fprintf(output, "LETTER_LOWERCASE "); break;
		case XML_REGEXP_LETTER_TITLECASE:    fprintf(output, "LETTER_TITLECASE "); break;
		case XML_REGEXP_LETTER_MODIFIER:     fprintf(output, "LETTER_MODIFIER "); break;
		case XML_REGEXP_LETTER_OTHERS:       fprintf(output, "LETTER_OTHERS "); break;
		case XML_REGEXP_MARK:                fprintf(output, "MARK "); break;
		case XML_REGEXP_MARK_NONSPACING:     fprintf(output, "MARK_NONSPACING "); break;
		case XML_REGEXP_MARK_SPACECOMBINING: fprintf(output, "MARK_SPACECOMBINING "); break;
		case XML_REGEXP_MARK_ENCLOSING:      fprintf(output, "MARK_ENCLOSING "); break;
		case XML_REGEXP_NUMBER:              fprintf(output, "NUMBER "); break;
		case XML_REGEXP_NUMBER_DECIMAL:  fprintf(output, "NUMBER_DECIMAL "); break;
		case XML_REGEXP_NUMBER_LETTER:   fprintf(output, "NUMBER_LETTER "); break;
		case XML_REGEXP_NUMBER_OTHERS:   fprintf(output, "NUMBER_OTHERS "); break;
		case XML_REGEXP_PUNCT:           fprintf(output, "PUNCT "); break;
		case XML_REGEXP_PUNCT_CONNECTOR: fprintf(output, "PUNCT_CONNECTOR "); break;
		case XML_REGEXP_PUNCT_DASH:      fprintf(output, "PUNCT_DASH "); break;
		case XML_REGEXP_PUNCT_OPEN:      fprintf(output, "PUNCT_OPEN "); break;
		case XML_REGEXP_PUNCT_CLOSE:     fprintf(output, "PUNCT_CLOSE "); break;
		case XML_REGEXP_PUNCT_INITQUOTE: fprintf(output, "PUNCT_INITQUOTE "); break;
		case XML_REGEXP_PUNCT_FINQUOTE:  fprintf(output, "PUNCT_FINQUOTE "); break;
		case XML_REGEXP_PUNCT_OTHERS:    fprintf(output, "PUNCT_OTHERS "); break;
		case XML_REGEXP_SEPAR:           fprintf(output, "SEPAR "); break;
		case XML_REGEXP_SEPAR_SPACE:     fprintf(output, "SEPAR_SPACE "); break;
		case XML_REGEXP_SEPAR_LINE:      fprintf(output, "SEPAR_LINE "); break;
		case XML_REGEXP_SEPAR_PARA:      fprintf(output, "SEPAR_PARA "); break;
		case XML_REGEXP_SYMBOL:          fprintf(output, "SYMBOL "); break;
		case XML_REGEXP_SYMBOL_MATH:     fprintf(output, "SYMBOL_MATH "); break;
		case XML_REGEXP_SYMBOL_CURRENCY: fprintf(output, "SYMBOL_CURRENCY "); break;
		case XML_REGEXP_SYMBOL_MODIFIER: fprintf(output, "SYMBOL_MODIFIER "); break;
		case XML_REGEXP_SYMBOL_OTHERS:   fprintf(output, "SYMBOL_OTHERS "); break;
		case XML_REGEXP_OTHER:           fprintf(output, "OTHER "); break;
		case XML_REGEXP_OTHER_CONTROL:   fprintf(output, "OTHER_CONTROL "); break;
		case XML_REGEXP_OTHER_FORMAT:    fprintf(output, "OTHER_FORMAT "); break;
		case XML_REGEXP_OTHER_PRIVATE:   fprintf(output, "OTHER_PRIVATE "); break;
		case XML_REGEXP_OTHER_NA:        fprintf(output, "OTHER_NA "); break;
		case XML_REGEXP_BLOCK_NAME:      fprintf(output, "BLOCK "); break;
	}
}

static void xmlRegPrintQuantType(FILE * output, xmlRegQuantType type)
{
	switch(type) {
		case XML_REGEXP_QUANT_EPSILON: fprintf(output, "epsilon "); break;
		case XML_REGEXP_QUANT_ONCE: fprintf(output, "once "); break;
		case XML_REGEXP_QUANT_OPT: fprintf(output, "? "); break;
		case XML_REGEXP_QUANT_MULT: fprintf(output, "* "); break;
		case XML_REGEXP_QUANT_PLUS: fprintf(output, "+ "); break;
		case XML_REGEXP_QUANT_RANGE: fprintf(output, "range "); break;
		case XML_REGEXP_QUANT_ONCEONLY: fprintf(output, "onceonly "); break;
		case XML_REGEXP_QUANT_ALL: fprintf(output, "all "); break;
	}
}

static void xmlRegPrintRange(FILE * output, xmlRegRangePtr range)
{
	fprintf(output, "  range: ");
	if(range->neg)
		fprintf(output, "negative ");
	xmlRegPrintAtomType(output, range->type);
	fprintf(output, "%c - %c\n", range->start, range->end);
}

static void xmlRegPrintAtom(FILE * output, xmlRegAtom * atom)
{
	fprintf(output, " atom: ");
	if(atom == NULL) {
		fprintf(output, "NULL\n");
	}
	else {
		if(atom->neg)
			fprintf(output, "not ");
		xmlRegPrintAtomType(output, atom->type);
		xmlRegPrintQuantType(output, atom->quant);
		if(atom->quant == XML_REGEXP_QUANT_RANGE)
			fprintf(output, "%d-%d ", atom->min, atom->max);
		if(atom->type == XML_REGEXP_STRING)
			fprintf(output, "'%s' ", (char *)atom->valuep);
		if(atom->type == XML_REGEXP_CHARVAL)
			fprintf(output, "char %c\n", atom->codepoint);
		else if(atom->type == XML_REGEXP_RANGES) {
			fprintf(output, "%d entries\n", atom->nbRanges);
			for(int i = 0; i < atom->nbRanges; i++)
				xmlRegPrintRange(output, atom->ranges[i]);
		}
		else if(atom->type == XML_REGEXP_SUBREG) {
			fprintf(output, "start %d end %d\n", atom->start->no, atom->stop->no);
		}
		else {
			fprintf(output, "\n");
		}
	}
}

static void xmlRegPrintTrans(FILE * output, xmlRegTransPtr trans)
{
	fprintf(output, "  trans: ");
	if(trans == NULL) {
		fprintf(output, "NULL\n");
		return;
	}
	if(trans->to < 0) {
		fprintf(output, "removed\n");
		return;
	}
	if(trans->nd != 0) {
		if(trans->nd == 2)
			fprintf(output, "last not determinist, ");
		else
			fprintf(output, "not determinist, ");
	}
	if(trans->counter >= 0) {
		fprintf(output, "counted %d, ", trans->counter);
	}
	if(trans->count == REGEXP_ALL_COUNTER) {
		fprintf(output, "all transition, ");
	}
	else if(trans->count >= 0) {
		fprintf(output, "count based %d, ", trans->count);
	}
	if(trans->atom == NULL) {
		fprintf(output, "epsilon to %d\n", trans->to);
		return;
	}
	if(trans->atom->type == XML_REGEXP_CHARVAL)
		fprintf(output, "char %c ", trans->atom->codepoint);
	fprintf(output, "atom %d, to %d\n", trans->atom->no, trans->to);
}

static void xmlRegPrintState(FILE * output, xmlRegStatePtr state)
{
	int i;
	fprintf(output, " state: ");
	if(state == NULL) {
		fprintf(output, "NULL\n");
		return;
	}
	if(state->type == XML_REGEXP_START_STATE)
		fprintf(output, "START ");
	if(state->type == XML_REGEXP_FINAL_STATE)
		fprintf(output, "FINAL ");

	fprintf(output, "%d, %d transitions:\n", state->no, state->nbTrans);
	for(i = 0; i < state->nbTrans; i++) {
		xmlRegPrintTrans(output, &(state->trans[i]));
	}
}

#ifdef DEBUG_REGEXP_GRAPH
static void xmlRegPrintCtxt(FILE * output, xmlRegParserCtxt * ctxt)
{
	int i;
	fprintf(output, " ctxt: ");
	if(!ctxt) {
		fprintf(output, "NULL\n");
		return;
	}
	fprintf(output, "'%s' ", ctxt->string);
	if(ctxt->error)
		fprintf(output, "error ");
	if(ctxt->neg)
		fprintf(output, "neg ");
	fprintf(output, "\n");
	fprintf(output, "%d atoms:\n", ctxt->nbAtoms);
	for(i = 0; i < ctxt->nbAtoms; i++) {
		fprintf(output, " %02d ", i);
		xmlRegPrintAtom(output, ctxt->atoms[i]);
	}
	if(ctxt->atom) {
		fprintf(output, "current atom:\n");
		xmlRegPrintAtom(output, ctxt->atom);
	}
	fprintf(output, "%d states:", ctxt->nbStates);
	if(ctxt->start)
		fprintf(output, " start: %d", ctxt->start->no);
	if(ctxt->end)
		fprintf(output, " end: %d", ctxt->end->no);
	fprintf(output, "\n");
	for(i = 0; i < ctxt->nbStates; i++) {
		xmlRegPrintState(output, ctxt->states[i]);
	}
	fprintf(output, "%d counters:\n", ctxt->nbCounters);
	for(i = 0; i < ctxt->nbCounters; i++) {
		fprintf(output, " %d: min %d max %d\n", i, ctxt->counters[i].min, ctxt->counters[i].max);
	}
}
#endif
// 
// Finite Automata structures manipulations
// 
static void FASTCALL xmlRegAtomAddRange(xmlRegParserCtxt * ctxt, xmlRegAtom * atom, int neg, xmlRegAtomType type, int start, int end, xmlChar * blockName)
{
	xmlRegRange * range;
	if(atom == NULL) {
		ERROR("add range: atom is NULL");
		return;
	}
	if(atom->type != XML_REGEXP_RANGES) {
		ERROR("add range: atom is not ranges");
		return;
	}
	if(atom->maxRanges == 0) {
		atom->maxRanges = 4;
		atom->ranges = static_cast<xmlRegRange **>(SAlloc::M(atom->maxRanges * sizeof(xmlRegRange *)));
		if(atom->ranges == NULL) {
			xmlRegexpErrMemory(ctxt, "adding ranges");
			atom->maxRanges = 0;
			return;
		}
	}
	else if(atom->nbRanges >= atom->maxRanges) {
		atom->maxRanges *= 2;
		xmlRegRangePtr * tmp = static_cast<xmlRegRange **>(SAlloc::R(atom->ranges, atom->maxRanges * sizeof(xmlRegRange *)));
		if(!tmp) {
			xmlRegexpErrMemory(ctxt, "adding ranges");
			atom->maxRanges /= 2;
			return;
		}
		atom->ranges = tmp;
	}
	range = xmlRegNewRange(ctxt, neg, type, start, end);
	if(range) {
		range->blockName = blockName;
		atom->ranges[atom->nbRanges++] = range;
	}
}

static int FASTCALL xmlRegGetCounter(xmlRegParserCtxt * ctxt)
{
	if(ctxt->maxCounters == 0) {
		ctxt->maxCounters = 4;
		ctxt->counters = static_cast<xmlRegCounter *>(SAlloc::M(ctxt->maxCounters * sizeof(xmlRegCounter)));
		if(ctxt->counters == NULL) {
			xmlRegexpErrMemory(ctxt, "allocating counter");
			ctxt->maxCounters = 0;
			return -1;
		}
	}
	else if(ctxt->nbCounters >= ctxt->maxCounters) {
		ctxt->maxCounters *= 2;
		xmlRegCounter * tmp = static_cast<xmlRegCounter *>(SAlloc::R(ctxt->counters, ctxt->maxCounters * sizeof(xmlRegCounter)));
		if(!tmp) {
			xmlRegexpErrMemory(ctxt, "allocating counter");
			ctxt->maxCounters /= 2;
			return -1;
		}
		ctxt->counters = tmp;
	}
	ctxt->counters[ctxt->nbCounters].min = -1;
	ctxt->counters[ctxt->nbCounters].max = -1;
	return (ctxt->nbCounters++);
}

static int FASTCALL xmlRegAtomPush(xmlRegParserCtxt * ctxt, xmlRegAtom * atom)
{
	if(atom == NULL) {
		ERROR("atom push: atom is NULL");
		return -1;
	}
	if(ctxt->maxAtoms == 0) {
		ctxt->maxAtoms = 4;
		ctxt->atoms = static_cast<xmlRegAtom **>(SAlloc::M(ctxt->maxAtoms * sizeof(xmlRegAtom *)));
		if(ctxt->atoms == NULL) {
			xmlRegexpErrMemory(ctxt, "pushing atom");
			ctxt->maxAtoms = 0;
			return -1;
		}
	}
	else if(ctxt->nbAtoms >= ctxt->maxAtoms) {
		xmlRegAtom ** tmp;
		ctxt->maxAtoms *= 2;
		tmp = static_cast<xmlRegAtom **>(SAlloc::R(ctxt->atoms, ctxt->maxAtoms * sizeof(xmlRegAtom *)));
		if(!tmp) {
			xmlRegexpErrMemory(ctxt, "allocating counter");
			ctxt->maxAtoms /= 2;
			return -1;
		}
		ctxt->atoms = tmp;
	}
	atom->no = ctxt->nbAtoms;
	ctxt->atoms[ctxt->nbAtoms++] = atom;
	return 0;
}

static void xmlRegStateAddTransTo(xmlRegParserCtxt * ctxt, xmlRegState * target, int from)
{
	if(target->maxTransTo == 0) {
		target->maxTransTo = 8;
		target->transTo = static_cast<int *>(SAlloc::M(target->maxTransTo * sizeof(int)));
		if(target->transTo == NULL) {
			xmlRegexpErrMemory(ctxt, "adding transition");
			target->maxTransTo = 0;
			return;
		}
	}
	else if(target->nbTransTo >= target->maxTransTo) {
		target->maxTransTo *= 2;
		int * tmp = static_cast<int *>(SAlloc::R(target->transTo, target->maxTransTo * sizeof(int)));
		if(!tmp) {
			xmlRegexpErrMemory(ctxt, "adding transition");
			target->maxTransTo /= 2;
			return;
		}
		target->transTo = tmp;
	}
	target->transTo[target->nbTransTo] = from;
	target->nbTransTo++;
}

static void FASTCALL xmlRegStateAddTrans(xmlRegParserCtxt * ctxt, xmlRegStatePtr state, xmlRegAtom * atom, xmlRegStatePtr target, int counter, int count)
{
	int nrtrans;
	if(state == NULL) {
		ERROR("add state: state is NULL");
		return;
	}
	if(target == NULL) {
		ERROR("add state: target is NULL");
		return;
	}
	/*
	 * Other routines follow the philosophy 'When in doubt, add a transition'
	 * so we check here whether such a transition is already present and, if
	 * so, silently ignore this request.
	 */
	for(nrtrans = state->nbTrans - 1; nrtrans >= 0; nrtrans--) {
		xmlRegTransPtr trans = &(state->trans[nrtrans]);
		if((trans->atom == atom) && (trans->to == target->no) && (trans->counter == counter) && (trans->count == count)) {
#ifdef DEBUG_REGEXP_GRAPH
			printf("Ignoring duplicate transition from %d to %d\n", state->no, target->no);
#endif
			return;
		}
	}
	if(state->maxTrans == 0) {
		state->maxTrans = 8;
		state->trans = (xmlRegTrans*)SAlloc::M(state->maxTrans * sizeof(xmlRegTrans));
		if(state->trans == NULL) {
			xmlRegexpErrMemory(ctxt, "adding transition");
			state->maxTrans = 0;
			return;
		}
	}
	else if(state->nbTrans >= state->maxTrans) {
		state->maxTrans *= 2;
		xmlRegTrans * tmp = static_cast<xmlRegTrans *>(SAlloc::R(state->trans, state->maxTrans * sizeof(xmlRegTrans)));
		if(!tmp) {
			xmlRegexpErrMemory(ctxt, "adding transition");
			state->maxTrans /= 2;
			return;
		}
		state->trans = tmp;
	}
#ifdef DEBUG_REGEXP_GRAPH
	printf("Add trans from %d to %d ", state->no, target->no);
	if(count == REGEXP_ALL_COUNTER)
		printf("all transition\n");
	else if(count >= 0)
		printf("count based %d\n", count);
	else if(counter >= 0)
		printf("counted %d\n", counter);
	else if(atom == NULL)
		printf("epsilon transition\n");
	else if(atom)
		xmlRegPrintAtom(stdout, atom);
#endif
	state->trans[state->nbTrans].atom = atom;
	state->trans[state->nbTrans].to = target->no;
	state->trans[state->nbTrans].counter = counter;
	state->trans[state->nbTrans].count = count;
	state->trans[state->nbTrans].nd = 0;
	state->nbTrans++;
	xmlRegStateAddTransTo(ctxt, target, state->no);
}

static int FASTCALL xmlRegStatePush(xmlRegParserCtxt * ctxt, xmlRegStatePtr state)
{
	if(state == NULL)
		return -1;
	if(ctxt->maxStates == 0) {
		ctxt->maxStates = 4;
		ctxt->states = static_cast<xmlRegStatePtr *>(SAlloc::M(ctxt->maxStates * sizeof(xmlRegStatePtr)));
		if(ctxt->states == NULL) {
			xmlRegexpErrMemory(ctxt, "adding state");
			ctxt->maxStates = 0;
			return -1;
		}
	}
	else if(ctxt->nbStates >= ctxt->maxStates) {
		ctxt->maxStates *= 2;
		xmlRegStatePtr * tmp = static_cast<xmlRegStatePtr *>(SAlloc::R(ctxt->states, ctxt->maxStates * sizeof(xmlRegStatePtr)));
		if(!tmp) {
			xmlRegexpErrMemory(ctxt, "adding state");
			ctxt->maxStates /= 2;
			return -1;
		}
		ctxt->states = tmp;
	}
	state->no = ctxt->nbStates;
	ctxt->states[ctxt->nbStates++] = state;
	return 0;
}
/**
 * xmlFAGenerateAllTransition:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 * @lax:
 *
 */
static void xmlFAGenerateAllTransition(xmlRegParserCtxt * ctxt, xmlRegStatePtr from, xmlRegStatePtr to, int lax)
{
	if(to == NULL) {
		to = xmlRegNewState(ctxt);
		xmlRegStatePush(ctxt, to);
		ctxt->state = to;
	}
	if(lax)
		xmlRegStateAddTrans(ctxt, from, NULL, to, -1, REGEXP_ALL_LAX_COUNTER);
	else
		xmlRegStateAddTrans(ctxt, from, NULL, to, -1, REGEXP_ALL_COUNTER);
}
/**
 * xmlFAGenerateEpsilonTransition:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 *
 */
static void FASTCALL xmlFAGenerateEpsilonTransition(xmlRegParserCtxt * ctxt, xmlRegStatePtr from, xmlRegStatePtr to)
{
	if(to == NULL) {
		to = xmlRegNewState(ctxt);
		xmlRegStatePush(ctxt, to);
		ctxt->state = to;
	}
	xmlRegStateAddTrans(ctxt, from, NULL, to, -1, -1);
}
/**
 * xmlFAGenerateCountedEpsilonTransition:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 * counter:  the counter for that transition
 *
 */
static void FASTCALL xmlFAGenerateCountedEpsilonTransition(xmlRegParserCtxt * ctxt, xmlRegStatePtr from, xmlRegStatePtr to, int counter) 
{
	if(to == NULL) {
		to = xmlRegNewState(ctxt);
		xmlRegStatePush(ctxt, to);
		ctxt->state = to;
	}
	xmlRegStateAddTrans(ctxt, from, NULL, to, counter, -1);
}
/**
 * xmlFAGenerateCountedTransition:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 * counter:  the counter for that transition
 *
 */
static void FASTCALL xmlFAGenerateCountedTransition(xmlRegParserCtxt * ctxt, xmlRegStatePtr from, xmlRegStatePtr to, int counter) 
{
	if(to == NULL) {
		to = xmlRegNewState(ctxt);
		xmlRegStatePush(ctxt, to);
		ctxt->state = to;
	}
	xmlRegStateAddTrans(ctxt, from, NULL, to, -1, counter);
}
/**
 * xmlFAGenerateTransitions:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 * @atom:  the atom generating the transition
 *
 * Returns 0 if success and -1 in case of error.
 */
static int xmlFAGenerateTransitions(xmlRegParserCtxt * ctxt, xmlRegStatePtr from, xmlRegStatePtr to, xmlRegAtom * atom)
{
	xmlRegStatePtr end;
	if(atom == NULL) {
		ERROR("genrate transition: atom == NULL");
		return -1;
	}
	if(atom->type == XML_REGEXP_SUBREG) {
		/*
		 * this is a subexpression handling one should not need to
		 * create a new node except for XML_REGEXP_QUANT_RANGE.
		 */
		if(xmlRegAtomPush(ctxt, atom) < 0) {
			return -1;
		}
		if(to && (atom->stop != to) && (atom->quant != XML_REGEXP_QUANT_RANGE)) {
			/*
			 * Generate an epsilon transition to link to the target
			 */
			xmlFAGenerateEpsilonTransition(ctxt, atom->stop, to);
#ifdef DV
		}
		else if((to == NULL) && (atom->quant != XML_REGEXP_QUANT_RANGE) && (atom->quant != XML_REGEXP_QUANT_ONCE)) {
			to = xmlRegNewState(ctxt);
			xmlRegStatePush(ctxt, to);
			ctxt->state = to;
			xmlFAGenerateEpsilonTransition(ctxt, atom->stop, to);
#endif
		}
		switch(atom->quant) {
			case XML_REGEXP_QUANT_OPT:
			    atom->quant = XML_REGEXP_QUANT_ONCE;
			    /*
			 * transition done to the state after end of atom.
			 * 1. set transition from atom start to new state
			 * 2. set transition from atom end to this state.
			     */
			    if(to == NULL) {
				    xmlFAGenerateEpsilonTransition(ctxt, atom->start, 0);
				    xmlFAGenerateEpsilonTransition(ctxt, atom->stop, ctxt->state);
			    }
			    else {
				    xmlFAGenerateEpsilonTransition(ctxt, atom->start, to);
			    }
			    break;
			case XML_REGEXP_QUANT_MULT:
			    atom->quant = XML_REGEXP_QUANT_ONCE;
			    xmlFAGenerateEpsilonTransition(ctxt, atom->start, atom->stop);
			    xmlFAGenerateEpsilonTransition(ctxt, atom->stop, atom->start);
			    break;
			case XML_REGEXP_QUANT_PLUS:
			    atom->quant = XML_REGEXP_QUANT_ONCE;
			    xmlFAGenerateEpsilonTransition(ctxt, atom->stop, atom->start);
			    break;
			case XML_REGEXP_QUANT_RANGE: 
				{
					int counter;
					xmlRegStatePtr inter;
					xmlRegStatePtr newstate;
					/*
					 * create the final state now if needed
					 */
					if(to) {
						newstate = to;
					}
					else {
						newstate = xmlRegNewState(ctxt);
						xmlRegStatePush(ctxt, newstate);
					}
					/*
					 * The principle here is to use counted transition
					 * to avoid explosion in the number of states in the
					 * graph. This is clearly more complex but should not
					 * be exploitable at runtime.
					 */
					if((atom->min == 0) && (atom->start0 == NULL)) {
						/*
						 * duplicate a transition based on atom to count next
						 * occurences after 1. We cannot loop to atom->start
						 * directly because we need an epsilon transition to
						 * newstate.
						 */
						/* ???? For some reason it seems we never reach that
						   case, I suppose this got optimized out before when
						   building the automata */
						xmlRegAtom * copy = xmlRegCopyAtom(ctxt, atom);
						if(copy == NULL)
							return -1;
						copy->quant = XML_REGEXP_QUANT_ONCE;
						copy->min = 0;
						copy->max = 0;
						if(xmlFAGenerateTransitions(ctxt, atom->start, NULL, copy) < 0)
							return -1;
						inter = ctxt->state;
						counter = xmlRegGetCounter(ctxt);
						ctxt->counters[counter].min = atom->min - 1;
						ctxt->counters[counter].max = atom->max - 1;
						/* count the number of times we see it again */
						xmlFAGenerateCountedEpsilonTransition(ctxt, inter, atom->stop, counter);
						/* allow a way out based on the count */
						xmlFAGenerateCountedTransition(ctxt, inter, newstate, counter);
						/* and also allow a direct exit for 0 */
						xmlFAGenerateEpsilonTransition(ctxt, atom->start, newstate);
					}
					else {
						/*
						 * either we need the atom at least once or there
						 * is an atom->start0 allowing to easilly plug the
						 * epsilon transition.
						 */
						counter = xmlRegGetCounter(ctxt);
						ctxt->counters[counter].min = atom->min - 1;
						ctxt->counters[counter].max = atom->max - 1;
						/* count the number of times we see it again */
						xmlFAGenerateCountedEpsilonTransition(ctxt, atom->stop, atom->start, counter);
						/* allow a way out based on the count */
						xmlFAGenerateCountedTransition(ctxt, atom->stop, newstate, counter);
						/* and if needed allow a direct exit for 0 */
						if(atom->min == 0)
							xmlFAGenerateEpsilonTransition(ctxt, atom->start0, newstate);
					}
					atom->min = 0;
					atom->max = 0;
					atom->quant = XML_REGEXP_QUANT_ONCE;
					ctxt->state = newstate;
				}
			default:
			    break;
		}
		return 0;
	}
	if((atom->min == 0) && (atom->max == 0) && (atom->quant == XML_REGEXP_QUANT_RANGE)) {
		/*
		 * we can discard the atom and generate an epsilon transition instead
		 */
		if(to == NULL) {
			to = xmlRegNewState(ctxt);
			if(to)
				xmlRegStatePush(ctxt, to);
			else {
				return -1;
			}
		}
		xmlFAGenerateEpsilonTransition(ctxt, from, to);
		ctxt->state = to;
		xmlRegFreeAtom(atom);
		return 0;
	}
	if(to == NULL) {
		to = xmlRegNewState(ctxt);
		if(to)
			xmlRegStatePush(ctxt, to);
		else {
			return -1;
		}
	}
	end = to;
	if((atom->quant == XML_REGEXP_QUANT_MULT) || (atom->quant == XML_REGEXP_QUANT_PLUS)) {
		/*
		 * Do not pollute the target state by adding transitions from
		 * it as it is likely to be the shared target of multiple branches.
		 * So isolate with an epsilon transition.
		 */
		xmlRegStatePtr tmp = xmlRegNewState(ctxt);
		if(tmp)
			xmlRegStatePush(ctxt, tmp);
		else {
			return -1;
		}
		xmlFAGenerateEpsilonTransition(ctxt, tmp, to);
		to = tmp;
	}
	if(xmlRegAtomPush(ctxt, atom) < 0) {
		return -1;
	}
	xmlRegStateAddTrans(ctxt, from, atom, to, -1, -1);
	ctxt->state = end;
	switch(atom->quant) {
		case XML_REGEXP_QUANT_OPT:
		    atom->quant = XML_REGEXP_QUANT_ONCE;
		    xmlFAGenerateEpsilonTransition(ctxt, from, to);
		    break;
		case XML_REGEXP_QUANT_MULT:
		    atom->quant = XML_REGEXP_QUANT_ONCE;
		    xmlFAGenerateEpsilonTransition(ctxt, from, to);
		    xmlRegStateAddTrans(ctxt, to, atom, to, -1, -1);
		    break;
		case XML_REGEXP_QUANT_PLUS:
		    atom->quant = XML_REGEXP_QUANT_ONCE;
		    xmlRegStateAddTrans(ctxt, to, atom, to, -1, -1);
		    break;
		case XML_REGEXP_QUANT_RANGE:
#if DV_test
		    if(atom->min == 0) {
			    xmlFAGenerateEpsilonTransition(ctxt, from, to);
		    }
#endif
		    break;
		default:
		    break;
	}
	return 0;
}
/**
 * xmlFAReduceEpsilonTransitions:
 * @ctxt:  a regexp parser context
 * @fromnr:  the from state
 * @tonr:  the to state
 * @counter:  should that transition be associated to a counted
 *
 */
static void xmlFAReduceEpsilonTransitions(xmlRegParserCtxt * ctxt, int fromnr, int tonr, int counter) 
{
	int transnr;
	xmlRegStatePtr from;
	xmlRegStatePtr to;
#ifdef DEBUG_REGEXP_GRAPH
	printf("xmlFAReduceEpsilonTransitions(%d, %d)\n", fromnr, tonr);
#endif
	from = ctxt->states[fromnr];
	if(from == NULL)
		return;
	to = ctxt->states[tonr];
	if(to == NULL)
		return;
	if((to->mark == XML_REGEXP_MARK_START) || (to->mark == XML_REGEXP_MARK_VISITED))
		return;
	to->mark = XML_REGEXP_MARK_VISITED;
	if(to->type == XML_REGEXP_FINAL_STATE) {
#ifdef DEBUG_REGEXP_GRAPH
		printf("State %d is final, so %d becomes final\n", tonr, fromnr);
#endif
		from->type = XML_REGEXP_FINAL_STATE;
	}
	for(transnr = 0; transnr < to->nbTrans; transnr++) {
		if(to->trans[transnr].to < 0)
			continue;
		if(to->trans[transnr].atom == NULL) {
			/*
			 * Don't remove counted transitions
			 * Don't loop either
			 */
			if(to->trans[transnr].to != fromnr) {
				if(to->trans[transnr].count >= 0) {
					int newto = to->trans[transnr].to;
					xmlRegStateAddTrans(ctxt, from, NULL, ctxt->states[newto], -1, to->trans[transnr].count);
				}
				else {
#ifdef DEBUG_REGEXP_GRAPH
					printf("Found epsilon trans %d from %d to %d\n", transnr, tonr, to->trans[transnr].to);
#endif
					if(to->trans[transnr].counter >= 0) {
						xmlFAReduceEpsilonTransitions(ctxt, fromnr, to->trans[transnr].to, to->trans[transnr].counter);
					}
					else {
						xmlFAReduceEpsilonTransitions(ctxt, fromnr, to->trans[transnr].to, counter);
					}
				}
			}
		}
		else {
			int newto = to->trans[transnr].to;
			if(to->trans[transnr].counter >= 0) {
				xmlRegStateAddTrans(ctxt, from, to->trans[transnr].atom, ctxt->states[newto], to->trans[transnr].counter, -1);
			}
			else {
				xmlRegStateAddTrans(ctxt, from, to->trans[transnr].atom, ctxt->states[newto], counter, -1);
			}
		}
	}
	to->mark = XML_REGEXP_MARK_NORMAL;
}
/**
 * xmlFAEliminateSimpleEpsilonTransitions:
 * @ctxt:  a regexp parser context
 *
 * Eliminating general epsilon transitions can get costly in the general
 * algorithm due to the large amount of generated new transitions and
 * associated comparisons. However for simple epsilon transition used just
 * to separate building blocks when generating the automata this can be
 * reduced to state elimination:
 *  - if there exists an epsilon from X to Y
 *  - if there is no other transition from X
 * then X and Y are semantically equivalent and X can be eliminated
 * If X is the start state then make Y the start state, else replace the
 * target of all transitions to X by transitions to Y.
 */
static void xmlFAEliminateSimpleEpsilonTransitions(xmlRegParserCtxt * ctxt) 
{
	int statenr, i, j, newto;
	xmlRegState * state;
	xmlRegState * tmp;
	for(statenr = 0; statenr < ctxt->nbStates; statenr++) {
		state = ctxt->states[statenr];
		if(state == NULL)
			continue;
		if(state->nbTrans != 1)
			continue;
		if(state->type == XML_REGEXP_UNREACH_STATE)
			continue;
		/* is the only transition out a basic transition */
		if(!state->trans[0].atom && (state->trans[0].to >= 0) && (state->trans[0].to != statenr) && (state->trans[0].counter < 0) && (state->trans[0].count < 0)) {
			newto = state->trans[0].to;
			if(state->type == XML_REGEXP_START_STATE) {
#ifdef DEBUG_REGEXP_GRAPH
				printf("Found simple epsilon trans from start %d to %d\n", statenr, newto);
#endif
			}
			else {
#ifdef DEBUG_REGEXP_GRAPH
				printf("Found simple epsilon trans from %d to %d\n", statenr, newto);
#endif
				for(i = 0; i < state->nbTransTo; i++) {
					tmp = ctxt->states[state->transTo[i]];
					for(j = 0; j < tmp->nbTrans; j++) {
						if(tmp->trans[j].to == statenr) {
#ifdef DEBUG_REGEXP_GRAPH
							printf("Changed transition %d on %d to go to %d\n", j, tmp->no, newto);
#endif
							tmp->trans[j].to = -1;
							xmlRegStateAddTrans(ctxt, tmp, tmp->trans[j].atom, ctxt->states[newto], tmp->trans[j].counter, tmp->trans[j].count);
						}
					}
				}
				if(state->type == XML_REGEXP_FINAL_STATE)
					ctxt->states[newto]->type = XML_REGEXP_FINAL_STATE;
				/* eliminate the transition completely */
				state->nbTrans = 0;
				state->type = XML_REGEXP_UNREACH_STATE;
			}
		}
	}
}

/**
 * xmlFAEliminateEpsilonTransitions:
 * @ctxt:  a regexp parser context
 *
 */
static void xmlFAEliminateEpsilonTransitions(xmlRegParserCtxt * ctxt)
{
	int statenr, transnr;
	xmlRegStatePtr state;
	int has_epsilon;
	if(ctxt->states == NULL)
		return;

	/*
	 * Eliminate simple epsilon transition and the associated unreachable
	 * states.
	 */
	xmlFAEliminateSimpleEpsilonTransitions(ctxt);
	for(statenr = 0; statenr < ctxt->nbStates; statenr++) {
		state = ctxt->states[statenr];
		if(state && state->type == XML_REGEXP_UNREACH_STATE) {
#ifdef DEBUG_REGEXP_GRAPH
			printf("Removed unreachable state %d\n", statenr);
#endif
			xmlRegFreeState(state);
			ctxt->states[statenr] = NULL;
		}
	}

	has_epsilon = 0;

	/*
	 * Build the completed transitions bypassing the epsilons
	 * Use a marking algorithm to avoid loops
	 * Mark sink states too.
	 * Process from the latests states backward to the start when
	 * there is long cascading epsilon chains this minimize the
	 * recursions and transition compares when adding the new ones
	 */
	for(statenr = ctxt->nbStates - 1; statenr >= 0; statenr--) {
		state = ctxt->states[statenr];
		if(state == NULL)
			continue;
		if((state->nbTrans == 0) && (state->type != XML_REGEXP_FINAL_STATE)) {
			state->type = XML_REGEXP_SINK_STATE;
		}
		for(transnr = 0; transnr < state->nbTrans; transnr++) {
			if((state->trans[transnr].atom == NULL) && (state->trans[transnr].to >= 0)) {
				if(state->trans[transnr].to == statenr) {
					state->trans[transnr].to = -1;
#ifdef DEBUG_REGEXP_GRAPH
					printf("Removed loopback epsilon trans %d on %d\n", transnr, statenr);
#endif
				}
				else if(state->trans[transnr].count < 0) {
					int newto = state->trans[transnr].to;

#ifdef DEBUG_REGEXP_GRAPH
					printf("Found epsilon trans %d from %d to %d\n", transnr, statenr, newto);
#endif
					has_epsilon = 1;
					state->trans[transnr].to = -2;
					state->mark = XML_REGEXP_MARK_START;
					xmlFAReduceEpsilonTransitions(ctxt, statenr, newto, state->trans[transnr].counter);
					state->mark = XML_REGEXP_MARK_NORMAL;
#ifdef DEBUG_REGEXP_GRAPH
				}
				else {
					printf("Found counted transition %d on %d\n", transnr, statenr);
#endif
				}
			}
		}
	}
	/*
	 * Eliminate the epsilon transitions
	 */
	if(has_epsilon) {
		for(statenr = 0; statenr < ctxt->nbStates; statenr++) {
			state = ctxt->states[statenr];
			if(state == NULL)
				continue;
			for(transnr = 0; transnr < state->nbTrans; transnr++) {
				xmlRegTransPtr trans = &(state->trans[transnr]);
				if(!trans->atom && (trans->count < 0) && (trans->to >= 0)) {
					trans->to = -1;
				}
			}
		}
	}

	/*
	 * Use this pass to detect unreachable states too
	 */
	for(statenr = 0; statenr < ctxt->nbStates; statenr++) {
		state = ctxt->states[statenr];
		if(state)
			state->reached = XML_REGEXP_MARK_NORMAL;
	}
	state = ctxt->states[0];
	if(state)
		state->reached = XML_REGEXP_MARK_START;
	while(state) {
		xmlRegStatePtr target = NULL;
		state->reached = XML_REGEXP_MARK_VISITED;
		/*
		 * Mark all states reachable from the current reachable state
		 */
		for(transnr = 0; transnr < state->nbTrans; transnr++) {
			if((state->trans[transnr].to >= 0) && (state->trans[transnr].atom || (state->trans[transnr].count >= 0))) {
				int newto = state->trans[transnr].to;

				if(ctxt->states[newto] == NULL)
					continue;
				if(ctxt->states[newto]->reached == XML_REGEXP_MARK_NORMAL) {
					ctxt->states[newto]->reached = XML_REGEXP_MARK_START;
					target = ctxt->states[newto];
				}
			}
		}

		/*
		 * find the next accessible state not explored
		 */
		if(target == NULL) {
			for(statenr = 1; statenr < ctxt->nbStates; statenr++) {
				state = ctxt->states[statenr];
				if(state && (state->reached == XML_REGEXP_MARK_START)) {
					target = state;
					break;
				}
			}
		}
		state = target;
	}
	for(statenr = 0; statenr < ctxt->nbStates; statenr++) {
		state = ctxt->states[statenr];
		if(state && state->reached == XML_REGEXP_MARK_NORMAL) {
#ifdef DEBUG_REGEXP_GRAPH
			printf("Removed unreachable state %d\n", statenr);
#endif
			xmlRegFreeState(state);
			ctxt->states[statenr] = NULL;
		}
	}
}

static int xmlFACompareRanges(xmlRegRangePtr range1, xmlRegRangePtr range2)
{
	int ret = 0;
	if((range1->type == XML_REGEXP_RANGES) ||
	    (range2->type == XML_REGEXP_RANGES) ||
	    (range2->type == XML_REGEXP_SUBREG) ||
	    (range1->type == XML_REGEXP_SUBREG) ||
	    (range1->type == XML_REGEXP_STRING) ||
	    (range2->type == XML_REGEXP_STRING))
		return -1;

	/* put them in order */
	if(range1->type > range2->type) {
		xmlRegRangePtr tmp = range1;
		range1 = range2;
		range2 = tmp;
	}
	if((range1->type == XML_REGEXP_ANYCHAR) || (range2->type == XML_REGEXP_ANYCHAR)) {
		ret = 1;
	}
	else if((range1->type == XML_REGEXP_EPSILON) || (range2->type == XML_REGEXP_EPSILON)) {
		return 0;
	}
	else if(range1->type == range2->type) {
		if(range1->type != XML_REGEXP_CHARVAL)
			ret = 1;
		else if((range1->end < range2->start) || (range2->end < range1->start))
			ret = 0;
		else
			ret = 1;
	}
	else if(range1->type == XML_REGEXP_CHARVAL) {
		int codepoint;
		int neg = 0;
		/*
		 * just check all codepoints in the range for acceptance,
		 * this is usually way cheaper since done only once at
		 * compilation than testing over and over at runtime or
		 * pushing too many states when evaluating.
		 */
		if(((range1->neg == 0) && (range2->neg != 0)) || ((range1->neg != 0) && (range2->neg == 0)))
			neg = 1;
		for(codepoint = range1->start; codepoint <= range1->end; codepoint++) {
			ret = xmlRegCheckCharacterRange(range2->type, codepoint, 0, range2->start, range2->end, range2->blockName);
			if(ret < 0)
				return -1;
			if(((neg == 1) && (ret == 0)) || ((neg == 0) && (ret == 1)))
				return 1;
		}
		return 0;
	}
	else if((range1->type == XML_REGEXP_BLOCK_NAME) || (range2->type == XML_REGEXP_BLOCK_NAME)) {
		if(range1->type == range2->type) {
			ret = sstreq(range1->blockName, range2->blockName);
		}
		else {
			/*
			 * comparing a block range with anything else is way
			 * too costly, and maintining the table is like too much
			 * memory too, so let's force the automata to save state
			 * here.
			 */
			return 1;
		}
	}
	else if((range1->type < XML_REGEXP_LETTER) || (range2->type < XML_REGEXP_LETTER)) {
		if((range1->type == XML_REGEXP_ANYSPACE) && (range2->type == XML_REGEXP_NOTSPACE))
			ret = 0;
		else if((range1->type == XML_REGEXP_INITNAME) && (range2->type == XML_REGEXP_NOTINITNAME))
			ret = 0;
		else if((range1->type == XML_REGEXP_NAMECHAR) && (range2->type == XML_REGEXP_NOTNAMECHAR))
			ret = 0;
		else if((range1->type == XML_REGEXP_DECIMAL) && (range2->type == XML_REGEXP_NOTDECIMAL))
			ret = 0;
		else if((range1->type == XML_REGEXP_REALCHAR) && (range2->type == XML_REGEXP_NOTREALCHAR))
			ret = 0;
		else {
			/* same thing to limit complexity */
			return 1;
		}
	}
	else {
		ret = 0;
		/* range1->type < range2->type here */
		switch(range1->type) {
			case XML_REGEXP_LETTER:
			    /* all disjoint except in the subgroups */
			    if((range2->type == XML_REGEXP_LETTER_UPPERCASE) ||
			    (range2->type == XML_REGEXP_LETTER_LOWERCASE) ||
			    (range2->type == XML_REGEXP_LETTER_TITLECASE) ||
			    (range2->type == XML_REGEXP_LETTER_MODIFIER) ||
			    (range2->type == XML_REGEXP_LETTER_OTHERS))
				    ret = 1;
			    break;
			case XML_REGEXP_MARK:
			    if((range2->type == XML_REGEXP_MARK_NONSPACING) ||
			    (range2->type == XML_REGEXP_MARK_SPACECOMBINING) ||
			    (range2->type == XML_REGEXP_MARK_ENCLOSING))
				    ret = 1;
			    break;
			case XML_REGEXP_NUMBER:
			    if((range2->type == XML_REGEXP_NUMBER_DECIMAL) ||
			    (range2->type == XML_REGEXP_NUMBER_LETTER) ||
			    (range2->type == XML_REGEXP_NUMBER_OTHERS))
				    ret = 1;
			    break;
			case XML_REGEXP_PUNCT:
			    if((range2->type == XML_REGEXP_PUNCT_CONNECTOR) ||
			    (range2->type == XML_REGEXP_PUNCT_DASH) ||
			    (range2->type == XML_REGEXP_PUNCT_OPEN) ||
			    (range2->type == XML_REGEXP_PUNCT_CLOSE) ||
			    (range2->type == XML_REGEXP_PUNCT_INITQUOTE) ||
			    (range2->type == XML_REGEXP_PUNCT_FINQUOTE) ||
			    (range2->type == XML_REGEXP_PUNCT_OTHERS))
				    ret = 1;
			    break;
			case XML_REGEXP_SEPAR:
			    if(oneof3(range2->type, XML_REGEXP_SEPAR_SPACE, XML_REGEXP_SEPAR_LINE, XML_REGEXP_SEPAR_PARA))
				    ret = 1;
			    break;
			case XML_REGEXP_SYMBOL:
			    if(oneof4(range2->type, XML_REGEXP_SYMBOL_MATH, XML_REGEXP_SYMBOL_CURRENCY, XML_REGEXP_SYMBOL_MODIFIER, XML_REGEXP_SYMBOL_OTHERS))
				    ret = 1;
			    break;
			case XML_REGEXP_OTHER:
			    if(oneof3(range2->type, XML_REGEXP_OTHER_CONTROL, XML_REGEXP_OTHER_FORMAT, XML_REGEXP_OTHER_PRIVATE))
				    ret = 1;
			    break;
			default:
			    if((range2->type >= XML_REGEXP_LETTER) && (range2->type < XML_REGEXP_BLOCK_NAME))
				    ret = 0;
			    else {
				    /* safety net ! */
				    return 1;
			    }
		}
	}
	if(((range1->neg == 0) && (range2->neg != 0)) ||
	    ((range1->neg != 0) && (range2->neg == 0)))
		ret = !ret;
	return ret;
}

/**
 * xmlFACompareAtomTypes:
 * @type1:  an atom type
 * @type2:  an atom type
 *
 * Compares two atoms type to check whether they intersect in some ways,
 * this is used by xmlFACompareAtoms only
 *
 * Returns 1 if they may intersect and 0 otherwise
 */
static int xmlFACompareAtomTypes(xmlRegAtomType type1, xmlRegAtomType type2)
{
	if((type1 == XML_REGEXP_EPSILON) ||
	    (type1 == XML_REGEXP_CHARVAL) ||
	    (type1 == XML_REGEXP_RANGES) ||
	    (type1 == XML_REGEXP_SUBREG) ||
	    (type1 == XML_REGEXP_STRING) ||
	    (type1 == XML_REGEXP_ANYCHAR))
		return 1;
	if((type2 == XML_REGEXP_EPSILON) ||
	    (type2 == XML_REGEXP_CHARVAL) ||
	    (type2 == XML_REGEXP_RANGES) ||
	    (type2 == XML_REGEXP_SUBREG) ||
	    (type2 == XML_REGEXP_STRING) ||
	    (type2 == XML_REGEXP_ANYCHAR))
		return 1;

	if(type1 == type2) return 1;

	/* simplify subsequent compares by making sure type1 < type2 */
	if(type1 > type2) {
		xmlRegAtomType tmp = type1;
		type1 = type2;
		type2 = tmp;
	}
	switch(type1) {
		case XML_REGEXP_ANYSPACE: /* \s */
		    /* can't be a letter, number, mark, pontuation, symbol */
		    if((type2 == XML_REGEXP_NOTSPACE) ||
		    ((type2 >= XML_REGEXP_LETTER) &&
			    (type2 <= XML_REGEXP_LETTER_OTHERS)) ||
		    ((type2 >= XML_REGEXP_NUMBER) &&
			    (type2 <= XML_REGEXP_NUMBER_OTHERS)) ||
		    ((type2 >= XML_REGEXP_MARK) &&
			    (type2 <= XML_REGEXP_MARK_ENCLOSING)) ||
		    ((type2 >= XML_REGEXP_PUNCT) &&
			    (type2 <= XML_REGEXP_PUNCT_OTHERS)) ||
		    ((type2 >= XML_REGEXP_SYMBOL) &&
			    (type2 <= XML_REGEXP_SYMBOL_OTHERS))
		    ) return 0;
		    break;
		case XML_REGEXP_NOTSPACE: /* \S */
		    break;
		case XML_REGEXP_INITNAME: /* \l */
		    /* can't be a number, mark, separator, pontuation, symbol or other */
		    if((type2 == XML_REGEXP_NOTINITNAME) ||
		    ((type2 >= XML_REGEXP_NUMBER) &&
			    (type2 <= XML_REGEXP_NUMBER_OTHERS)) ||
		    ((type2 >= XML_REGEXP_MARK) &&
			    (type2 <= XML_REGEXP_MARK_ENCLOSING)) ||
		    ((type2 >= XML_REGEXP_SEPAR) &&
			    (type2 <= XML_REGEXP_SEPAR_PARA)) ||
		    ((type2 >= XML_REGEXP_PUNCT) &&
			    (type2 <= XML_REGEXP_PUNCT_OTHERS)) ||
		    ((type2 >= XML_REGEXP_SYMBOL) &&
			    (type2 <= XML_REGEXP_SYMBOL_OTHERS)) ||
		    ((type2 >= XML_REGEXP_OTHER) &&
			    (type2 <= XML_REGEXP_OTHER_NA))
		    ) return 0;
		    break;
		case XML_REGEXP_NOTINITNAME: /* \L */
		    break;
		case XML_REGEXP_NAMECHAR: /* \c */
		    /* can't be a mark, separator, pontuation, symbol or other */
		    if((type2 == XML_REGEXP_NOTNAMECHAR) || ((type2 >= XML_REGEXP_MARK) &&
			    (type2 <= XML_REGEXP_MARK_ENCLOSING)) ||
		    ((type2 >= XML_REGEXP_PUNCT) && (type2 <= XML_REGEXP_PUNCT_OTHERS)) ||
		    ((type2 >= XML_REGEXP_SEPAR) && (type2 <= XML_REGEXP_SEPAR_PARA)) ||
		    ((type2 >= XML_REGEXP_SYMBOL) && (type2 <= XML_REGEXP_SYMBOL_OTHERS)) ||
		    ((type2 >= XML_REGEXP_OTHER) && (type2 <= XML_REGEXP_OTHER_NA))
		    ) return 0;
		    break;
		case XML_REGEXP_NOTNAMECHAR: /* \C */
		    break;
		case XML_REGEXP_DECIMAL: /* \d */
		    /* can't be a letter, mark, separator, pontuation, symbol or other */
		    if(oneof2(type2, XML_REGEXP_NOTDECIMAL, XML_REGEXP_REALCHAR) ||
		    ((type2 >= XML_REGEXP_LETTER) && (type2 <= XML_REGEXP_LETTER_OTHERS)) ||
		    ((type2 >= XML_REGEXP_MARK) && (type2 <= XML_REGEXP_MARK_ENCLOSING)) ||
		    ((type2 >= XML_REGEXP_PUNCT) && (type2 <= XML_REGEXP_PUNCT_OTHERS)) ||
		    ((type2 >= XML_REGEXP_SEPAR) && (type2 <= XML_REGEXP_SEPAR_PARA)) ||
		    ((type2 >= XML_REGEXP_SYMBOL) && (type2 <= XML_REGEXP_SYMBOL_OTHERS)) ||
		    ((type2 >= XML_REGEXP_OTHER) && (type2 <= XML_REGEXP_OTHER_NA))
		    ) return 0;
		    break;
		case XML_REGEXP_NOTDECIMAL: /* \D */
		    break;
		case XML_REGEXP_REALCHAR: /* \w */
		    /* can't be a mark, separator, pontuation, symbol or other */
		    if((type2 == XML_REGEXP_NOTDECIMAL) ||
				((type2 >= XML_REGEXP_MARK) && (type2 <= XML_REGEXP_MARK_ENCLOSING)) ||
				((type2 >= XML_REGEXP_PUNCT) && (type2 <= XML_REGEXP_PUNCT_OTHERS)) ||
				((type2 >= XML_REGEXP_SEPAR) && (type2 <= XML_REGEXP_SEPAR_PARA)) ||
				((type2 >= XML_REGEXP_SYMBOL) && (type2 <= XML_REGEXP_SYMBOL_OTHERS)) ||
				((type2 >= XML_REGEXP_OTHER) && (type2 <= XML_REGEXP_OTHER_NA)))
				return 0;
		    break;
		case XML_REGEXP_NOTREALCHAR: /* \W */
		    break;
		/*
		 * at that point we know both type 1 and type2 are from
		 * character categories are ordered and are different,
		 * it becomes simple because this is a partition
		 */
		case XML_REGEXP_LETTER:
			return (type2 <= XML_REGEXP_LETTER_OTHERS) ? 1 : 0;
		case XML_REGEXP_LETTER_UPPERCASE:
		case XML_REGEXP_LETTER_LOWERCASE:
		case XML_REGEXP_LETTER_TITLECASE:
		case XML_REGEXP_LETTER_MODIFIER:
		case XML_REGEXP_LETTER_OTHERS:
		    return 0;
		case XML_REGEXP_MARK:
			return (type2 <= XML_REGEXP_MARK_ENCLOSING) ? 1 : 0;
		case XML_REGEXP_MARK_NONSPACING:
		case XML_REGEXP_MARK_SPACECOMBINING:
		case XML_REGEXP_MARK_ENCLOSING:
		    return 0;
		case XML_REGEXP_NUMBER:
			return (type2 <= XML_REGEXP_NUMBER_OTHERS) ? 1 : 0;
		case XML_REGEXP_NUMBER_DECIMAL:
		case XML_REGEXP_NUMBER_LETTER:
		case XML_REGEXP_NUMBER_OTHERS:
		    return 0;
		case XML_REGEXP_PUNCT:
			return (type2 <= XML_REGEXP_PUNCT_OTHERS) ? 1 : 0;
		case XML_REGEXP_PUNCT_CONNECTOR:
		case XML_REGEXP_PUNCT_DASH:
		case XML_REGEXP_PUNCT_OPEN:
		case XML_REGEXP_PUNCT_CLOSE:
		case XML_REGEXP_PUNCT_INITQUOTE:
		case XML_REGEXP_PUNCT_FINQUOTE:
		case XML_REGEXP_PUNCT_OTHERS:
		    return 0;
		case XML_REGEXP_SEPAR:
			return (type2 <= XML_REGEXP_SEPAR_PARA) ? 1 : 0;
		case XML_REGEXP_SEPAR_SPACE:
		case XML_REGEXP_SEPAR_LINE:
		case XML_REGEXP_SEPAR_PARA:
		    return 0;
		case XML_REGEXP_SYMBOL:
			return (type2 <= XML_REGEXP_SYMBOL_OTHERS) ? 1 : 0;
		case XML_REGEXP_SYMBOL_MATH:
		case XML_REGEXP_SYMBOL_CURRENCY:
		case XML_REGEXP_SYMBOL_MODIFIER:
		case XML_REGEXP_SYMBOL_OTHERS:
		    return 0;
		case XML_REGEXP_OTHER:
			return (type2 <= XML_REGEXP_OTHER_NA) ? 1 : 0;
		case XML_REGEXP_OTHER_CONTROL:
		case XML_REGEXP_OTHER_FORMAT:
		case XML_REGEXP_OTHER_PRIVATE:
		case XML_REGEXP_OTHER_NA:
		    return 0;
		default:
		    break;
	}
	return 1;
}

/**
 * xmlFAEqualAtoms:
 * @atom1:  an atom
 * @atom2:  an atom
 * @deep: if not set only compare string pointers
 *
 * Compares two atoms to check whether they are the same exactly
 * this is used to remove equivalent transitions
 *
 * Returns 1 if same and 0 otherwise
 */
static int xmlFAEqualAtoms(xmlRegAtom * atom1, xmlRegAtom * atom2, int deep)
{
	int ret = 0;
	if(atom1 == atom2)
		return 1;
	if(!atom1 || !atom2)
		return 0;
	if(atom1->type != atom2->type)
		return 0;
	switch(atom1->type) {
		case XML_REGEXP_EPSILON:
		    ret = 0;
		    break;
		case XML_REGEXP_STRING:
			ret = (!deep) ? (atom1->valuep == atom2->valuep) : sstreq((xmlChar *)atom1->valuep, (xmlChar *)atom2->valuep);
		    break;
		case XML_REGEXP_CHARVAL:
		    ret = (atom1->codepoint == atom2->codepoint);
		    break;
		case XML_REGEXP_RANGES:
		    /* too hard to do in the general case */
		    ret = 0;
		default:
		    break;
	}
	return ret;
}

/**
 * xmlFACompareAtoms:
 * @atom1:  an atom
 * @atom2:  an atom
 * @deep: if not set only compare string pointers
 *
 * Compares two atoms to check whether they intersect in some ways,
 * this is used by xmlFAComputesDeterminism and xmlFARecurseDeterminism only
 *
 * Returns 1 if yes and 0 otherwise
 */
static int xmlFACompareAtoms(xmlRegAtom * atom1, xmlRegAtom * atom2, int deep)
{
	int ret = 1;
	if(atom1 == atom2)
		return 1;
	if(!atom1 || !atom2)
		return 0;
	if((atom1->type == XML_REGEXP_ANYCHAR) || (atom2->type == XML_REGEXP_ANYCHAR))
		return 1;
	if(atom1->type > atom2->type) {
		xmlRegAtom * tmp;
		tmp = atom1;
		atom1 = atom2;
		atom2 = tmp;
	}
	if(atom1->type != atom2->type) {
		ret = xmlFACompareAtomTypes(atom1->type, atom2->type);
		/* if they can't intersect at the type level break now */
		if(!ret)
			return 0;
	}
	switch(atom1->type) {
		case XML_REGEXP_STRING:
			ret = (!deep) ? (atom1->valuep != atom2->valuep) : xmlRegStrEqualWildcard((xmlChar *)atom1->valuep, (xmlChar *)atom2->valuep);
		    break;
		case XML_REGEXP_EPSILON:
		    goto not_determinist;
		case XML_REGEXP_CHARVAL:
		    if(atom2->type == XML_REGEXP_CHARVAL) {
			    ret = (atom1->codepoint == atom2->codepoint);
		    }
		    else {
			    ret = xmlRegCheckCharacter(atom2, atom1->codepoint);
			    if(ret < 0)
				    ret = 1;
		    }
		    break;
		case XML_REGEXP_RANGES:
		    if(atom2->type == XML_REGEXP_RANGES) {
			    int i, j, res;
			    xmlRegRangePtr r1, r2;

			    /*
			 * need to check that none of the ranges eventually matches
			     */
			    for(i = 0; i < atom1->nbRanges; i++) {
				    for(j = 0; j < atom2->nbRanges; j++) {
					    r1 = atom1->ranges[i];
					    r2 = atom2->ranges[j];
					    res = xmlFACompareRanges(r1, r2);
					    if(res == 1) {
						    ret = 1;
						    goto done;
					    }
				    }
			    }
			    ret = 0;
		    }
		    break;
		default:
		    goto not_determinist;
	}
done:
	if(atom1->neg != atom2->neg) {
		ret = !ret;
	}
	if(!ret)
		return 0;
not_determinist:
	return 1;
}

/**
 * xmlFARecurseDeterminism:
 * @ctxt:  a regexp parser context
 *
 * Check whether the associated regexp is determinist,
 * should be called after xmlFAEliminateEpsilonTransitions()
 *
 */
static int xmlFARecurseDeterminism(xmlRegParserCtxt * ctxt, xmlRegStatePtr state, int to, xmlRegAtom * atom)
{
	int ret = 1;
	int res;
	int transnr, nbTrans;
	xmlRegTransPtr t1;
	int deep = 1;
	if(state == NULL)
		return ret;
	if(state->markd == XML_REGEXP_MARK_VISITED)
		return ret;
	if(ctxt->flags & AM_AUTOMATA_RNG)
		deep = 0;

	/*
	 * don't recurse on transitions potentially added in the course of
	 * the elimination.
	 */
	nbTrans = state->nbTrans;
	for(transnr = 0; transnr < nbTrans; transnr++) {
		t1 = &(state->trans[transnr]);
		/*
		 * check transitions conflicting with the one looked at
		 */
		if(t1->atom == NULL) {
			if(t1->to < 0)
				continue;
			state->markd = XML_REGEXP_MARK_VISITED;
			res = xmlFARecurseDeterminism(ctxt, ctxt->states[t1->to], to, atom);
			state->markd = XML_REGEXP_MARK_NORMAL;
			if(res == 0) {
				ret = 0;
				/* t1->nd = 1; */
			}
			continue;
		}
		if(t1->to != to)
			continue;
		if(xmlFACompareAtoms(t1->atom, atom, deep)) {
			ret = 0;
			/* mark the transition as non-deterministic */
			t1->nd = 1;
		}
	}
	return ret;
}

/**
 * xmlFAComputesDeterminism:
 * @ctxt:  a regexp parser context
 *
 * Check whether the associated regexp is determinist,
 * should be called after xmlFAEliminateEpsilonTransitions()
 *
 */
static int xmlFAComputesDeterminism(xmlRegParserCtxt * ctxt)
{
	int statenr, transnr;
	xmlRegStatePtr state;
	xmlRegTransPtr t1, t2, last;
	int i;
	int ret = 1;
	int deep = 1;
#ifdef DEBUG_REGEXP_GRAPH
	printf("xmlFAComputesDeterminism\n");
	xmlRegPrintCtxt(stdout, ctxt);
#endif
	if(ctxt->determinist != -1)
		return (ctxt->determinist);
	if(ctxt->flags & AM_AUTOMATA_RNG)
		deep = 0;
	/*
	 * First cleanup the automata removing cancelled transitions
	 */
	for(statenr = 0; statenr < ctxt->nbStates; statenr++) {
		state = ctxt->states[statenr];
		if(state && state->nbTrans >= 2) {
			for(transnr = 0; transnr < state->nbTrans; transnr++) {
				t1 = &(state->trans[transnr]);
				//
				// Determinism checks in case of counted or all transitions
				// will have to be handled separately
				//
				if(t1->atom == NULL) {
					// t1->nd = 1;
					continue;
				}
				if(t1->to == -1) // eliminated
					continue;
				for(i = 0; i < transnr; i++) {
					t2 = &(state->trans[i]);
					if(t2->to == -1) /* eliminated */
						continue;
					if(t2->atom) {
						if(t1->to == t2->to) {
							/*
							* Here we use deep because we want to keep the
							* transitions which indicate a conflict
							*/
							if(xmlFAEqualAtoms(t1->atom, t2->atom, deep) && (t1->counter == t2->counter) && (t1->count == t2->count))
								t2->to = -1;  /* eliminated */
						}
					}
				}
			}
		}
	}
	//
	// Check for all states that there aren't 2 transitions
	// with the same atom and a different target.
	//
	for(statenr = 0; statenr < ctxt->nbStates; statenr++) {
		state = ctxt->states[statenr];
		if(state && state->nbTrans >= 2) {
			last = NULL;
			for(transnr = 0; transnr < state->nbTrans; transnr++) {
				t1 = &(state->trans[transnr]);
				/*
				* Determinism checks in case of counted or all transitions
				* will have to be handled separately
				*/
				if(t1->atom == NULL) {
					continue;
				}
				if(t1->to == -1) /* eliminated */
					continue;
				for(i = 0; i < transnr; i++) {
					t2 = &(state->trans[i]);
					if(t2->to == -1) /* eliminated */
						continue;
					if(t2->atom) {
						/*
						* But here we don't use deep because we want to
						* find transitions which indicate a conflict
						*/
						if(xmlFACompareAtoms(t1->atom, t2->atom, 1)) {
							ret = 0;
							/* mark the transitions as non-deterministic ones */
							t1->nd = 1;
							t2->nd = 1;
							last = t1;
						}
					}
					else if(t1->to != -1) {
						/*
						* do the closure in case of remaining specific
						* epsilon transitions like choices or all
						*/
						ret = xmlFARecurseDeterminism(ctxt, ctxt->states[t1->to],
							t2->to, t2->atom);
						/* don't shortcut the computation so all non deterministic
						transition get marked down
						if (ret == 0)
							return 0;
						*/
						if(!ret) {
							t1->nd = 1;
							/* t2->nd = 1; */
							last = t1;
						}
					}
				}
				// don't shortcut the computation so all non deterministic transition get marked down
				// if(!ret) break;
			}

			/*
			* mark specifically the last non-deterministic transition
			* from a state since there is no need to set-up rollback
			* from it
			*/
			if(last) {
				last->nd = 2;
			}
			// don't shortcut the computation so all non deterministic transition get marked down
			//if(!ret) break;
		}
	}
	ctxt->determinist = ret;
	return ret;
}

/************************************************************************
*									*
*	Routines to check input against transition atoms		*
*									*
************************************************************************/

static int FASTCALL xmlRegCheckCharacterRange(xmlRegAtomType type, int codepoint, int neg, int start, int end, const xmlChar * blockName) 
{
	int ret = 0;
	switch(type) {
		case XML_REGEXP_STRING:
		case XML_REGEXP_SUBREG:
		case XML_REGEXP_RANGES:
		case XML_REGEXP_EPSILON:
		    return -1;
		case XML_REGEXP_ANYCHAR: ret = ((codepoint != '\n') && (codepoint != '\r')); break;
		case XML_REGEXP_CHARVAL: ret = ((codepoint >= start) && (codepoint <= end)); break;
		case XML_REGEXP_NOTSPACE:
		    neg = !neg;
		case XML_REGEXP_ANYSPACE: ret = oneof4(codepoint, '\n', '\r', '\t', ' '); break;
		case XML_REGEXP_NOTINITNAME:
		    neg = !neg;
		case XML_REGEXP_INITNAME: ret = (IS_LETTER(codepoint) || (codepoint == '_') || (codepoint == ':')); break;
		case XML_REGEXP_NOTNAMECHAR:
		    neg = !neg;
		case XML_REGEXP_NAMECHAR:
		    ret = (IS_LETTER(codepoint) || IS_DIGIT(codepoint) || (codepoint == '.') || (codepoint == '-') ||
				(codepoint == '_') || (codepoint == ':') || IS_COMBINING(codepoint) || IS_EXTENDER(codepoint));
		    break;
		case XML_REGEXP_NOTDECIMAL:
		    neg = !neg;
		case XML_REGEXP_DECIMAL: ret = xmlUCSIsCatNd(codepoint); break;
		case XML_REGEXP_REALCHAR:
		    neg = !neg;
		case XML_REGEXP_NOTREALCHAR:
		    ret = xmlUCSIsCatP(codepoint);
			SETIFZ(ret, xmlUCSIsCatZ(codepoint));
			SETIFZ(ret, xmlUCSIsCatC(codepoint));
		    break;
		case XML_REGEXP_LETTER: ret = xmlUCSIsCatL(codepoint); break;
		case XML_REGEXP_LETTER_UPPERCASE: ret = xmlUCSIsCatLu(codepoint); break;
		case XML_REGEXP_LETTER_LOWERCASE: ret = xmlUCSIsCatLl(codepoint); break;
		case XML_REGEXP_LETTER_TITLECASE: ret = xmlUCSIsCatLt(codepoint); break;
		case XML_REGEXP_LETTER_MODIFIER: ret = xmlUCSIsCatLm(codepoint); break;
		case XML_REGEXP_LETTER_OTHERS: ret = xmlUCSIsCatLo(codepoint); break;
		case XML_REGEXP_MARK: ret = xmlUCSIsCatM(codepoint); break;
		case XML_REGEXP_MARK_NONSPACING: ret = xmlUCSIsCatMn(codepoint); break;
		case XML_REGEXP_MARK_SPACECOMBINING: ret = xmlUCSIsCatMc(codepoint); break;
		case XML_REGEXP_MARK_ENCLOSING: ret = xmlUCSIsCatMe(codepoint); break;
		case XML_REGEXP_NUMBER: ret = xmlUCSIsCatN(codepoint); break;
		case XML_REGEXP_NUMBER_DECIMAL: ret = xmlUCSIsCatNd(codepoint); break;
		case XML_REGEXP_NUMBER_LETTER: ret = xmlUCSIsCatNl(codepoint); break;
		case XML_REGEXP_NUMBER_OTHERS: ret = xmlUCSIsCatNo(codepoint); break;
		case XML_REGEXP_PUNCT: ret = xmlUCSIsCatP(codepoint); break;
		case XML_REGEXP_PUNCT_CONNECTOR: ret = xmlUCSIsCatPc(codepoint); break;
		case XML_REGEXP_PUNCT_DASH: ret = xmlUCSIsCatPd(codepoint); break;
		case XML_REGEXP_PUNCT_OPEN: ret = xmlUCSIsCatPs(codepoint); break;
		case XML_REGEXP_PUNCT_CLOSE: ret = xmlUCSIsCatPe(codepoint); break;
		case XML_REGEXP_PUNCT_INITQUOTE: ret = xmlUCSIsCatPi(codepoint); break;
		case XML_REGEXP_PUNCT_FINQUOTE: ret = xmlUCSIsCatPf(codepoint); break;
		case XML_REGEXP_PUNCT_OTHERS: ret = xmlUCSIsCatPo(codepoint); break;
		case XML_REGEXP_SEPAR: ret = xmlUCSIsCatZ(codepoint); break;
		case XML_REGEXP_SEPAR_SPACE: ret = xmlUCSIsCatZs(codepoint); break;
		case XML_REGEXP_SEPAR_LINE: ret = xmlUCSIsCatZl(codepoint); break;
		case XML_REGEXP_SEPAR_PARA: ret = xmlUCSIsCatZp(codepoint); break;
		case XML_REGEXP_SYMBOL: ret = xmlUCSIsCatS(codepoint); break;
		case XML_REGEXP_SYMBOL_MATH: ret = xmlUCSIsCatSm(codepoint); break;
		case XML_REGEXP_SYMBOL_CURRENCY: ret = xmlUCSIsCatSc(codepoint); break;
		case XML_REGEXP_SYMBOL_MODIFIER: ret = xmlUCSIsCatSk(codepoint); break;
		case XML_REGEXP_SYMBOL_OTHERS: ret = xmlUCSIsCatSo(codepoint); break;
		case XML_REGEXP_OTHER: ret = xmlUCSIsCatC(codepoint); break;
		case XML_REGEXP_OTHER_CONTROL: ret = xmlUCSIsCatCc(codepoint); break;
		case XML_REGEXP_OTHER_FORMAT: ret = xmlUCSIsCatCf(codepoint); break;
		case XML_REGEXP_OTHER_PRIVATE: ret = xmlUCSIsCatCo(codepoint); break;
		case XML_REGEXP_OTHER_NA:
		    /* ret = xmlUCSIsCatCn(codepoint); */
		    /* Seems it doesn't exist anymore in recent Unicode releases */
		    ret = 0;
		    break;
		case XML_REGEXP_BLOCK_NAME: ret = xmlUCSIsBlock(codepoint, (const char *)blockName); break;
	}
	if(neg)
		return (!ret);
	return ret;
}

static int xmlRegCheckCharacter(xmlRegAtom * atom, int codepoint) 
{
	int i, ret = 0;
	xmlRegRange * range;
	if((atom == NULL) || (!IS_CHAR(codepoint)))
		return -1;
	switch(atom->type) {
		case XML_REGEXP_SUBREG:
		case XML_REGEXP_EPSILON:
		    return -1;
		case XML_REGEXP_CHARVAL:
		    return (codepoint == atom->codepoint);
		case XML_REGEXP_RANGES: {
		    int accept = 0;
		    for(i = 0; i < atom->nbRanges; i++) {
			    range = atom->ranges[i];
			    if(range->neg == 2) {
				    ret = xmlRegCheckCharacterRange(range->type, codepoint, 0, range->start, range->end, range->blockName);
				    if(ret != 0)
					    return 0;  /* excluded char */
			    }
			    else if(range->neg) {
				    ret = xmlRegCheckCharacterRange(range->type, codepoint, 0, range->start, range->end, range->blockName);
				    if(!ret)
					    accept = 1;
				    else
					    return 0;
			    }
			    else {
				    ret = xmlRegCheckCharacterRange(range->type, codepoint, 0, range->start, range->end, range->blockName);
				    if(ret != 0)
					    accept = 1;  /* might still be excluded */
			    }
		    }
		    return (accept);
	    }
		case XML_REGEXP_STRING:
		    printf("TODO: XML_REGEXP_STRING\n");
		    return -1;
		case XML_REGEXP_ANYCHAR:
		case XML_REGEXP_ANYSPACE:
		case XML_REGEXP_NOTSPACE:
		case XML_REGEXP_INITNAME:
		case XML_REGEXP_NOTINITNAME:
		case XML_REGEXP_NAMECHAR:
		case XML_REGEXP_NOTNAMECHAR:
		case XML_REGEXP_DECIMAL:
		case XML_REGEXP_NOTDECIMAL:
		case XML_REGEXP_REALCHAR:
		case XML_REGEXP_NOTREALCHAR:
		case XML_REGEXP_LETTER:
		case XML_REGEXP_LETTER_UPPERCASE:
		case XML_REGEXP_LETTER_LOWERCASE:
		case XML_REGEXP_LETTER_TITLECASE:
		case XML_REGEXP_LETTER_MODIFIER:
		case XML_REGEXP_LETTER_OTHERS:
		case XML_REGEXP_MARK:
		case XML_REGEXP_MARK_NONSPACING:
		case XML_REGEXP_MARK_SPACECOMBINING:
		case XML_REGEXP_MARK_ENCLOSING:
		case XML_REGEXP_NUMBER:
		case XML_REGEXP_NUMBER_DECIMAL:
		case XML_REGEXP_NUMBER_LETTER:
		case XML_REGEXP_NUMBER_OTHERS:
		case XML_REGEXP_PUNCT:
		case XML_REGEXP_PUNCT_CONNECTOR:
		case XML_REGEXP_PUNCT_DASH:
		case XML_REGEXP_PUNCT_OPEN:
		case XML_REGEXP_PUNCT_CLOSE:
		case XML_REGEXP_PUNCT_INITQUOTE:
		case XML_REGEXP_PUNCT_FINQUOTE:
		case XML_REGEXP_PUNCT_OTHERS:
		case XML_REGEXP_SEPAR:
		case XML_REGEXP_SEPAR_SPACE:
		case XML_REGEXP_SEPAR_LINE:
		case XML_REGEXP_SEPAR_PARA:
		case XML_REGEXP_SYMBOL:
		case XML_REGEXP_SYMBOL_MATH:
		case XML_REGEXP_SYMBOL_CURRENCY:
		case XML_REGEXP_SYMBOL_MODIFIER:
		case XML_REGEXP_SYMBOL_OTHERS:
		case XML_REGEXP_OTHER:
		case XML_REGEXP_OTHER_CONTROL:
		case XML_REGEXP_OTHER_FORMAT:
		case XML_REGEXP_OTHER_PRIVATE:
		case XML_REGEXP_OTHER_NA:
		case XML_REGEXP_BLOCK_NAME:
		    ret = xmlRegCheckCharacterRange(atom->type, codepoint, 0, 0, 0, (const xmlChar *)atom->valuep);
		    if(atom->neg)
			    ret = !ret;
		    break;
	}
	return ret;
}

/************************************************************************
*									*
*	Saving and restoring state of an execution context		*
*									*
************************************************************************/

#ifdef DEBUG_REGEXP_EXEC
static void xmlFARegDebugExec(xmlRegExecCtxtPtr exec) 
{
	printf("state: %d:%d:idx %d", exec->state->no, exec->transno, exec->index);
	if(exec->inputStack) {
		int i;
		printf(": ");
		for(i = 0; (i < 3) && (i < exec->inputStackNr); i++)
			printf("%s ", (const char *)exec->inputStack[exec->inputStackNr - (i + 1)].value);
	}
	else {
		printf(": %s", &(exec->inputString[exec->index]));
	}
	printf("\n");
}
#endif

static void xmlFARegExecSave(xmlRegExecCtxtPtr exec) 
{
#ifdef DEBUG_REGEXP_EXEC
	printf("saving ");
	exec->transno++;
	xmlFARegDebugExec(exec);
	exec->transno--;
#endif
#ifdef MAX_PUSH
	if(exec->nbPush > MAX_PUSH) {
		return;
	}
	exec->nbPush++;
#endif

	if(exec->maxRollbacks == 0) {
		exec->maxRollbacks = 4;
		exec->rollbacks = (xmlRegExecRollback*)SAlloc::M(exec->maxRollbacks *
		    sizeof(xmlRegExecRollback));
		if(exec->rollbacks == NULL) {
			xmlRegexpErrMemory(NULL, "saving regexp");
			exec->maxRollbacks = 0;
			return;
		}
		memzero(exec->rollbacks, exec->maxRollbacks * sizeof(xmlRegExecRollback));
	}
	else if(exec->nbRollbacks >= exec->maxRollbacks) {
		xmlRegExecRollback * tmp;
		int len = exec->maxRollbacks;
		exec->maxRollbacks *= 2;
		tmp = (xmlRegExecRollback*)SAlloc::R(exec->rollbacks, exec->maxRollbacks * sizeof(xmlRegExecRollback));
		if(!tmp) {
			xmlRegexpErrMemory(NULL, "saving regexp");
			exec->maxRollbacks /= 2;
			return;
		}
		exec->rollbacks = tmp;
		tmp = &exec->rollbacks[len];
		memzero(tmp, (exec->maxRollbacks - len) * sizeof(xmlRegExecRollback));
	}
	exec->rollbacks[exec->nbRollbacks].state = exec->state;
	exec->rollbacks[exec->nbRollbacks].index = exec->index;
	exec->rollbacks[exec->nbRollbacks].nextbranch = exec->transno + 1;
	if(exec->comp->nbCounters > 0) {
		if(exec->rollbacks[exec->nbRollbacks].counts == NULL) {
			exec->rollbacks[exec->nbRollbacks].counts = (int *)SAlloc::M(exec->comp->nbCounters * sizeof(int));
			if(exec->rollbacks[exec->nbRollbacks].counts == NULL) {
				xmlRegexpErrMemory(NULL, "saving regexp");
				exec->status = -5;
				return;
			}
		}
		memcpy(exec->rollbacks[exec->nbRollbacks].counts, exec->counts, exec->comp->nbCounters * sizeof(int));
	}
	exec->nbRollbacks++;
}

static void FASTCALL xmlFARegExecRollBack(xmlRegExecCtxtPtr exec) 
{
	if(exec->nbRollbacks <= 0) {
		exec->status = -1;
#ifdef DEBUG_REGEXP_EXEC
		printf("rollback failed on empty stack\n");
#endif
		return;
	}
	exec->nbRollbacks--;
	exec->state = exec->rollbacks[exec->nbRollbacks].state;
	exec->index = exec->rollbacks[exec->nbRollbacks].index;
	exec->transno = exec->rollbacks[exec->nbRollbacks].nextbranch;
	if(exec->comp->nbCounters > 0) {
		if(exec->rollbacks[exec->nbRollbacks].counts == NULL) {
			fprintf(stderr, "exec save: allocation failed");
			exec->status = -6;
			return;
		}
		if(exec->counts) {
			memcpy(exec->counts, exec->rollbacks[exec->nbRollbacks].counts, exec->comp->nbCounters * sizeof(int));
		}
	}
#ifdef DEBUG_REGEXP_EXEC
	printf("restored ");
	xmlFARegDebugExec(exec);
#endif
}
//
// Verifier, running an input against a compiled regexp
// 
static int xmlFARegExec(xmlRegexp * comp, const xmlChar * content) 
{
	xmlRegExecCtxt execval;
	xmlRegExecCtxtPtr exec = &execval;
	int ret, codepoint = 0, len, deter;
	exec->inputString = content;
	exec->index = 0;
	exec->nbPush = 0;
	exec->determinist = 1;
	exec->maxRollbacks = 0;
	exec->nbRollbacks = 0;
	exec->rollbacks = NULL;
	exec->status = 0;
	exec->comp = comp;
	exec->state = comp->states[0];
	exec->transno = 0;
	exec->transcount = 0;
	exec->inputStack = NULL;
	exec->inputStackMax = 0;
	if(comp->nbCounters > 0) {
		exec->counts = (int *)SAlloc::M(comp->nbCounters * sizeof(int));
		if(exec->counts == NULL) {
			xmlRegexpErrMemory(NULL, "running regexp");
			return -1;
		}
		memzero(exec->counts, comp->nbCounters * sizeof(int));
	}
	else
		exec->counts = NULL;
	while(!exec->status && exec->state && (exec->inputString[exec->index] || (exec->state && (exec->state->type != XML_REGEXP_FINAL_STATE)))) {
		xmlRegTransPtr trans;
		xmlRegAtom * atom;
		/*
		 * If end of input on non-terminal state, rollback, however we may
		 * still have epsilon like transition for counted transitions
		 * on counters, in that case don't break too early.  Additionally,
		 * if we are working on a range like "AB{0,2}", where B is not present,
		 * we don't want to break.
		 */
		len = 1;
		if((exec->inputString[exec->index] == 0) && (exec->counts == NULL)) {
			/*
			 * if there is a transition, we must check if
			 *  atom allows minOccurs of 0
			 */
			if(exec->transno < exec->state->nbTrans) {
				trans = &exec->state->trans[exec->transno];
				if(trans->to >=0) {
					atom = trans->atom;
					if(!((atom->min == 0) && (atom->max > 0)))
						goto rollback;
				}
			}
			else
				goto rollback;
		}
		exec->transcount = 0;
		for(; exec->transno < exec->state->nbTrans; exec->transno++) {
			trans = &exec->state->trans[exec->transno];
			if(trans->to < 0)
				continue;
			atom = trans->atom;
			ret = 0;
			deter = 1;
			if(trans->count >= 0) {
				int count;
				xmlRegCounter * counter;
				if(exec->counts == NULL) {
					exec->status = -1;
					goto error;
				}
				/*
				 * A counted transition.
				 */
				count = exec->counts[trans->count];
				counter = &exec->comp->counters[trans->count];
#ifdef DEBUG_REGEXP_EXEC
				printf("testing count %d: val %d, min %d, max %d\n", trans->count, count, counter->min,  counter->max);
#endif
				ret = ((count >= counter->min) && (count <= counter->max));
				if((ret) && (counter->min != counter->max))
					deter = 0;
			}
			else if(atom == NULL) {
				fprintf(stderr, "epsilon transition left at runtime\n");
				exec->status = -2;
				break;
			}
			else if(exec->inputString[exec->index] != 0) {
				codepoint = CUR_SCHAR(&(exec->inputString[exec->index]), len);
				ret = xmlRegCheckCharacter(atom, codepoint);
				if((ret == 1) && (atom->min >= 0) && (atom->max > 0)) {
					xmlRegStatePtr to = comp->states[trans->to];
					/*
					 * this is a multiple input sequence
					 * If there is a counter associated increment it now.
					 * before potentially saving and rollback
					 * do not increment if the counter is already over the
					 * maximum limit in which case get to next transition
					 */
					if(trans->counter >= 0) {
						xmlRegCounter * counter;
						if((exec->counts == NULL) || (exec->comp == NULL) || (exec->comp->counters == NULL)) {
							exec->status = -1;
							goto error;
						}
						counter = &exec->comp->counters[trans->counter];
						if(exec->counts[trans->counter] >= counter->max)
							continue;  /* for loop on transitions */

#ifdef DEBUG_REGEXP_EXEC
						printf("Increasing count %d\n", trans->counter);
#endif
						exec->counts[trans->counter]++;
					}
					if(exec->state->nbTrans > exec->transno + 1) {
						xmlFARegExecSave(exec);
					}
					exec->transcount = 1;
					do {
						/*
						 * Try to progress as much as possible on the input
						 */
						if(exec->transcount == atom->max) {
							break;
						}
						exec->index += len;
						/*
						 * End of input: stop here
						 */
						if(exec->inputString[exec->index] == 0) {
							exec->index -= len;
							break;
						}
						if(exec->transcount >= atom->min) {
							int transno = exec->transno;
							xmlRegStatePtr state = exec->state;
							/*
							 * The transition is acceptable save it
							 */
							exec->transno = -1; /* trick */
							exec->state = to;
							xmlFARegExecSave(exec);
							exec->transno = transno;
							exec->state = state;
						}
						codepoint = CUR_SCHAR(&(exec->inputString[exec->index]), len);
						ret = xmlRegCheckCharacter(atom, codepoint);
						exec->transcount++;
					} while(ret == 1);
					if(exec->transcount < atom->min)
						ret = 0;

					/*
					 * If the last check failed but one transition was found
					 * possible, rollback
					 */
					if(ret < 0)
						ret = 0;
					if(!ret) {
						goto rollback;
					}
					if(trans->counter >= 0) {
						if(exec->counts == NULL) {
							exec->status = -1;
							goto error;
						}
#ifdef DEBUG_REGEXP_EXEC
						printf("Decreasing count %d\n", trans->counter);
#endif
						exec->counts[trans->counter]--;
					}
				}
				else if((ret == 0) && (atom->min == 0) && (atom->max > 0)) {
					/*
					 * we don't match on the codepoint, but minOccurs of 0
					 * says that's ok.  Setting len to 0 inhibits stepping
					 * over the codepoint.
					 */
					exec->transcount = 1;
					len = 0;
					ret = 1;
				}
			}
			else if((atom->min == 0) && (atom->max > 0)) {
				/* another spot to match when minOccurs is 0 */
				exec->transcount = 1;
				len = 0;
				ret = 1;
			}
			if(ret == 1) {
				if((trans->nd == 1) ||
				    ((trans->count >= 0) && (deter == 0) &&
					    (exec->state->nbTrans > exec->transno + 1))) {
#ifdef DEBUG_REGEXP_EXEC
					if(trans->nd == 1)
						printf("Saving on nd transition atom %d for %c at %d\n",
						    trans->atom->no, codepoint, exec->index);
					else
						printf("Saving on counted transition count %d for %c at %d\n",
						    trans->count, codepoint, exec->index);
#endif
					xmlFARegExecSave(exec);
				}
				if(trans->counter >= 0) {
					xmlRegCounter * counter;
					/* make sure we don't go over the counter maximum value */
					if((exec->counts == NULL) || (exec->comp == NULL) || (exec->comp->counters == NULL)) {
						exec->status = -1;
						goto error;
					}
					counter = &exec->comp->counters[trans->counter];
					if(exec->counts[trans->counter] >= counter->max)
						continue;  /* for loop on transitions */
#ifdef DEBUG_REGEXP_EXEC
					printf("Increasing count %d\n", trans->counter);
#endif
					exec->counts[trans->counter]++;
				}
				if((trans->count >= 0) &&
				    (trans->count < REGEXP_ALL_COUNTER)) {
					if(exec->counts == NULL) {
						exec->status = -1;
						goto error;
					}
#ifdef DEBUG_REGEXP_EXEC
					printf("resetting count %d on transition\n",
					    trans->count);
#endif
					exec->counts[trans->count] = 0;
				}
#ifdef DEBUG_REGEXP_EXEC
				printf("entering state %d\n", trans->to);
#endif
				exec->state = comp->states[trans->to];
				exec->transno = 0;
				if(trans->atom) {
					exec->index += len;
				}
				goto progress;
			}
			else if(ret < 0) {
				exec->status = -4;
				break;
			}
		}
		if((exec->transno != 0) || (exec->state->nbTrans == 0)) {
rollback:
			/*
			 * Failed to find a way out
			 */
			exec->determinist = 0;
#ifdef DEBUG_REGEXP_EXEC
			printf("rollback from state %d on %d:%c\n", exec->state->no,
			    codepoint, codepoint);
#endif
			xmlFARegExecRollBack(exec);
		}
progress:
		continue;
	}
error:
	if(exec->rollbacks) {
		if(exec->counts) {
			for(int i = 0; i < exec->maxRollbacks; i++)
				SAlloc::F(exec->rollbacks[i].counts);
		}
		SAlloc::F(exec->rollbacks);
	}
	if(exec->state == NULL)
		return -1;
	SAlloc::F(exec->counts);
	if(exec->status == 0)
		return 1;
	if(exec->status == -1) {
		if(exec->nbPush > MAX_PUSH)
			return -1;
		return 0;
	}
	return (exec->status);
}

/************************************************************************
*									*
*	Progressive interface to the verifier one atom at a time	*
*									*
************************************************************************/
#ifdef DEBUG_ERR
static void testerr(xmlRegExecCtxtPtr exec);
#endif

/**
 * xmlRegNewExecCtxt:
 * @comp: a precompiled regular expression
 * @callback: a callback function used for handling progresses in the
 *       automata matching phase
 * @data: the context data associated to the callback in this context
 *
 * Build a context used for progressive evaluation of a regexp.
 *
 * Returns the new context
 */
xmlRegExecCtxtPtr xmlRegNewExecCtxt(xmlRegexp * comp, xmlRegExecCallbacks callback, void * data)
{
	xmlRegExecCtxtPtr exec;
	if(comp == NULL)
		return 0;
	if((comp->compact == NULL) && (comp->states == NULL))
		return 0;
	exec = static_cast<xmlRegExecCtxtPtr>(SAlloc::M(sizeof(xmlRegExecCtxt)));
	if(exec == NULL) {
		xmlRegexpErrMemory(NULL, "creating execution context");
		return 0;
	}
	memzero(exec, sizeof(xmlRegExecCtxt));
	exec->inputString = NULL;
	exec->index = 0;
	exec->determinist = 1;
	exec->maxRollbacks = 0;
	exec->nbRollbacks = 0;
	exec->rollbacks = NULL;
	exec->status = 0;
	exec->comp = comp;
	if(comp->compact == NULL)
		exec->state = comp->states[0];
	exec->transno = 0;
	exec->transcount = 0;
	exec->callback = callback;
	exec->data = data;
	if(comp->nbCounters > 0) {
		/*
		 * For error handling, exec->counts is allocated twice the size
		 * the second half is used to store the data in case of rollback
		 */
		exec->counts = (int *)SAlloc::M(comp->nbCounters * sizeof(int) * 2);
		if(exec->counts == NULL) {
			xmlRegexpErrMemory(NULL, "creating execution context");
			SAlloc::F(exec);
			return 0;
		}
		memzero(exec->counts, comp->nbCounters * sizeof(int) * 2);
		exec->errCounts = &exec->counts[comp->nbCounters];
	}
	else {
		exec->counts = NULL;
		exec->errCounts = NULL;
	}
	exec->inputStackMax = 0;
	exec->inputStackNr = 0;
	exec->inputStack = NULL;
	exec->errStateNo = -1;
	exec->errString = NULL;
	exec->nbPush = 0;
	return (exec);
}

/**
 * xmlRegFreeExecCtxt:
 * @exec: a regular expression evaulation context
 *
 * Free the structures associated to a regular expression evaulation context.
 */
void xmlRegFreeExecCtxt(xmlRegExecCtxtPtr exec)
{
	if(exec) {
		if(exec->rollbacks) {
			if(exec->counts) {
				for(int i = 0; i < exec->maxRollbacks; i++)
					SAlloc::F(exec->rollbacks[i].counts);
			}
			SAlloc::F(exec->rollbacks);
		}
		SAlloc::F(exec->counts);
		if(exec->inputStack) {
			for(int i = 0; i < exec->inputStackNr; i++) {
				SAlloc::F(exec->inputStack[i].value);
			}
			SAlloc::F(exec->inputStack);
		}
		SAlloc::F(exec->errString);
		SAlloc::F(exec);
	}
}

static void xmlFARegExecSaveInputString(xmlRegExecCtxtPtr exec, const xmlChar * value, void * data)
{
#ifdef DEBUG_PUSH
	printf("saving value: %d:%s\n", exec->inputStackNr, value);
#endif
	if(exec->inputStackMax == 0) {
		exec->inputStackMax = 4;
		exec->inputStack = (xmlRegInputTokenPtr)SAlloc::M(exec->inputStackMax * sizeof(xmlRegInputToken));
		if(exec->inputStack == NULL) {
			xmlRegexpErrMemory(NULL, "pushing input string");
			exec->inputStackMax = 0;
			return;
		}
	}
	else if(exec->inputStackNr + 1 >= exec->inputStackMax) {
		xmlRegInputTokenPtr tmp;
		exec->inputStackMax *= 2;
		tmp = (xmlRegInputTokenPtr)SAlloc::R(exec->inputStack, exec->inputStackMax * sizeof(xmlRegInputToken));
		if(!tmp) {
			xmlRegexpErrMemory(NULL, "pushing input string");
			exec->inputStackMax /= 2;
			return;
		}
		exec->inputStack = tmp;
	}
	exec->inputStack[exec->inputStackNr].value = sstrdup(value);
	exec->inputStack[exec->inputStackNr].data = data;
	exec->inputStackNr++;
	exec->inputStack[exec->inputStackNr].value = NULL;
	exec->inputStack[exec->inputStackNr].data = NULL;
}

/**
 * xmlRegStrEqualWildcard:
 * @expStr:  the string to be evaluated
 * @valStr:  the validation string
 *
 * Checks if both strings are equal or have the same content. "*"
 * can be used as a wildcard in @valStr; "|" is used as a seperator of
 * substrings in both @expStr and @valStr.
 *
 * Returns 1 if the comparison is satisfied and the number of substrings
 * is equal, 0 otherwise.
 */

static int xmlRegStrEqualWildcard(const xmlChar * expStr, const xmlChar * valStr) {
	if(expStr == valStr) return 1;
	if(expStr == NULL) return 0;
	if(valStr == NULL) return 0;
	do {
		/*
		 * Eval if we have a wildcard for the current item.
		 */
		if(*expStr != *valStr) {
			/* if one of them starts with a wildcard make valStr be it */
			if(*valStr == '*') {
				const xmlChar * tmp;

				tmp = valStr;
				valStr = expStr;
				expStr = tmp;
			}
			if((*valStr != 0) && (*expStr != 0) && (*expStr++ == '*')) {
				do {
					if(*valStr == XML_REG_STRING_SEPARATOR)
						break;
					valStr++;
				} while(*valStr != 0);
				continue;
			}
			else
				return 0;
		}
		expStr++;
		valStr++;
	} while(*valStr != 0);
	if(*expStr != 0)
		return 0;
	else
		return 1;
}
/**
 * xmlRegCompactPushString:
 * @exec: a regexp execution context
 * @comp:  the precompiled exec with a compact table
 * @value: a string token input
 * @data: data associated to the token to reuse in callbacks
 *
 * Push one input token in the execution context
 *
 * Returns: 1 if the regexp reached a final state, 0 if non-final, and
 *   a negative value in case of error.
 */
static int xmlRegCompactPushString(xmlRegExecCtxtPtr exec, xmlRegexp * comp, const xmlChar * value, void * data) 
{
	int state = exec->index;
	int i, target;
	if((comp == NULL) || (comp->compact == NULL) || (comp->stringMap == NULL))
		return -1;
	if(!value) {
		/*
		 * are we at a final state ?
		 */
		if(comp->compact[state * (comp->nbstrings + 1)] == XML_REGEXP_FINAL_STATE)
			return 1;
		return 0;
	}

#ifdef DEBUG_PUSH
	printf("value pushed: %s\n", value);
#endif

	/*
	 * Examine all outside transitions from current state
	 */
	for(i = 0; i < comp->nbstrings; i++) {
		target = comp->compact[state * (comp->nbstrings + 1) + i + 1];
		if((target > 0) && (target <= comp->nbstates)) {
			target--; /* to avoid 0 */
			if(xmlRegStrEqualWildcard(comp->stringMap[i], value)) {
				exec->index = target;
				if(exec->callback && comp->transdata) {
					exec->callback((xmlRegExecCtxtPtr)exec->data, value, comp->transdata[state * comp->nbstrings + i], data);
				}
#ifdef DEBUG_PUSH
				printf("entering state %d\n", target);
#endif
				if(comp->compact[target * (comp->nbstrings + 1)] == XML_REGEXP_SINK_STATE)
					goto error;
				if(comp->compact[target * (comp->nbstrings + 1)] == XML_REGEXP_FINAL_STATE)
					return 1;
				return 0;
			}
		}
	}
	/*
	 * Failed to find an exit transition out from current state for the
	 * current token
	 */
#ifdef DEBUG_PUSH
	printf("failed to find a transition for %s on state %d\n", value, state);
#endif
error:
	SAlloc::F(exec->errString);
	exec->errString = sstrdup(value);
	exec->errStateNo = state;
	exec->status = -1;
#ifdef DEBUG_ERR
	testerr(exec);
#endif
	return -1;
}

/**
 * xmlRegExecPushStringInternal:
 * @exec: a regexp execution context or NULL to indicate the end
 * @value: a string token input
 * @data: data associated to the token to reuse in callbacks
 * @compound: value was assembled from 2 strings
 *
 * Push one input token in the execution context
 *
 * Returns: 1 if the regexp reached a final state, 0 if non-final, and
 *   a negative value in case of error.
 */
static int xmlRegExecPushStringInternal(xmlRegExecCtxtPtr exec, const xmlChar * value, void * data, int compound) 
{
	xmlRegTransPtr trans;
	xmlRegAtom * atom;
	int ret;
	int final = 0;
	int progress = 1;
	if(exec == NULL)
		return -1;
	if(exec->comp == NULL)
		return -1;
	if(exec->status != 0)
		return (exec->status);
	if(exec->comp->compact)
		return (xmlRegCompactPushString(exec, exec->comp, value, data));
	if(!value) {
		if(exec->state->type == XML_REGEXP_FINAL_STATE)
			return 1;
		final = 1;
	}
#ifdef DEBUG_PUSH
	printf("value pushed: %s\n", value);
#endif
	/*
	 * If we have an active rollback stack push the new value there
	 * and get back to where we were left
	 */
	if(value && (exec->inputStackNr > 0)) {
		xmlFARegExecSaveInputString(exec, value, data);
		value = exec->inputStack[exec->index].value;
		data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
		printf("value loaded: %s\n", value);
#endif
	}

	while((exec->status == 0) && (value || ((final == 1) && (exec->state->type != XML_REGEXP_FINAL_STATE)))) {
		/*
		 * End of input on non-terminal state, rollback, however we may
		 * still have epsilon like transition for counted transitions
		 * on counters, in that case don't break too early.
		 */
		if((value == NULL) && (exec->counts == NULL))
			goto rollback;
		exec->transcount = 0;
		for(; exec->transno < exec->state->nbTrans; exec->transno++) {
			trans = &exec->state->trans[exec->transno];
			if(trans->to < 0)
				continue;
			atom = trans->atom;
			ret = 0;
			if(trans->count == REGEXP_ALL_LAX_COUNTER) {
				int i;
				int count;
				xmlRegTransPtr t;
				xmlRegCounter * counter;
				ret = 0;
#ifdef DEBUG_PUSH
				printf("testing all lax %d\n", trans->count);
#endif
				/*
				 * Check all counted transitions from the current state
				 */
				if((value == NULL) && (final)) {
					ret = 1;
				}
				else if(value) {
					for(i = 0; i < exec->state->nbTrans; i++) {
						t = &exec->state->trans[i];
						if((t->counter < 0) || (t == trans))
							continue;
						counter = &exec->comp->counters[t->counter];
						count = exec->counts[t->counter];
						if((count < counter->max) && t->atom && (sstreq(value, (xmlChar *)t->atom->valuep))) {
							ret = 0;
							break;
						}
						if((count >= counter->min) && (count < counter->max) && t->atom && (sstreq(value, (xmlChar *)t->atom->valuep))) {
							ret = 1;
							break;
						}
					}
				}
			}
			else if(trans->count == REGEXP_ALL_COUNTER) {
				int i;
				int count;
				xmlRegTransPtr t;
				xmlRegCounter * counter;
				ret = 1;
#ifdef DEBUG_PUSH
				printf("testing all %d\n", trans->count);
#endif
				/*
				 * Check all counted transitions from the current state
				 */
				for(i = 0; i < exec->state->nbTrans; i++) {
					t = &exec->state->trans[i];
					if((t->counter < 0) || (t == trans))
						continue;
					counter = &exec->comp->counters[t->counter];
					count = exec->counts[t->counter];
					if((count < counter->min) || (count > counter->max)) {
						ret = 0;
						break;
					}
				}
			}
			else if(trans->count >= 0) {
				/*
				 * A counted transition.
				 */
				int count = exec->counts[trans->count];
				xmlRegCounter * counter = &exec->comp->counters[trans->count];
#ifdef DEBUG_PUSH
				printf("testing count %d: val %d, min %d, max %d\n",
				    trans->count, count, counter->min,  counter->max);
#endif
				ret = ((count >= counter->min) && (count <= counter->max));
			}
			else if(atom == NULL) {
				fprintf(stderr, "epsilon transition left at runtime\n");
				exec->status = -2;
				break;
			}
			else if(value) {
				ret = xmlRegStrEqualWildcard((xmlChar *)atom->valuep, value);
				if(atom->neg) {
					ret = !ret;
					if(!compound)
						ret = 0;
				}
				if((ret == 1) && (trans->counter >= 0)) {
					int count = exec->counts[trans->counter];
					xmlRegCounter * counter = &exec->comp->counters[trans->counter];
					if(count >= counter->max)
						ret = 0;
				}
				if((ret == 1) && (atom->min > 0) && (atom->max > 0)) {
					xmlRegStatePtr to = exec->comp->states[trans->to];
					/*
					 * this is a multiple input sequence
					 */
					if(exec->state->nbTrans > exec->transno + 1) {
						if(exec->inputStackNr <= 0) {
							xmlFARegExecSaveInputString(exec, value, data);
						}
						xmlFARegExecSave(exec);
					}
					exec->transcount = 1;
					do {
						/*
						 * Try to progress as much as possible on the input
						 */
						if(exec->transcount == atom->max) {
							break;
						}
						exec->index++;
						value = exec->inputStack[exec->index].value;
						data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
						printf("value loaded: %s\n", value);
#endif
						/*
						 * End of input: stop here
						 */
						if(!value) {
							exec->index--;
							break;
						}
						if(exec->transcount >= atom->min) {
							int transno = exec->transno;
							xmlRegStatePtr state = exec->state;
							/*
							 * The transition is acceptable save it
							 */
							exec->transno = -1; /* trick */
							exec->state = to;
							if(exec->inputStackNr <= 0) {
								xmlFARegExecSaveInputString(exec, value, data);
							}
							xmlFARegExecSave(exec);
							exec->transno = transno;
							exec->state = state;
						}
						ret = sstreq(value, (xmlChar *)atom->valuep);
						exec->transcount++;
					} while(ret == 1);
					if(exec->transcount < atom->min)
						ret = 0;
					/*
					 * If the last check failed but one transition was found
					 * possible, rollback
					 */
					if(ret < 0)
						ret = 0;
					if(!ret) {
						goto rollback;
					}
				}
			}
			if(ret == 1) {
				if(exec->callback && atom && data) {
					exec->callback((xmlRegExecCtxtPtr)exec->data, (const xmlChar *)atom->valuep, atom->data, data);
				}
				if(exec->state->nbTrans > exec->transno + 1) {
					if(exec->inputStackNr <= 0) {
						xmlFARegExecSaveInputString(exec, value, data);
					}
					xmlFARegExecSave(exec);
				}
				if(trans->counter >= 0) {
#ifdef DEBUG_PUSH
					printf("Increasing count %d\n", trans->counter);
#endif
					exec->counts[trans->counter]++;
				}
				if((trans->count >= 0) && (trans->count < REGEXP_ALL_COUNTER)) {
#ifdef DEBUG_REGEXP_EXEC
					printf("resetting count %d on transition\n", trans->count);
#endif
					exec->counts[trans->count] = 0;
				}
#ifdef DEBUG_PUSH
				printf("entering state %d\n", trans->to);
#endif
				if(exec->comp->states[trans->to] && (exec->comp->states[trans->to]->type == XML_REGEXP_SINK_STATE)) {
					// 
					// entering a sink state, save the current state as error state.
					// 
					SAlloc::F(exec->errString);
					exec->errString = sstrdup(value);
					exec->errState = exec->state;
					memcpy(exec->errCounts, exec->counts, exec->comp->nbCounters * sizeof(int));
				}
				exec->state = exec->comp->states[trans->to];
				exec->transno = 0;
				if(trans->atom) {
					if(exec->inputStack) {
						exec->index++;
						if(exec->index < exec->inputStackNr) {
							value = exec->inputStack[exec->index].value;
							data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
							printf("value loaded: %s\n", value);
#endif
						}
						else {
							value = NULL;
							data = NULL;
#ifdef DEBUG_PUSH
							printf("end of input\n");
#endif
						}
					}
					else {
						value = NULL;
						data = NULL;
#ifdef DEBUG_PUSH
						printf("end of input\n");
#endif
					}
				}
				goto progress;
			}
			else if(ret < 0) {
				exec->status = -4;
				break;
			}
		}
		if((exec->transno != 0) || (exec->state->nbTrans == 0)) {
rollback:
			/*
			 * if we didn't yet rollback on the current input
			 * store the current state as the error state.
			 */
			if((progress) && exec->state && (exec->state->type != XML_REGEXP_SINK_STATE)) {
				progress = 0;
				SAlloc::F(exec->errString);
				exec->errString = sstrdup(value);
				exec->errState = exec->state;
				memcpy(exec->errCounts, exec->counts, exec->comp->nbCounters * sizeof(int));
			}
			/*
			 * Failed to find a way out
			 */
			exec->determinist = 0;
			xmlFARegExecRollBack(exec);
			if(exec->inputStack && (exec->status == 0)) {
				value = exec->inputStack[exec->index].value;
				data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
				printf("value loaded: %s\n", value);
#endif
			}
		}
		continue;
progress:
		progress = 1;
		continue;
	}
	if(exec->status == 0) {
		return (exec->state->type == XML_REGEXP_FINAL_STATE);
	}
#ifdef DEBUG_ERR
	if(exec->status < 0) {
		testerr(exec);
	}
#endif
	return (exec->status);
}

/**
 * xmlRegExecPushString:
 * @exec: a regexp execution context or NULL to indicate the end
 * @value: a string token input
 * @data: data associated to the token to reuse in callbacks
 *
 * Push one input token in the execution context
 *
 * Returns: 1 if the regexp reached a final state, 0 if non-final, and
 *   a negative value in case of error.
 */
int xmlRegExecPushString(xmlRegExecCtxtPtr exec, const xmlChar * value, void * data)
{
	return xmlRegExecPushStringInternal(exec, value, data, 0);
}

/**
 * xmlRegExecPushString2:
 * @exec: a regexp execution context or NULL to indicate the end
 * @value: the first string token input
 * @value2: the second string token input
 * @data: data associated to the token to reuse in callbacks
 *
 * Push one input token in the execution context
 *
 * Returns: 1 if the regexp reached a final state, 0 if non-final, and
 *   a negative value in case of error.
 */
int xmlRegExecPushString2(xmlRegExecCtxtPtr exec, const xmlChar * value, const xmlChar * value2, void * data)
{
	int    ret = -1;
	xmlChar buf[256];
	xmlChar * str = 0;
	if(exec && exec->comp) {
		if(exec->status)
			ret = exec->status;
		else if(!value2)
			ret = xmlRegExecPushString(exec, value, data);
		else {
			const size_t lenn = sstrlen(value2);
			const size_t lenp = sstrlen(value);
			if((lenn + lenp + 2) > sizeof(buf)) {
				str = static_cast<xmlChar *>(SAlloc::M(lenn + lenp + 2));
				if(!str) {
					exec->status = -1;
					return -1;
				}
			}
			else {
				str = buf;
			}
			memcpy(&str[0], value, lenp);
			str[lenp] = XML_REG_STRING_SEPARATOR;
			memcpy(&str[lenp + 1], value2, lenn);
			str[lenn + lenp + 1] = 0;
			if(exec->comp->compact)
				ret = xmlRegCompactPushString(exec, exec->comp, str, data);
			else
				ret = xmlRegExecPushStringInternal(exec, str, data, 1);
			if(str != buf)
				SAlloc::F(str);
		}
	}
	return ret;
}

/**
 * xmlRegExecGetValues:
 * @exec: a regexp execution context
 * @err: error extraction or normal one
 * @nbval: pointer to the number of accepted values IN/OUT
 * @nbneg: return number of negative transitions
 * @values: pointer to the array of acceptable values
 * @terminal: return value if this was a terminal state
 *
 * Extract informations from the regexp execution, internal routine to
 * implement xmlRegExecNextValues() and xmlRegExecErrInfo()
 *
 * Returns: 0 in case of success or -1 in case of error.
 */
static int xmlRegExecGetValues(xmlRegExecCtxtPtr exec, int err, int * nbval, int * nbneg, xmlChar ** values, int * terminal) 
{
	int maxval;
	int nb = 0;
	if((exec == NULL) || (nbval == NULL) || (nbneg == NULL) || (values == NULL) || (*nbval <= 0))
		return -1;
	maxval = *nbval;
	*nbval = 0;
	*nbneg = 0;
	if(exec->comp && exec->comp->compact) {
		int target, i, state;
		xmlRegexp * comp = exec->comp;
		if(err) {
			if(exec->errStateNo == -1) 
				return -1;
			state = exec->errStateNo;
		}
		else {
			state = exec->index;
		}
		if(terminal) {
			if(comp->compact[state * (comp->nbstrings + 1)] == XML_REGEXP_FINAL_STATE)
				*terminal = 1;
			else
				*terminal = 0;
		}
		for(i = 0; (i < comp->nbstrings) && (nb < maxval); i++) {
			target = comp->compact[state * (comp->nbstrings + 1) + i + 1];
			if((target > 0) && (target <= comp->nbstates) && (comp->compact[(target - 1) * (comp->nbstrings + 1)] != XML_REGEXP_SINK_STATE)) {
				values[nb++] = comp->stringMap[i];
				(*nbval)++;
			}
		}
		for(i = 0; (i < comp->nbstrings) && (nb < maxval); i++) {
			target = comp->compact[state * (comp->nbstrings + 1) + i + 1];
			if((target > 0) && (target <= comp->nbstates) && (comp->compact[(target - 1) * (comp->nbstrings + 1)] == XML_REGEXP_SINK_STATE)) {
				values[nb++] = comp->stringMap[i];
				(*nbneg)++;
			}
		}
	}
	else {
		int transno;
		xmlRegTransPtr trans;
		xmlRegAtom * atom;
		xmlRegStatePtr state;
		if(terminal) {
			if(exec->state->type == XML_REGEXP_FINAL_STATE)
				*terminal = 1;
			else
				*terminal = 0;
		}
		if(err) {
			if(exec->errState == NULL)
				return -1;
			state = exec->errState;
		}
		else {
			if(exec->state == NULL)
				return -1;
			state = exec->state;
		}
		for(transno = 0; (transno < state->nbTrans) && (nb < maxval); transno++) {
			trans = &state->trans[transno];
			if(trans->to < 0)
				continue;
			atom = trans->atom;
			if((atom == NULL) || (atom->valuep == NULL))
				continue;
			if(trans->count == REGEXP_ALL_LAX_COUNTER) {
				/* this should not be reached but ... */
				TODO;
			}
			else if(trans->count == REGEXP_ALL_COUNTER) {
				/* this should not be reached but ... */
				TODO;
			}
			else if(trans->counter >= 0) {
				xmlRegCounter * counter = NULL;
				int count;
				if(err)
					count = exec->errCounts[trans->counter];
				else
					count = exec->counts[trans->counter];
				if(exec->comp)
					counter = &exec->comp->counters[trans->counter];
				if((counter == NULL) || (count < counter->max)) {
					if(atom->neg)
						values[nb++] = (xmlChar *)atom->valuep2;
					else
						values[nb++] = (xmlChar *)atom->valuep;
					(*nbval)++;
				}
			}
			else {
				if(exec->comp && exec->comp->states[trans->to] && (exec->comp->states[trans->to]->type != XML_REGEXP_SINK_STATE)) {
					if(atom->neg)
						values[nb++] = (xmlChar *)atom->valuep2;
					else
						values[nb++] = (xmlChar *)atom->valuep;
					(*nbval)++;
				}
			}
		}
		for(transno = 0; (transno < state->nbTrans) && (nb < maxval); transno++) {
			trans = &state->trans[transno];
			if(trans->to < 0)
				continue;
			atom = trans->atom;
			if((atom == NULL) || (atom->valuep == NULL))
				continue;
			if(trans->count == REGEXP_ALL_LAX_COUNTER) {
				continue;
			}
			else if(trans->count == REGEXP_ALL_COUNTER) {
				continue;
			}
			else if(trans->counter >= 0) {
				continue;
			}
			else {
				if(exec->comp->states[trans->to] && exec->comp->states[trans->to]->type == XML_REGEXP_SINK_STATE) {
					if(atom->neg)
						values[nb++] = (xmlChar *)atom->valuep2;
					else
						values[nb++] = (xmlChar *)atom->valuep;
					(*nbneg)++;
				}
			}
		}
	}
	return 0;
}

/**
 * xmlRegExecNextValues:
 * @exec: a regexp execution context
 * @nbval: pointer to the number of accepted values IN/OUT
 * @nbneg: return number of negative transitions
 * @values: pointer to the array of acceptable values
 * @terminal: return value if this was a terminal state
 *
 * Extract informations from the regexp execution,
 * the parameter @values must point to an array of @nbval string pointers
 * on return nbval will contain the number of possible strings in that
 * state and the @values array will be updated with them. The string values
 * returned will be freed with the @exec context and don't need to be
 * deallocated.
 *
 * Returns: 0 in case of success or -1 in case of error.
 */
int xmlRegExecNextValues(xmlRegExecCtxtPtr exec, int * nbval, int * nbneg,
    xmlChar ** values, int * terminal) {
	return (xmlRegExecGetValues(exec, 0, nbval, nbneg, values, terminal));
}

/**
 * xmlRegExecErrInfo:
 * @exec: a regexp execution context generating an error
 * @string: return value for the error string
 * @nbval: pointer to the number of accepted values IN/OUT
 * @nbneg: return number of negative transitions
 * @values: pointer to the array of acceptable values
 * @terminal: return value if this was a terminal state
 *
 * Extract error informations from the regexp execution, the parameter
 * @string will be updated with the value pushed and not accepted,
 * the parameter @values must point to an array of @nbval string pointers
 * on return nbval will contain the number of possible strings in that
 * state and the @values array will be updated with them. The string values
 * returned will be freed with the @exec context and don't need to be
 * deallocated.
 *
 * Returns: 0 in case of success or -1 in case of error.
 */
int xmlRegExecErrInfo(xmlRegExecCtxtPtr exec, const xmlChar ** string, int * nbval, int * nbneg, xmlChar ** values, int * terminal) 
{
	if(exec == NULL)
		return -1;
	if(string) {
		if(exec->status != 0)
			*string = exec->errString;
		else
			*string = NULL;
	}
	return (xmlRegExecGetValues(exec, 1, nbval, nbneg, values, terminal));
}

#ifdef DEBUG_ERR
static void testerr(xmlRegExecCtxtPtr exec) {
	const xmlChar * string;
	xmlChar * values[5];
	int nb = 5;
	int nbneg;
	int terminal;
	xmlRegExecErrInfo(exec, &string, &nb, &nbneg, &values[0], &terminal);
}

#endif

#if 0
static int xmlRegExecPushChar(xmlRegExecCtxtPtr exec, int UCS) {
	xmlRegTransPtr trans;
	xmlRegAtom * atom;
	int ret;
	int codepoint, len;

	if(exec == NULL)
		return -1;
	if(exec->status != 0)
		return (exec->status);

	while((exec->status == 0) &&
	    ((exec->inputString[exec->index] != 0) ||
		    (exec->state->type != XML_REGEXP_FINAL_STATE))) {
		/*
		 * End of input on non-terminal state, rollback, however we may
		 * still have epsilon like transition for counted transitions
		 * on counters, in that case don't break too early.
		 */
		if((exec->inputString[exec->index] == 0) && (exec->counts == NULL))
			goto rollback;

		exec->transcount = 0;
		for(; exec->transno < exec->state->nbTrans; exec->transno++) {
			trans = &exec->state->trans[exec->transno];
			if(trans->to < 0)
				continue;
			atom = trans->atom;
			ret = 0;
			if(trans->count >= 0) {
				int count;
				xmlRegCounter * counter;
				/*
				 * A counted transition.
				 */
				count = exec->counts[trans->count];
				counter = &exec->comp->counters[trans->count];
#ifdef DEBUG_REGEXP_EXEC
				printf("testing count %d: val %d, min %d, max %d\n",
				    trans->count, count, counter->min,  counter->max);
#endif
				ret = ((count >= counter->min) && (count <= counter->max));
			}
			else if(atom == NULL) {
				fprintf(stderr, "epsilon transition left at runtime\n");
				exec->status = -2;
				break;
			}
			else if(exec->inputString[exec->index] != 0) {
				codepoint = CUR_SCHAR(&(exec->inputString[exec->index]), len);
				ret = xmlRegCheckCharacter(atom, codepoint);
				if((ret == 1) && (atom->min > 0) && (atom->max > 0)) {
					xmlRegStatePtr to = exec->comp->states[trans->to];

					/*
					 * this is a multiple input sequence
					 */
					if(exec->state->nbTrans > exec->transno + 1) {
						xmlFARegExecSave(exec);
					}
					exec->transcount = 1;
					do {
						/*
						 * Try to progress as much as possible on the input
						 */
						if(exec->transcount == atom->max) {
							break;
						}
						exec->index += len;
						/*
						 * End of input: stop here
						 */
						if(exec->inputString[exec->index] == 0) {
							exec->index -= len;
							break;
						}
						if(exec->transcount >= atom->min) {
							int transno = exec->transno;
							xmlRegStatePtr state = exec->state;

							/*
							 * The transition is acceptable save it
							 */
							exec->transno = -1; /* trick */
							exec->state = to;
							xmlFARegExecSave(exec);
							exec->transno = transno;
							exec->state = state;
						}
						codepoint = CUR_SCHAR(&(exec->inputString[exec->index]),
						    len);
						ret = xmlRegCheckCharacter(atom, codepoint);
						exec->transcount++;
					} while(ret == 1);
					if(exec->transcount < atom->min)
						ret = 0;

					/*
					 * If the last check failed but one transition was found
					 * possible, rollback
					 */
					if(ret < 0)
						ret = 0;
					if(!ret) {
						goto rollback;
					}
				}
			}
			if(ret == 1) {
				if(exec->state->nbTrans > exec->transno + 1) {
					xmlFARegExecSave(exec);
				}
				/*
				 * restart count for expressions like this ((abc){2})*
				 */
				if(trans->count >= 0) {
#ifdef DEBUG_REGEXP_EXEC
					printf("Reset count %d\n", trans->count);
#endif
					exec->counts[trans->count] = 0;
				}
				if(trans->counter >= 0) {
#ifdef DEBUG_REGEXP_EXEC
					printf("Increasing count %d\n", trans->counter);
#endif
					exec->counts[trans->counter]++;
				}
#ifdef DEBUG_REGEXP_EXEC
				printf("entering state %d\n", trans->to);
#endif
				exec->state = exec->comp->states[trans->to];
				exec->transno = 0;
				if(trans->atom) {
					exec->index += len;
				}
				goto progress;
			}
			else if(ret < 0) {
				exec->status = -4;
				break;
			}
		}
		if((exec->transno != 0) || (exec->state->nbTrans == 0)) {
rollback:
			/*
			 * Failed to find a way out
			 */
			exec->determinist = 0;
			xmlFARegExecRollBack(exec);
		}
progress:
		continue;
	}
}

#endif
/************************************************************************
*									*
*	Parser for the Schemas Datatype Regular Expressions		*
*	http://www.w3.org/TR/2001/REC-xmlschema-2-20010502/#regexs	*
*									*
************************************************************************/

/**
 * xmlFAIsChar:
 * @ctxt:  a regexp parser context
 *
 * [10]   Char   ::=   [^.\?*+()|#x5B#x5D]
 */
static int xmlFAIsChar(xmlRegParserCtxt * ctxt) 
{
	int len;
	int cur = CUR_SCHAR(ctxt->cur, len);
	if((cur == '.') || (cur == '\\') || (cur == '?') || (cur == '*') || (cur == '+') || (cur == '(') || (cur == ')') || (cur == '|') || (cur == 0x5B) || (cur == 0x5D) || (cur == 0))
		return -1;
	return cur;
}
/**
 * xmlFAParseCharProp:
 * @ctxt:  a regexp parser context
 *
 * [27]   charProp   ::=   IsCategory | IsBlock
 * [28]   IsCategory ::= Letters | Marks | Numbers | Punctuation |
 *                  Separators | Symbols | Others
 * [29]   Letters   ::=   'L' [ultmo]?
 * [30]   Marks   ::=   'M' [nce]?
 * [31]   Numbers   ::=   'N' [dlo]?
 * [32]   Punctuation   ::=   'P' [cdseifo]?
 * [33]   Separators   ::=   'Z' [slp]?
 * [34]   Symbols   ::=   'S' [mcko]?
 * [35]   Others   ::=   'C' [cfon]?
 * [36]   IsBlock   ::=   'Is' [a-zA-Z0-9#x2D]+
 */
static void xmlFAParseCharProp(xmlRegParserCtxt * ctxt) {
	int cur;
	xmlRegAtomType type = (xmlRegAtomType)0;
	xmlChar * blockName = NULL;

	cur = CUR;
	if(cur == 'L') {
		NEXT;
		cur = CUR;
		if(cur == 'u') {
			NEXT;
			type = XML_REGEXP_LETTER_UPPERCASE;
		}
		else if(cur == 'l') {
			NEXT;
			type = XML_REGEXP_LETTER_LOWERCASE;
		}
		else if(cur == 't') {
			NEXT;
			type = XML_REGEXP_LETTER_TITLECASE;
		}
		else if(cur == 'm') {
			NEXT;
			type = XML_REGEXP_LETTER_MODIFIER;
		}
		else if(cur == 'o') {
			NEXT;
			type = XML_REGEXP_LETTER_OTHERS;
		}
		else {
			type = XML_REGEXP_LETTER;
		}
	}
	else if(cur == 'M') {
		NEXT;
		cur = CUR;
		if(cur == 'n') {
			NEXT;
			/* nonspacing */
			type = XML_REGEXP_MARK_NONSPACING;
		}
		else if(cur == 'c') {
			NEXT;
			/* spacing combining */
			type = XML_REGEXP_MARK_SPACECOMBINING;
		}
		else if(cur == 'e') {
			NEXT;
			/* enclosing */
			type = XML_REGEXP_MARK_ENCLOSING;
		}
		else {
			/* all marks */
			type = XML_REGEXP_MARK;
		}
	}
	else if(cur == 'N') {
		NEXT;
		cur = CUR;
		if(cur == 'd') {
			NEXT;
			/* digital */
			type = XML_REGEXP_NUMBER_DECIMAL;
		}
		else if(cur == 'l') {
			NEXT;
			/* letter */
			type = XML_REGEXP_NUMBER_LETTER;
		}
		else if(cur == 'o') {
			NEXT;
			/* other */
			type = XML_REGEXP_NUMBER_OTHERS;
		}
		else {
			/* all numbers */
			type = XML_REGEXP_NUMBER;
		}
	}
	else if(cur == 'P') {
		NEXT;
		cur = CUR;
		if(cur == 'c') {
			NEXT;
			/* connector */
			type = XML_REGEXP_PUNCT_CONNECTOR;
		}
		else if(cur == 'd') {
			NEXT;
			/* dash */
			type = XML_REGEXP_PUNCT_DASH;
		}
		else if(cur == 's') {
			NEXT;
			/* open */
			type = XML_REGEXP_PUNCT_OPEN;
		}
		else if(cur == 'e') {
			NEXT;
			/* close */
			type = XML_REGEXP_PUNCT_CLOSE;
		}
		else if(cur == 'i') {
			NEXT;
			/* initial quote */
			type = XML_REGEXP_PUNCT_INITQUOTE;
		}
		else if(cur == 'f') {
			NEXT;
			/* final quote */
			type = XML_REGEXP_PUNCT_FINQUOTE;
		}
		else if(cur == 'o') {
			NEXT;
			/* other */
			type = XML_REGEXP_PUNCT_OTHERS;
		}
		else {
			/* all punctuation */
			type = XML_REGEXP_PUNCT;
		}
	}
	else if(cur == 'Z') {
		NEXT;
		cur = CUR;
		if(cur == 's') {
			NEXT;
			/* space */
			type = XML_REGEXP_SEPAR_SPACE;
		}
		else if(cur == 'l') {
			NEXT;
			/* line */
			type = XML_REGEXP_SEPAR_LINE;
		}
		else if(cur == 'p') {
			NEXT;
			/* paragraph */
			type = XML_REGEXP_SEPAR_PARA;
		}
		else {
			/* all separators */
			type = XML_REGEXP_SEPAR;
		}
	}
	else if(cur == 'S') {
		NEXT;
		cur = CUR;
		if(cur == 'm') {
			NEXT;
			type = XML_REGEXP_SYMBOL_MATH;
			/* math */
		}
		else if(cur == 'c') {
			NEXT;
			type = XML_REGEXP_SYMBOL_CURRENCY;
			/* currency */
		}
		else if(cur == 'k') {
			NEXT;
			type = XML_REGEXP_SYMBOL_MODIFIER;
			/* modifiers */
		}
		else if(cur == 'o') {
			NEXT;
			type = XML_REGEXP_SYMBOL_OTHERS;
			/* other */
		}
		else {
			/* all symbols */
			type = XML_REGEXP_SYMBOL;
		}
	}
	else if(cur == 'C') {
		NEXT;
		cur = CUR;
		if(cur == 'c') {
			NEXT;
			/* control */
			type = XML_REGEXP_OTHER_CONTROL;
		}
		else if(cur == 'f') {
			NEXT;
			/* format */
			type = XML_REGEXP_OTHER_FORMAT;
		}
		else if(cur == 'o') {
			NEXT;
			/* private use */
			type = XML_REGEXP_OTHER_PRIVATE;
		}
		else if(cur == 'n') {
			NEXT;
			/* not assigned */
			type = XML_REGEXP_OTHER_NA;
		}
		else {
			/* all others */
			type = XML_REGEXP_OTHER;
		}
	}
	else if(cur == 'I') {
		const xmlChar * start;
		NEXT;
		cur = CUR;
		if(cur != 's') {
			ERROR("IsXXXX expected");
			return;
		}
		NEXT;
		start = ctxt->cur;
		cur = CUR;
		if(((cur >= 'a') && (cur <= 'z')) || ((cur >= 'A') && (cur <= 'Z')) || ((cur >= '0') && (cur <= '9')) || (cur == 0x2D)) {
			NEXT;
			cur = CUR;
			while(((cur >= 'a') && (cur <= 'z')) || ((cur >= 'A') && (cur <= 'Z')) || ((cur >= '0') && (cur <= '9')) || (cur == 0x2D)) {
				NEXT;
				cur = CUR;
			}
		}
		type = XML_REGEXP_BLOCK_NAME;
		blockName = xmlStrndup(start, ctxt->cur - start);
	}
	else {
		ERROR("Unknown char property");
		return;
	}
	if(ctxt->atom == NULL) {
		ctxt->atom = xmlRegNewAtom(ctxt, type);
		if(ctxt->atom)
			ctxt->atom->valuep = blockName;
	}
	else if(ctxt->atom->type == XML_REGEXP_RANGES) {
		xmlRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg, type, 0, 0, blockName);
	}
}

/**
 * xmlFAParseCharClassEsc:
 * @ctxt:  a regexp parser context
 *
 * [23] charClassEsc ::= ( SingleCharEsc | MultiCharEsc | catEsc | complEsc )
 * [24] SingleCharEsc ::= '\' [nrt\|.?*+(){}#x2D#x5B#x5D#x5E]
 * [25] catEsc   ::=   '\p{' charProp '}'
 * [26] complEsc ::=   '\P{' charProp '}'
 * [37] MultiCharEsc ::= '.' | ('\' [sSiIcCdDwW])
 */
static void xmlFAParseCharClassEsc(xmlRegParserCtxt * ctxt)
{
	int cur;

	if(CUR == '.') {
		if(ctxt->atom == NULL) {
			ctxt->atom = xmlRegNewAtom(ctxt, XML_REGEXP_ANYCHAR);
		}
		else if(ctxt->atom->type == XML_REGEXP_RANGES) {
			xmlRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
			    XML_REGEXP_ANYCHAR, 0, 0, 0);
		}
		NEXT;
		return;
	}
	if(CUR != '\\') {
		ERROR("Escaped sequence: expecting \\");
		return;
	}
	NEXT;
	cur = CUR;
	if(cur == 'p') {
		NEXT;
		if(CUR != '{') {
			ERROR("Expecting '{'");
			return;
		}
		NEXT;
		xmlFAParseCharProp(ctxt);
		if(CUR != '}') {
			ERROR("Expecting '}'");
			return;
		}
		NEXT;
	}
	else if(cur == 'P') {
		NEXT;
		if(CUR != '{') {
			ERROR("Expecting '{'");
			return;
		}
		NEXT;
		xmlFAParseCharProp(ctxt);
		ctxt->atom->neg = 1;
		if(CUR != '}') {
			ERROR("Expecting '}'");
			return;
		}
		NEXT;
	}
	else if((cur == 'n') || (cur == 'r') || (cur == 't') || (cur == '\\') ||
	    (cur == '|') || (cur == '.') || (cur == '?') || (cur == '*') ||
	    (cur == '+') || (cur == '(') || (cur == ')') || (cur == '{') ||
	    (cur == '}') || (cur == 0x2D) || (cur == 0x5B) || (cur == 0x5D) ||
	    (cur == 0x5E)) {
		if(ctxt->atom == NULL) {
			ctxt->atom = xmlRegNewAtom(ctxt, XML_REGEXP_CHARVAL);
			if(ctxt->atom) {
				switch(cur) {
					case 'n': ctxt->atom->codepoint = '\n'; break;
					case 'r': ctxt->atom->codepoint = '\r'; break;
					case 't': ctxt->atom->codepoint = '\t'; break;
					default: ctxt->atom->codepoint = cur;
				}
			}
		}
		else if(ctxt->atom->type == XML_REGEXP_RANGES) {
			switch(cur) {
				case 'n':
				    cur = '\n';
				    break;
				case 'r':
				    cur = '\r';
				    break;
				case 't':
				    cur = '\t';
				    break;
			}
			xmlRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
			    XML_REGEXP_CHARVAL, cur, cur, 0);
		}
		NEXT;
	}
	else if((cur == 's') || (cur == 'S') || (cur == 'i') || (cur == 'I') ||
	    (cur == 'c') || (cur == 'C') || (cur == 'd') || (cur == 'D') ||
	    (cur == 'w') || (cur == 'W')) {
		xmlRegAtomType type = XML_REGEXP_ANYSPACE;

		switch(cur) {
			case 's':
			    type = XML_REGEXP_ANYSPACE;
			    break;
			case 'S':
			    type = XML_REGEXP_NOTSPACE;
			    break;
			case 'i':
			    type = XML_REGEXP_INITNAME;
			    break;
			case 'I':
			    type = XML_REGEXP_NOTINITNAME;
			    break;
			case 'c':
			    type = XML_REGEXP_NAMECHAR;
			    break;
			case 'C':
			    type = XML_REGEXP_NOTNAMECHAR;
			    break;
			case 'd':
			    type = XML_REGEXP_DECIMAL;
			    break;
			case 'D':
			    type = XML_REGEXP_NOTDECIMAL;
			    break;
			case 'w':
			    type = XML_REGEXP_REALCHAR;
			    break;
			case 'W':
			    type = XML_REGEXP_NOTREALCHAR;
			    break;
		}
		NEXT;
		if(ctxt->atom == NULL) {
			ctxt->atom = xmlRegNewAtom(ctxt, type);
		}
		else if(ctxt->atom->type == XML_REGEXP_RANGES) {
			xmlRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
			    type, 0, 0, 0);
		}
	}
	else {
		ERROR("Wrong escape sequence, misuse of character '\\'");
	}
}

/**
 * xmlFAParseCharRange:
 * @ctxt:  a regexp parser context
 *
 * [17]   charRange   ::=     seRange | XmlCharRef | XmlCharIncDash
 * [18]   seRange   ::=   charOrEsc '-' charOrEsc
 * [20]   charOrEsc   ::=   XmlChar | SingleCharEsc
 * [21]   XmlChar   ::=   [^\#x2D#x5B#x5D]
 * [22]   XmlCharIncDash   ::=   [^\#x5B#x5D]
 */
static void xmlFAParseCharRange(xmlRegParserCtxt * ctxt) {
	int cur, len;
	int start = -1;
	int end = -1;

	if(CUR == '\0') {
		ERROR("Expecting ']'");
		return;
	}

	cur = CUR;
	if(cur == '\\') {
		NEXT;
		cur = CUR;
		switch(cur) {
			case 'n': start = 0xA; break;
			case 'r': start = 0xD; break;
			case 't': start = 0x9; break;
			case '\\': case '|': case '.': case '-': case '^': case '?':
			case '*': case '+': case '{': case '}': case '(': case ')':
			case '[': case ']':
			    start = cur; break;
			default:
			    ERROR("Invalid escape value");
			    return;
		}
		end = start;
		len = 1;
	}
	else if((cur != 0x5B) && (cur != 0x5D)) {
		end = start = CUR_SCHAR(ctxt->cur, len);
	}
	else {
		ERROR("Expecting a char range");
		return;
	}
	/*
	 * Since we are "inside" a range, we can assume ctxt->cur is past
	 * the start of ctxt->string, and PREV should be safe
	 */
	if((start == '-') && (NXT(1) != ']') && (PREV != '[') && (PREV != '^')) {
		NEXTL(len);
		return;
	}
	NEXTL(len);
	cur = CUR;
	if((cur != '-') || (NXT(1) == ']')) {
		xmlRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg, XML_REGEXP_CHARVAL, start, end, 0);
		return;
	}
	NEXT;
	cur = CUR;
	if(cur == '\\') {
		NEXT;
		cur = CUR;
		switch(cur) {
			case 'n': end = 0xA; break;
			case 'r': end = 0xD; break;
			case 't': end = 0x9; break;
			case '\\': case '|': case '.': case '-': case '^': case '?':
			case '*': case '+': case '{': case '}': case '(': case ')':
			case '[': case ']':
			    end = cur; break;
			default:
			    ERROR("Invalid escape value");
			    return;
		}
		len = 1;
	}
	else if((cur != 0x5B) && (cur != 0x5D)) {
		end = CUR_SCHAR(ctxt->cur, len);
	}
	else {
		ERROR("Expecting the end of a char range");
		return;
	}
	NEXTL(len);
	/* @todo check that the values are acceptable character ranges for XML */
	if(end < start) {
		ERROR("End of range is before start of range");
	}
	else {
		xmlRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
		    XML_REGEXP_CHARVAL, start, end, 0);
	}
	return;
}

/**
 * xmlFAParsePosCharGroup:
 * @ctxt:  a regexp parser context
 *
 * [14]   posCharGroup ::= ( charRange | charClassEsc  )+
 */
static void xmlFAParsePosCharGroup(xmlRegParserCtxt * ctxt)
{
	do {
		if(CUR == '\\') {
			xmlFAParseCharClassEsc(ctxt);
		}
		else {
			xmlFAParseCharRange(ctxt);
		}
	} while((CUR != ']') && (CUR != '^') && (CUR != '-') &&
	    (CUR != 0) && (ctxt->error == 0));
}

/**
 * xmlFAParseCharGroup:
 * @ctxt:  a regexp parser context
 *
 * [13]   charGroup    ::= posCharGroup | negCharGroup | charClassSub
 * [15]   negCharGroup ::= '^' posCharGroup
 * [16]   charClassSub ::= ( posCharGroup | negCharGroup ) '-' charClassExpr
 * [12]   charClassExpr ::= '[' charGroup ']'
 */
static void xmlFAParseCharGroup(xmlRegParserCtxt * ctxt)
{
	int n = ctxt->neg;
	while((CUR != ']') && (ctxt->error == 0)) {
		if(CUR == '^') {
			int neg = ctxt->neg;

			NEXT;
			ctxt->neg = !ctxt->neg;
			xmlFAParsePosCharGroup(ctxt);
			ctxt->neg = neg;
		}
		else if((CUR == '-') && (NXT(1) == '[')) {
			int neg = ctxt->neg;
			ctxt->neg = 2;
			NEXT; /* eat the '-' */
			NEXT; /* eat the '[' */
			xmlFAParseCharGroup(ctxt);
			if(CUR == ']') {
				NEXT;
			}
			else {
				ERROR("charClassExpr: ']' expected");
				break;
			}
			ctxt->neg = neg;
			break;
		}
		else if(CUR != ']') {
			xmlFAParsePosCharGroup(ctxt);
		}
	}
	ctxt->neg = n;
}

/**
 * xmlFAParseCharClass:
 * @ctxt:  a regexp parser context
 *
 * [11]   charClass   ::=     charClassEsc | charClassExpr
 * [12]   charClassExpr   ::=   '[' charGroup ']'
 */
static void xmlFAParseCharClass(xmlRegParserCtxt * ctxt)
{
	if(CUR == '[') {
		NEXT;
		ctxt->atom = xmlRegNewAtom(ctxt, XML_REGEXP_RANGES);
		if(ctxt->atom) {
			xmlFAParseCharGroup(ctxt);
			if(CUR == ']') {
				NEXT;
			}
			else {
				ERROR("xmlFAParseCharClass: ']' expected");
			}
		}
	}
	else {
		xmlFAParseCharClassEsc(ctxt);
	}
}

/**
 * xmlFAParseQuantExact:
 * @ctxt:  a regexp parser context
 *
 * [8]   QuantExact   ::=   [0-9]+
 *
 * Returns 0 if success or -1 in case of error
 */
static int xmlFAParseQuantExact(xmlRegParserCtxt * ctxt)
{
	int ret = 0;
	int ok = 0;
	while((CUR >= '0') && (CUR <= '9')) {
		ret = ret * 10 + (CUR - '0');
		ok = 1;
		NEXT;
	}
	return (ok != 1) ? -1 : ret;
}

/**
 * xmlFAParseQuantifier:
 * @ctxt:  a regexp parser context
 *
 * [4]   quantifier   ::=   [?*+] | ( '{' quantity '}' )
 * [5]   quantity   ::=   quantRange | quantMin | QuantExact
 * [6]   quantRange   ::=   QuantExact ',' QuantExact
 * [7]   quantMin   ::=   QuantExact ','
 * [8]   QuantExact   ::=   [0-9]+
 */
static int xmlFAParseQuantifier(xmlRegParserCtxt * ctxt)
{
	int cur = CUR;
	if(oneof3(cur, '?', '*', '+')) {
		if(ctxt->atom) {
			if(cur == '?')
				ctxt->atom->quant = XML_REGEXP_QUANT_OPT;
			else if(cur == '*')
				ctxt->atom->quant = XML_REGEXP_QUANT_MULT;
			else if(cur == '+')
				ctxt->atom->quant = XML_REGEXP_QUANT_PLUS;
		}
		NEXT;
		return 1;
	}
	if(cur == '{') {
		int min = 0, max = 0;
		NEXT;
		cur = xmlFAParseQuantExact(ctxt);
		if(cur >= 0)
			min = cur;
		if(CUR == ',') {
			NEXT;
			if(CUR == '}')
				max = INT_MAX;
			else {
				cur = xmlFAParseQuantExact(ctxt);
				if(cur >= 0)
					max = cur;
				else {
					ERROR("Improper quantifier");
				}
			}
		}
		if(CUR == '}') {
			NEXT;
		}
		else {
			ERROR("Unterminated quantifier");
		}
		if(max == 0)
			max = min;
		if(ctxt->atom) {
			ctxt->atom->quant = XML_REGEXP_QUANT_RANGE;
			ctxt->atom->min = min;
			ctxt->atom->max = max;
		}
		return 1;
	}
	return 0;
}

/**
 * xmlFAParseAtom:
 * @ctxt:  a regexp parser context
 *
 * [9]   atom   ::=   Char | charClass | ( '(' regExp ')' )
 */
static int xmlFAParseAtom(xmlRegParserCtxt * ctxt)
{
	int len;
	int codepoint = xmlFAIsChar(ctxt);
	if(codepoint > 0) {
		ctxt->atom = xmlRegNewAtom(ctxt, XML_REGEXP_CHARVAL);
		if(ctxt->atom == NULL)
			return -1;
		codepoint = CUR_SCHAR(ctxt->cur, len);
		ctxt->atom->codepoint = codepoint;
		NEXTL(len);
		return 1;
	}
	else if(CUR == '|') {
		return 0;
	}
	else if(CUR == 0) {
		return 0;
	}
	else if(CUR == ')') {
		return 0;
	}
	else if(CUR == '(') {
		xmlRegStatePtr start;
		xmlRegStatePtr oldend;
		xmlRegStatePtr start0;
		NEXT;
		/*
		 * this extra Epsilon transition is needed if we count with 0 allowed
		 * unfortunately this can't be known at that point
		 */
		xmlFAGenerateEpsilonTransition(ctxt, ctxt->state, 0);
		start0 = ctxt->state;
		xmlFAGenerateEpsilonTransition(ctxt, ctxt->state, 0);
		start = ctxt->state;
		oldend = ctxt->end;
		ctxt->end = NULL;
		ctxt->atom = NULL;
		xmlFAParseRegExp(ctxt, 0);
		if(CUR == ')') {
			NEXT;
		}
		else {
			ERROR("xmlFAParseAtom: expecting ')'");
		}
		ctxt->atom = xmlRegNewAtom(ctxt, XML_REGEXP_SUBREG);
		if(ctxt->atom == NULL)
			return -1;
		ctxt->atom->start = start;
		ctxt->atom->start0 = start0;
		ctxt->atom->stop = ctxt->state;
		ctxt->end = oldend;
		return 1;
	}
	else if((CUR == '[') || (CUR == '\\') || (CUR == '.')) {
		xmlFAParseCharClass(ctxt);
		return 1;
	}
	return 0;
}
/**
 * xmlFAParsePiece:
 * @ctxt:  a regexp parser context
 *
 * [3]   piece   ::=   atom quantifier?
 */
static int xmlFAParsePiece(xmlRegParserCtxt * ctxt)
{
	int ret;
	ctxt->atom = NULL;
	ret = xmlFAParseAtom(ctxt);
	if(!ret)
		return 0;
	if(ctxt->atom == NULL) {
		ERROR("internal: no atom generated");
	}
	xmlFAParseQuantifier(ctxt);
	return 1;
}

/**
 * xmlFAParseBranch:
 * @ctxt:  a regexp parser context
 * @to: optional target to the end of the branch
 *
 * @to is used to optimize by removing duplicate path in automata
 * in expressions like (a|b)(c|d)
 *
 * [2]   branch   ::=   piece*
 */
static int xmlFAParseBranch(xmlRegParserCtxt * ctxt, xmlRegStatePtr to)
{
	xmlRegStatePtr previous = ctxt->state;
	int ret = xmlFAParsePiece(ctxt);
	if(ret != 0) {
		if(xmlFAGenerateTransitions(ctxt, previous, (CUR=='|' || CUR==')') ? to : NULL, ctxt->atom) < 0)
			return -1;
		previous = ctxt->state;
		ctxt->atom = NULL;
	}
	while((ret != 0) && (ctxt->error == 0)) {
		ret = xmlFAParsePiece(ctxt);
		if(ret) {
			if(xmlFAGenerateTransitions(ctxt, previous, (CUR=='|' || CUR==')') ? to : NULL, ctxt->atom) < 0)
				return -1;
			previous = ctxt->state;
			ctxt->atom = NULL;
		}
	}
	return 0;
}

/**
 * xmlFAParseRegExp:
 * @ctxt:  a regexp parser context
 * @top:  is this the top-level expression ?
 *
 * [1]   regExp   ::=     branch  ( '|' branch )*
 */
static void xmlFAParseRegExp(xmlRegParserCtxt * ctxt, int top)
{
	xmlRegStatePtr end;
	/* if not top start should have been generated by an epsilon trans */
	xmlRegStatePtr start = ctxt->state;
	ctxt->end = NULL;
	xmlFAParseBranch(ctxt, 0);
	if(top) {
#ifdef DEBUG_REGEXP_GRAPH
		printf("State %d is final\n", ctxt->state->no);
#endif
		ctxt->state->type = XML_REGEXP_FINAL_STATE;
	}
	if(CUR != '|') {
		ctxt->end = ctxt->state;
		return;
	}
	end = ctxt->state;
	while((CUR == '|') && (ctxt->error == 0)) {
		NEXT;
		if(CUR == 0) {
			ERROR("expecting a branch after |")
			return;
		}
		ctxt->state = start;
		ctxt->end = NULL;
		xmlFAParseBranch(ctxt, end);
	}
	if(!top) {
		ctxt->state = end;
		ctxt->end = end;
	}
}
//
// The basic API
// 
/**
 * xmlRegexpPrint:
 * @output: the file for the output debug
 * @regexp: the compiled regexp
 *
 * Print the content of the compiled regular expression
 */
void xmlRegexpPrint(FILE * output, xmlRegexp * regexp)
{
	int i;
	if(output) {
		fprintf(output, " regexp: ");
		if(regexp == NULL) {
			fprintf(output, "NULL\n");
		}
		else {
			fprintf(output, "'%s' ", regexp->string);
			fprintf(output, "\n");
			fprintf(output, "%d atoms:\n", regexp->nbAtoms);
			for(i = 0; i < regexp->nbAtoms; i++) {
				fprintf(output, " %02d ", i);
				xmlRegPrintAtom(output, regexp->atoms[i]);
			}
			fprintf(output, "%d states:", regexp->nbStates);
			fprintf(output, "\n");
			for(i = 0; i < regexp->nbStates; i++) {
				xmlRegPrintState(output, regexp->states[i]);
			}
			fprintf(output, "%d counters:\n", regexp->nbCounters);
			for(i = 0; i < regexp->nbCounters; i++) {
				fprintf(output, " %d: min %d max %d\n", i, regexp->counters[i].min, regexp->counters[i].max);
			}
		}
	}
}
/**
 * xmlRegexpCompile:
 * @regexp:  a regular expression string
 *
 * Parses a regular expression conforming to XML Schemas Part 2 Datatype
 * Appendix F and builds an automata suitable for testing strings against
 * that regular expression
 *
 * Returns the compiled expression or NULL in case of error
 */
xmlRegexp * xmlRegexpCompile(const xmlChar * regexp)
{
	xmlRegexp * ret;
	xmlRegParserCtxt * ctxt = xmlRegNewParserCtxt(regexp);
	if(!ctxt)
		return 0;
	/* initialize the parser */
	ctxt->end = NULL;
	ctxt->start = ctxt->state = xmlRegNewState(ctxt);
	xmlRegStatePush(ctxt, ctxt->start);

	/* parse the expression building an automata */
	xmlFAParseRegExp(ctxt, 1);
	if(CUR != 0) {
		ERROR("xmlFAParseRegExp: extra characters");
	}
	if(ctxt->error) {
		xmlRegFreeParserCtxt(ctxt);
		return 0;
	}
	ctxt->end = ctxt->state;
	ctxt->start->type = XML_REGEXP_START_STATE;
	ctxt->end->type = XML_REGEXP_FINAL_STATE;

	/* remove the Epsilon except for counted transitions */
	xmlFAEliminateEpsilonTransitions(ctxt);
	if(ctxt->error) {
		xmlRegFreeParserCtxt(ctxt);
		return 0;
	}
	ret = xmlRegEpxFromParse(ctxt);
	xmlRegFreeParserCtxt(ctxt);
	return ret;
}
/**
 * xmlRegexpExec:
 * @comp:  the compiled regular expression
 * @content:  the value to check against the regular expression
 *
 * Check if the regular expression generates the value
 *
 * Returns 1 if it matches, 0 if not and a negative value in case of error
 */
int xmlRegexpExec(xmlRegexp * comp, const xmlChar * content)
{
	return (comp && content) ? xmlFARegExec(comp, content) : -1;
}
/**
 * xmlRegexpIsDeterminist:
 * @comp:  the compiled regular expression
 *
 * Check if the regular expression is determinist
 *
 * Returns 1 if it yes, 0 if not and a negative value in case of error
 */
int xmlRegexpIsDeterminist(xmlRegexp * comp)
{
	xmlAutomataPtr am;
	int ret;
	if(comp == NULL)
		return -1;
	if(comp->determinist != -1)
		return (comp->determinist);
	am = xmlNewAutomata();
	if(am->states) {
		for(int i = 0; i < am->nbStates; i++)
			xmlRegFreeState(am->states[i]);
		SAlloc::F(am->states);
	}
	am->nbAtoms = comp->nbAtoms;
	am->atoms = comp->atoms;
	am->nbStates = comp->nbStates;
	am->states = comp->states;
	am->determinist = -1;
	am->flags = comp->flags;
	ret = xmlFAComputesDeterminism(am);
	am->atoms = NULL;
	am->states = NULL;
	xmlFreeAutomata(am);
	comp->determinist = ret;
	return ret;
}
/**
 * @regexp:  the regexp
 * Free a regexp
 */
void xmlRegFreeRegexp(xmlRegexp * regexp)
{
	if(regexp) {
		SAlloc::F(regexp->string);
		if(regexp->states) {
			for(int i = 0; i < regexp->nbStates; i++)
				xmlRegFreeState(regexp->states[i]);
			SAlloc::F(regexp->states);
		}
		if(regexp->atoms) {
			for(int i = 0; i < regexp->nbAtoms; i++)
				xmlRegFreeAtom(regexp->atoms[i]);
			SAlloc::F(regexp->atoms);
		}
		SAlloc::F(regexp->counters);
		SAlloc::F(regexp->compact);
		SAlloc::F(regexp->transdata);
		if(regexp->stringMap) {
			for(int i = 0; i < regexp->nbstrings; i++)
				SAlloc::F(regexp->stringMap[i]);
			SAlloc::F(regexp->stringMap);
		}
		SAlloc::F(regexp);
	}
}

#ifdef LIBXML_AUTOMATA_ENABLED
/************************************************************************
*									*
*			The Automata interface				*
*									*
************************************************************************/

/**
 * xmlNewAutomata:
 *
 * Create a new automata
 *
 * Returns the new object or NULL in case of failure
 */
xmlAutomataPtr xmlNewAutomata() 
{
	xmlAutomata * ctxt = xmlRegNewParserCtxt(NULL);
	if(!ctxt)
		return 0;
	/* initialize the parser */
	ctxt->end = NULL;
	ctxt->start = ctxt->state = xmlRegNewState(ctxt);
	if(ctxt->start == NULL) {
		xmlFreeAutomata(ctxt);
		return 0;
	}
	ctxt->start->type = XML_REGEXP_START_STATE;
	if(xmlRegStatePush(ctxt, ctxt->start) < 0) {
		xmlRegFreeState(ctxt->start);
		xmlFreeAutomata(ctxt);
		return 0;
	}
	ctxt->flags = 0;

	return ctxt;
}
/**
 * xmlFreeAutomata:
 * @am: an automata
 *
 * Free an automata
 */
void xmlFreeAutomata(xmlAutomataPtr am) 
{
	xmlRegFreeParserCtxt(am);
}
/**
 * xmlAutomataSetFlags:
 * @am: an automata
 * @flags:  a set of internal flags
 *
 * Set some flags on the automata
 */
void xmlAutomataSetFlags(xmlAutomataPtr am, int flags) 
{
	if(am)
		am->flags |= flags;
}
/**
 * xmlAutomataGetInitState:
 * @am: an automata
 *
 * Initial state lookup
 *
 * Returns the initial state of the automata
 */
xmlAutomataStatePtr xmlAutomataGetInitState(xmlAutomataPtr am) 
{
	return am ? (am->start) : 0;
}
/**
 * xmlAutomataSetFinalState:
 * @am: an automata
 * @state: a state in this automata
 *
 * Makes that state a final state
 *
 * Returns 0 or -1 in case of error
 */
int xmlAutomataSetFinalState(xmlAutomataPtr am, xmlAutomataStatePtr state) 
{
	if((am == NULL) || (state == NULL))
		return -1;
	state->type = XML_REGEXP_FINAL_STATE;
	return 0;
}
/**
 * xmlAutomataNewTransition:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @token: the input string associated to that transition
 * @data: data passed to the callback function if the transition is activated
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a transition from the @from state to the target state
 * activated by the value of @token
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewTransition(xmlAutomataPtr am, xmlAutomataStatePtr from, xmlAutomataStatePtr to, const xmlChar * token, void * data) 
{
	xmlRegAtom * atom;
	if((am == NULL) || (from == NULL) || (token == NULL))
		return 0;
	atom = xmlRegNewAtom(am, XML_REGEXP_STRING);
	if(atom == NULL)
		return 0;
	atom->data = data;
	atom->valuep = sstrdup(token);
	if(xmlFAGenerateTransitions(am, from, to, atom) < 0) {
		xmlRegFreeAtom(atom);
		return 0;
	}
	if(to == NULL)
		return (am->state);
	return to;
}
/**
 * xmlAutomataNewTransition2:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @token: the first input string associated to that transition
 * @token2: the second input string associated to that transition
 * @data: data passed to the callback function if the transition is activated
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a transition from the @from state to the target state
 * activated by the value of @token
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewTransition2(xmlAutomataPtr am, xmlAutomataStatePtr from,
    xmlAutomataStatePtr to, const xmlChar * token, const xmlChar * token2, void * data) 
{
	xmlRegAtom * atom;
	if((am == NULL) || (from == NULL) || (token == NULL))
		return 0;
	atom = xmlRegNewAtom(am, XML_REGEXP_STRING);
	if(atom == NULL)
		return 0;
	atom->data = data;
	if(isempty(token2)) {
		atom->valuep = sstrdup(token);
	}
	else {
		int lenn = sstrlen(token2);
		int lenp = sstrlen(token);
		xmlChar * str = static_cast<xmlChar *>(SAlloc::M(lenn + lenp + 2));
		if(!str) {
			xmlRegFreeAtom(atom);
			return 0;
		}
		memcpy(&str[0], token, lenp);
		str[lenp] = '|';
		memcpy(&str[lenp + 1], token2, lenn);
		str[lenn + lenp + 1] = 0;

		atom->valuep = str;
	}
	if(xmlFAGenerateTransitions(am, from, to, atom) < 0) {
		xmlRegFreeAtom(atom);
		return 0;
	}
	if(to == NULL)
		return (am->state);
	return to;
}
/**
 * xmlAutomataNewNegTrans:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @token: the first input string associated to that transition
 * @token2: the second input string associated to that transition
 * @data: data passed to the callback function if the transition is activated
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a transition from the @from state to the target state
 * activated by any value except (@token,@token2)
 * Note that if @token2 is not NULL, then (X, NULL) won't match to follow
 # the semantic of XSD ##other
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewNegTrans(xmlAutomataPtr am, xmlAutomataStatePtr from,
    xmlAutomataStatePtr to, const xmlChar * token, const xmlChar * token2, void * data) 
{
	xmlRegAtom * atom;
	xmlChar err_msg[200];
	if((am == NULL) || (from == NULL) || (token == NULL))
		return 0;
	atom = xmlRegNewAtom(am, XML_REGEXP_STRING);
	if(atom == NULL)
		return 0;
	atom->data = data;
	atom->neg = 1;
	if(isempty(token2)) {
		atom->valuep = sstrdup(token);
	}
	else {
		int lenn = sstrlen(token2);
		int lenp = sstrlen(token);
		xmlChar * str = static_cast<xmlChar *>(SAlloc::M(lenn + lenp + 2));
		if(!str) {
			xmlRegFreeAtom(atom);
			return 0;
		}
		memcpy(&str[0], token, lenp);
		str[lenp] = '|';
		memcpy(&str[lenp + 1], token2, lenn);
		str[lenn + lenp + 1] = 0;
		atom->valuep = str;
	}
	snprintf((char *)err_msg, 199, "not %s", (const char *)atom->valuep);
	err_msg[199] = 0;
	atom->valuep2 = sstrdup(err_msg);

	if(xmlFAGenerateTransitions(am, from, to, atom) < 0) {
		xmlRegFreeAtom(atom);
		return 0;
	}
	am->negs++;
	if(to == NULL)
		return (am->state);
	return to;
}

/**
 * xmlAutomataNewCountTrans2:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @token: the input string associated to that transition
 * @token2: the second input string associated to that transition
 * @min:  the minimum successive occurences of token
 * @max:  the maximum successive occurences of token
 * @data:  data associated to the transition
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a transition from the @from state to the target state
 * activated by a succession of input of value @token and @token2 and
 * whose number is between @min and @max
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewCountTrans2(xmlAutomataPtr am, xmlAutomataStatePtr from, xmlAutomataStatePtr to, const xmlChar * token,
    const xmlChar * token2, int min, int max, void * data) 
{
	xmlRegAtom * atom;
	int counter;
	if((am == NULL) || (from == NULL) || (token == NULL))
		return 0;
	if(min < 0)
		return 0;
	if((max < min) || (max < 1))
		return 0;
	atom = xmlRegNewAtom(am, XML_REGEXP_STRING);
	if(atom == NULL)
		return 0;
	if((token2 == NULL) || (*token2 == 0)) {
		atom->valuep = sstrdup(token);
	}
	else {
		int lenn = sstrlen(token2);
		int lenp = sstrlen(token);
		xmlChar * str = static_cast<xmlChar *>(SAlloc::M(lenn + lenp + 2));
		if(!str) {
			xmlRegFreeAtom(atom);
			return 0;
		}
		memcpy(&str[0], token, lenp);
		str[lenp] = '|';
		memcpy(&str[lenp + 1], token2, lenn);
		str[lenn + lenp + 1] = 0;

		atom->valuep = str;
	}
	atom->data = data;
	if(min == 0)
		atom->min = 1;
	else
		atom->min = min;
	atom->max = max;
	/*
	 * associate a counter to the transition.
	 */
	counter = xmlRegGetCounter(am);
	am->counters[counter].min = min;
	am->counters[counter].max = max;
	/* xmlFAGenerateTransitions(am, from, to, atom); */
	if(to == NULL) {
		to = xmlRegNewState(am);
		xmlRegStatePush(am, to);
	}
	xmlRegStateAddTrans(am, from, atom, to, counter, -1);
	xmlRegAtomPush(am, atom);
	am->state = to;
	if(to == NULL)
		to = am->state;
	if(to == NULL)
		return 0;
	if(min == 0)
		xmlFAGenerateEpsilonTransition(am, from, to);
	return to;
}
/**
 * xmlAutomataNewCountTrans:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @token: the input string associated to that transition
 * @min:  the minimum successive occurences of token
 * @max:  the maximum successive occurences of token
 * @data:  data associated to the transition
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a transition from the @from state to the target state
 * activated by a succession of input of value @token and whose number
 * is between @min and @max
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewCountTrans(xmlAutomataPtr am, xmlAutomataStatePtr from, xmlAutomataStatePtr to, const xmlChar * token, int min, int max, void * data) 
{
	xmlRegAtom * atom;
	int counter;
	if((am == NULL) || (from == NULL) || (token == NULL))
		return 0;
	if(min < 0)
		return 0;
	if((max < min) || (max < 1))
		return 0;
	atom = xmlRegNewAtom(am, XML_REGEXP_STRING);
	if(atom == NULL)
		return 0;
	atom->valuep = sstrdup(token);
	atom->data = data;
	if(min == 0)
		atom->min = 1;
	else
		atom->min = min;
	atom->max = max;
	/*
	 * associate a counter to the transition.
	 */
	counter = xmlRegGetCounter(am);
	am->counters[counter].min = min;
	am->counters[counter].max = max;

	/* xmlFAGenerateTransitions(am, from, to, atom); */
	if(to == NULL) {
		to = xmlRegNewState(am);
		xmlRegStatePush(am, to);
	}
	xmlRegStateAddTrans(am, from, atom, to, counter, -1);
	xmlRegAtomPush(am, atom);
	am->state = to;
	if(to == NULL)
		to = am->state;
	if(to == NULL)
		return 0;
	if(min == 0)
		xmlFAGenerateEpsilonTransition(am, from, to);
	return to;
}
/**
 * xmlAutomataNewOnceTrans2:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @token: the input string associated to that transition
 * @token2: the second input string associated to that transition
 * @min:  the minimum successive occurences of token
 * @max:  the maximum successive occurences of token
 * @data:  data associated to the transition
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a transition from the @from state to the target state
 * activated by a succession of input of value @token and @token2 and whose
 * number is between @min and @max, moreover that transition can only be
 * crossed once.
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewOnceTrans2(xmlAutomataPtr am, xmlAutomataStatePtr from,
    xmlAutomataStatePtr to, const xmlChar * token, const xmlChar * token2, int min, int max, void * data)
{
	xmlRegAtom * atom;
	int counter;
	if((am == NULL) || (from == NULL) || (token == NULL))
		return 0;
	if(min < 1)
		return 0;
	if((max < min) || (max < 1))
		return 0;
	atom = xmlRegNewAtom(am, XML_REGEXP_STRING);
	if(atom == NULL)
		return 0;
	if(isempty(token2)) {
		atom->valuep = sstrdup(token);
	}
	else {
		int lenn = sstrlen(token2);
		int lenp = sstrlen(token);
		xmlChar * str = static_cast<xmlChar *>(SAlloc::M(lenn + lenp + 2));
		if(!str) {
			xmlRegFreeAtom(atom);
			return 0;
		}
		memcpy(&str[0], token, lenp);
		str[lenp] = '|';
		memcpy(&str[lenp + 1], token2, lenn);
		str[lenn + lenp + 1] = 0;
		atom->valuep = str;
	}
	atom->data = data;
	atom->quant = XML_REGEXP_QUANT_ONCEONLY;
	atom->min = min;
	atom->max = max;
	/*
	 * associate a counter to the transition.
	 */
	counter = xmlRegGetCounter(am);
	am->counters[counter].min = 1;
	am->counters[counter].max = 1;

	/* xmlFAGenerateTransitions(am, from, to, atom); */
	if(to == NULL) {
		to = xmlRegNewState(am);
		xmlRegStatePush(am, to);
	}
	xmlRegStateAddTrans(am, from, atom, to, counter, -1);
	xmlRegAtomPush(am, atom);
	am->state = to;
	return to;
}

/**
 * xmlAutomataNewOnceTrans:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @token: the input string associated to that transition
 * @min:  the minimum successive occurences of token
 * @max:  the maximum successive occurences of token
 * @data:  data associated to the transition
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a transition from the @from state to the target state
 * activated by a succession of input of value @token and whose number
 * is between @min and @max, moreover that transition can only be crossed
 * once.
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewOnceTrans(xmlAutomataPtr am, xmlAutomataStatePtr from,
    xmlAutomataStatePtr to, const xmlChar * token, int min, int max, void * data)
{
	xmlRegAtom * atom;
	int counter;
	if((am == NULL) || (from == NULL) || (token == NULL))
		return 0;
	if(min < 1)
		return 0;
	if((max < min) || (max < 1))
		return 0;
	atom = xmlRegNewAtom(am, XML_REGEXP_STRING);
	if(atom == NULL)
		return 0;
	atom->valuep = sstrdup(token);
	atom->data = data;
	atom->quant = XML_REGEXP_QUANT_ONCEONLY;
	atom->min = min;
	atom->max = max;
	/*
	 * associate a counter to the transition.
	 */
	counter = xmlRegGetCounter(am);
	am->counters[counter].min = 1;
	am->counters[counter].max = 1;
	/* xmlFAGenerateTransitions(am, from, to, atom); */
	if(to == NULL) {
		to = xmlRegNewState(am);
		xmlRegStatePush(am, to);
	}
	xmlRegStateAddTrans(am, from, atom, to, counter, -1);
	xmlRegAtomPush(am, atom);
	am->state = to;
	return to;
}
/**
 * xmlAutomataNewState:
 * @am: an automata
 *
 * Create a new disconnected state in the automata
 *
 * Returns the new state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewState(xmlAutomataPtr am)
{
	xmlAutomataStatePtr to = 0;
	if(am) {
		to = xmlRegNewState(am);
		xmlRegStatePush(am, to);
	}
	return to;
}
/**
 * xmlAutomataNewEpsilon:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds an epsilon transition from the @from state to the
 * target state
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr FASTCALL xmlAutomataNewEpsilon(xmlAutomataPtr am, xmlAutomataStatePtr from, xmlAutomataStatePtr to)
{
	if(!am || !from)
		return 0;
	else {
		xmlFAGenerateEpsilonTransition(am, from, to);
		return to ? to : am->state;
	}
}
/**
 * xmlAutomataNewAllTrans:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @lax: allow to transition if not all all transitions have been activated
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds a an ALL transition from the @from state to the
 * target state. That transition is an epsilon transition allowed only when
 * all transitions from the @from node have been activated.
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewAllTrans(xmlAutomataPtr am, xmlAutomataStatePtr from, xmlAutomataStatePtr to, int lax)
{
	if((am == NULL) || (from == NULL))
		return 0;
	xmlFAGenerateAllTransition(am, from, to, lax);
	return to ? to : am->state;
}
/**
 * xmlAutomataNewCounter:
 * @am: an automata
 * @min:  the minimal value on the counter
 * @max:  the maximal value on the counter
 *
 * Create a new counter
 *
 * Returns the counter number or -1 in case of error
 */
int xmlAutomataNewCounter(xmlAutomataPtr am, int min, int max)
{
	int ret;
	if(am == NULL)
		return -1;
	ret = xmlRegGetCounter(am);
	if(ret < 0)
		return -1;
	am->counters[ret].min = min;
	am->counters[ret].max = max;
	return ret;
}
/**
 * xmlAutomataNewCountedTrans:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @counter: the counter associated to that transition
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds an epsilon transition from the @from state to the target state
 * which will increment the counter provided
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr FASTCALL xmlAutomataNewCountedTrans(xmlAutomataPtr am, xmlAutomataStatePtr from, xmlAutomataStatePtr to, int counter)
{
	if(!am || !from || counter < 0)
		return 0;
	xmlFAGenerateCountedEpsilonTransition(am, from, to, counter);
	return to ? to : am->state;
}
/**
 * xmlAutomataNewCounterTrans:
 * @am: an automata
 * @from: the starting point of the transition
 * @to: the target point of the transition or NULL
 * @counter: the counter associated to that transition
 *
 * If @to is NULL, this creates first a new target state in the automata
 * and then adds an epsilon transition from the @from state to the target state
 * which will be allowed only if the counter is within the right range.
 *
 * Returns the target state or NULL in case of error
 */
xmlAutomataStatePtr xmlAutomataNewCounterTrans(xmlAutomataPtr am, xmlAutomataStatePtr from, xmlAutomataStatePtr to, int counter)
{
	if((am == NULL) || (from == NULL) || (counter < 0))
		return 0;
	xmlFAGenerateCountedTransition(am, from, to, counter);
	return to ? to : am->state;
}
/**
 * xmlAutomataCompile:
 * @am: an automata
 *
 * Compile the automata into a Reg Exp ready for being executed.
 * The automata should be free after this point.
 *
 * Returns the compiled regexp or NULL in case of error
 */
xmlRegexp * xmlAutomataCompile(xmlAutomataPtr am)
{
	xmlRegexp * ret;
	if((am == NULL) || (am->error != 0)) 
		return 0;
	xmlFAEliminateEpsilonTransitions(am);
	/* xmlFAComputesDeterminism(am); */
	ret = xmlRegEpxFromParse(am);
	return ret;
}
/**
 * xmlAutomataIsDeterminist:
 * @am: an automata
 *
 * Checks if an automata is determinist.
 *
 * Returns 1 if true, 0 if not, and -1 in case of error
 */
int xmlAutomataIsDeterminist(xmlAutomataPtr am)
{
	int ret;
	if(am == NULL)
		return -1;
	ret = xmlFAComputesDeterminism(am);
	return ret;
}

#endif /* LIBXML_AUTOMATA_ENABLED */

#ifdef LIBXML_EXPR_ENABLED
// 
// Formal Expression handling code
// 
// Expression handling context
// 
struct _xmlExpCtxt {
	xmlDict * dict;
	xmlExpNodePtr * table;
	int size;
	int nbElems;
	int nb_nodes;
	int maxNodes;
	const char * expr;
	const char * cur;
	int nb_cons;
	int tabSize;
};
/**
 * xmlExpNewCtxt:
 * @maxNodes:  the maximum number of nodes
 * @dict:  optional dictionnary to use internally
 *
 * Creates a new context for manipulating expressions
 *
 * Returns the context or NULL in case of error
 */
xmlExpCtxtPtr xmlExpNewCtxt(int maxNodes, xmlDict * dict)
{
	xmlExpCtxtPtr ret;
	int size = 256;
	if(maxNodes <= 4096)
		maxNodes = 4096;
	ret = (xmlExpCtxtPtr)SAlloc::M(sizeof(xmlExpCtxt));
	if(!ret)
		return 0;
	memzero(ret, sizeof(xmlExpCtxt));
	ret->size = size;
	ret->nbElems = 0;
	ret->maxNodes = maxNodes;
	ret->table = (xmlExpNodePtr *)SAlloc::M(size * sizeof(xmlExpNodePtr));
	if(ret->table == NULL) {
		SAlloc::F(ret);
		return 0;
	}
	memzero(ret->table, size * sizeof(xmlExpNodePtr));
	if(!dict) {
		ret->dict = xmlDictCreate();
		if(ret->dict == NULL) {
			SAlloc::F(ret->table);
			SAlloc::F(ret);
			return 0;
		}
	}
	else {
		ret->dict = dict;
		xmlDictReference(ret->dict);
	}
	return ret;
}
/**
 * xmlExpFreeCtxt:
 * @ctxt:  an expression context
 *
 * Free an expression context
 */
void xmlExpFreeCtxt(xmlExpCtxtPtr ctxt)
{
	if(ctxt) {
		xmlDictFree(ctxt->dict);
		SAlloc::F(ctxt->table);
		SAlloc::F(ctxt);
	}
}
// 
// Structure associated to an expression node
// 
#define MAX_NODES 10000

/* #define DEBUG_DERIV */
/*
 * @todo 
 * - Wildcards
 * - public API for creation
 *
 * Started
 * - regression testing
 *
 * Done
 * - split into module and test tool
 * - memleaks
 */
typedef enum {
	XML_EXP_NILABLE = (1 << 0)
} xmlExpNodeInfo;

#define IS_NILLABLE(P_Node) ((P_Node)->info & XML_EXP_NILABLE)

struct _xmlExpNode {
	uchar type; /* xmlExpNodeType */
	uchar info; /* OR of xmlExpNodeInfo */
	ushort key; /* the hash key */
	uint ref; /* The number of references */
	int c_max;      /* the maximum length it can consume */
	xmlExpNodePtr exp_left;
	xmlExpNodePtr next; /* the next node in the hash table or free list */
	union {
		struct {
			int f_min;
			int f_max;
		} count;
		struct {
			xmlExpNodePtr f_right;
		} children;
		const xmlChar * f_str;
	} field;
};

#define exp_min field.count.f_min
#define exp_max field.count.f_max
/* #define exp_left field.children.f_left */
#define exp_right field.children.f_right
#define exp_str field.f_str

static xmlExpNodePtr xmlExpNewNode(xmlExpCtxtPtr ctxt, xmlExpNodeType type);
static xmlExpNode forbiddenExpNode = { XML_EXP_FORBID, 0, 0, 0, 0, NULL, NULL, {{ 0, 0}} };
xmlExpNodePtr forbiddenExp = &forbiddenExpNode;
static xmlExpNode emptyExpNode = { XML_EXP_EMPTY, 1, 0, 0, 0, NULL, NULL, {{ 0, 0}} };
xmlExpNodePtr emptyExp = &emptyExpNode;
// 
// The custom hash table for unicity and canonicalization of sub-expressions pointers						*
// 
/*
 * xmlExpHashNameComputeKey:
 * Calculate the hash key for a token
 */
static ushort xmlExpHashNameComputeKey(const xmlChar * name)
{
	ushort value = 0L;
	char ch;
	if(name) {
		value += 30 * (*name);
		while((ch = *name++) != 0) {
			value = (ushort)(value ^ ((value << 5) + (value >> 3) + static_cast<ulong>(ch)));
		}
	}
	return (value);
}
/*
 * xmlExpHashComputeKey:
 * Calculate the hash key for a compound expression
 */
static ushort xmlExpHashComputeKey(xmlExpNodeType type, xmlExpNodePtr left, xmlExpNodePtr right)
{
	ulong  value;
	ushort ret;
	switch(type) {
		case XML_EXP_SEQ:
		    value = left->key;
		    value += right->key;
		    value *= 3;
		    ret = (ushort)value;
		    break;
		case XML_EXP_OR:
		    value = left->key;
		    value += right->key;
		    value *= 7;
		    ret = (ushort)value;
		    break;
		case XML_EXP_COUNT:
		    value = left->key;
		    value += right->key;
		    ret = (ushort)value;
		    break;
		default:
		    ret = 0;
	}
	return ret;
}

static xmlExpNodePtr xmlExpNewNode(xmlExpCtxtPtr ctxt, xmlExpNodeType type)
{
	xmlExpNodePtr ret = 0;
	if(ctxt->nb_nodes < MAX_NODES) {
		ret = (xmlExpNodePtr)SAlloc::M(sizeof(xmlExpNode));
		if(ret) {
			memzero(ret, sizeof(xmlExpNode));
			ret->type = type;
			ret->next = NULL;
			ctxt->nb_nodes++;
			ctxt->nb_cons++;
		}
	}
	return ret;
}
/**
 * xmlExpHashGetEntry:
 * @table: the hash table
 *
 * Get the unique entry from the hash table. The entry is created if
 * needed. @left and @right are consumed, i.e. their ref count will
 * be decremented by the operation.
 *
 * Returns the pointer or NULL in case of error
 */
static xmlExpNodePtr xmlExpHashGetEntry(xmlExpCtxtPtr ctxt, xmlExpNodeType type,
    xmlExpNodePtr left, xmlExpNodePtr right, const xmlChar * name, int min, int max)
{
	ushort kbase, key;
	xmlExpNodePtr entry;
	xmlExpNodePtr insert;
	if(!ctxt)
		return 0;
	/*
	 * Check for duplicate and insertion location.
	 */
	if(type == XML_EXP_ATOM) {
		kbase = xmlExpHashNameComputeKey(name);
	}
	else if(type == XML_EXP_COUNT) {
		/* COUNT reduction rule 1 */
		/* a{1} -> a */
		if(min == max) {
			if(min == 1) {
				return (left);
			}
			else if(min == 0) {
				xmlExpFree(ctxt, left);
				return emptyExp;
			}
		}
		if(min < 0) {
			xmlExpFree(ctxt, left);
			return (forbiddenExp);
		}
		if(max == -1)
			kbase = min + 79;
		else
			kbase = max - min;
		kbase += left->key;
	}
	else if(type == XML_EXP_OR) {
		/* Forbid reduction rules */
		if(left->type == XML_EXP_FORBID) {
			xmlExpFree(ctxt, left);
			return (right);
		}
		if(right->type == XML_EXP_FORBID) {
			xmlExpFree(ctxt, right);
			return (left);
		}

		/* OR reduction rule 1 */
		/* a | a reduced to a */
		if(left == right) {
			left->ref--;
			return (left);
		}
		/* OR canonicalization rule 1 */
		/* linearize (a | b) | c into a | (b | c) */
		if((left->type == XML_EXP_OR) && (right->type != XML_EXP_OR)) {
			xmlExpNodePtr tmp = left;
			left = right;
			right = tmp;
		}
		/* OR reduction rule 2 */
		/* a | (a | b) and b | (a | b) are reduced to a | b */
		if(right->type == XML_EXP_OR) {
			if((left == right->exp_left) || (left == right->exp_right)) {
				xmlExpFree(ctxt, left);
				return (right);
			}
		}
		/* OR canonicalization rule 2 */
		/* linearize (a | b) | c into a | (b | c) */
		if(left->type == XML_EXP_OR) {
			xmlExpNodePtr tmp;
			/* OR canonicalization rule 2 */
			if((left->exp_right->type != XML_EXP_OR) && (left->exp_right->key < left->exp_left->key)) {
				tmp = left->exp_right;
				left->exp_right = left->exp_left;
				left->exp_left = tmp;
			}
			left->exp_right->ref++;
			tmp = xmlExpHashGetEntry(ctxt, XML_EXP_OR, left->exp_right, right, NULL, 0, 0);
			left->exp_left->ref++;
			tmp = xmlExpHashGetEntry(ctxt, XML_EXP_OR, left->exp_left, tmp, NULL, 0, 0);
			xmlExpFree(ctxt, left);
			return tmp;
		}
		if(right->type == XML_EXP_OR) {
			/* Ordering in the tree */
			/* C | (A | B) -> A | (B | C) */
			if(left->key > right->exp_right->key) {
				xmlExpNodePtr tmp;
				right->exp_right->ref++;
				tmp = xmlExpHashGetEntry(ctxt, XML_EXP_OR, right->exp_right, left, NULL, 0, 0);
				right->exp_left->ref++;
				tmp = xmlExpHashGetEntry(ctxt, XML_EXP_OR, right->exp_left, tmp, NULL, 0, 0);
				xmlExpFree(ctxt, right);
				return tmp;
			}
			/* Ordering in the tree */
			/* B | (A | C) -> A | (B | C) */
			if(left->key > right->exp_left->key) {
				xmlExpNodePtr tmp;
				right->exp_right->ref++;
				tmp = xmlExpHashGetEntry(ctxt, XML_EXP_OR, left, right->exp_right, NULL, 0, 0);
				right->exp_left->ref++;
				tmp = xmlExpHashGetEntry(ctxt, XML_EXP_OR, right->exp_left, tmp, NULL, 0, 0);
				xmlExpFree(ctxt, right);
				return tmp;
			}
		}
		/* we know both types are != XML_EXP_OR here */
		else if(left->key > right->key) {
			xmlExpNodePtr tmp = left;
			left = right;
			right = tmp;
		}
		kbase = xmlExpHashComputeKey(type, left, right);
	}
	else if(type == XML_EXP_SEQ) {
		if(left->type == XML_EXP_FORBID) { // Forbid reduction rules
			xmlExpFree(ctxt, right);
			return (left);
		}
		else if(right->type == XML_EXP_FORBID) {
			xmlExpFree(ctxt, left);
			return (right);
		}
		else if(right->type == XML_EXP_EMPTY) { // Empty reduction rules
			return (left);
		}
		else if(left->type == XML_EXP_EMPTY) {
			return (right);
		}
		else
			kbase = xmlExpHashComputeKey(type, left, right);
	}
	else
		return 0;
	key = kbase % ctxt->size;
	if(ctxt->table[key]) {
		for(insert = ctxt->table[key]; insert; insert = insert->next) {
			if((insert->key == kbase) && (insert->type == type)) {
				if(type == XML_EXP_ATOM) {
					if(name == insert->exp_str) {
						insert->ref++;
						return (insert);
					}
				}
				else if(type == XML_EXP_COUNT) {
					if((insert->exp_min == min) && (insert->exp_max == max) && (insert->exp_left == left)) {
						insert->ref++;
						left->ref--;
						return (insert);
					}
				}
				else if((insert->exp_left == left) && (insert->exp_right == right)) {
					insert->ref++;
					left->ref--;
					right->ref--;
					return (insert);
				}
			}
		}
	}
	entry = xmlExpNewNode(ctxt, type);
	if(entry == NULL)
		return 0;
	entry->key = kbase;
	if(type == XML_EXP_ATOM) {
		entry->exp_str = name;
		entry->c_max = 1;
	}
	else if(type == XML_EXP_COUNT) {
		entry->exp_min = min;
		entry->exp_max = max;
		entry->exp_left = left;
		if((min == 0) || (IS_NILLABLE(left)))
			entry->info |= XML_EXP_NILABLE;
		if(max < 0)
			entry->c_max = -1;
		else
			entry->c_max = max * entry->exp_left->c_max;
	}
	else {
		entry->exp_left = left;
		entry->exp_right = right;
		if(type == XML_EXP_OR) {
			if((IS_NILLABLE(left)) || (IS_NILLABLE(right)))
				entry->info |= XML_EXP_NILABLE;
			if((entry->exp_left->c_max == -1) || (entry->exp_right->c_max == -1))
				entry->c_max = -1;
			else if(entry->exp_left->c_max > entry->exp_right->c_max)
				entry->c_max = entry->exp_left->c_max;
			else
				entry->c_max = entry->exp_right->c_max;
		}
		else {
			if((IS_NILLABLE(left)) && (IS_NILLABLE(right)))
				entry->info |= XML_EXP_NILABLE;
			if((entry->exp_left->c_max == -1) || (entry->exp_right->c_max == -1))
				entry->c_max = -1;
			else
				entry->c_max = entry->exp_left->c_max + entry->exp_right->c_max;
		}
	}
	entry->ref = 1;
	if(ctxt->table[key])
		entry->next = ctxt->table[key];
	ctxt->table[key] = entry;
	ctxt->nbElems++;
	return (entry);
}
/**
 * xmlExpFree:
 * @ctxt: the expression context
 * @exp: the expression
 *
 * Dereference the expression
 */
void FASTCALL xmlExpFree(xmlExpCtxt * ctxt, xmlExpNode * pExp)
{
	if(pExp && (pExp != forbiddenExp) && (pExp != emptyExp)) {
		pExp->ref--;
		if(pExp->ref == 0) {
			// Unlink it first from the hash table
			ushort key = pExp->key % ctxt->size;
			if(ctxt->table[key] == pExp) {
				ctxt->table[key] = pExp->next;
			}
			else {
				for(xmlExpNode * tmp = ctxt->table[key]; tmp; tmp = tmp->next) {
					if(tmp->next == pExp) {
						tmp->next = pExp->next;
						break;
					}
				}
			}
			if(oneof2(pExp->type, XML_EXP_SEQ, XML_EXP_OR)) {
				xmlExpFree(ctxt, pExp->exp_left); // @recursion
				xmlExpFree(ctxt, pExp->exp_right); // @recursion
			}
			else if(pExp->type == XML_EXP_COUNT) {
				xmlExpFree(ctxt, pExp->exp_left); // @recursion
			}
			SAlloc::F(pExp);
			ctxt->nb_nodes--;
		}
	}
}
/**
 * xmlExpRef:
 * @exp: the expression
 *
 * Increase the reference count of the expression
 */
void xmlExpRef(xmlExpNodePtr exp)
{
	if(exp)
		exp->ref++;
}

/**
 * xmlExpNewAtom:
 * @ctxt: the expression context
 * @name: the atom name
 * @len: the atom name length in byte (or -1);
 *
 * Get the atom associated to this name from that context
 *
 * Returns the node or NULL in case of error
 */
xmlExpNodePtr xmlExpNewAtom(xmlExpCtxtPtr ctxt, const xmlChar * name, int len)
{
	if(!ctxt || !name)
		return 0;
	name = xmlDictLookup(ctxt->dict, name, len);
	if(!name)
		return 0;
	return (xmlExpHashGetEntry(ctxt, XML_EXP_ATOM, NULL, NULL, name, 0, 0));
}
/**
 * xmlExpNewOr:
 * @ctxt: the expression context
 * @left: left expression
 * @right: right expression
 *
 * Get the atom associated to the choice @left | @right
 * Note that @left and @right are consumed in the operation, to keep
 * an handle on them use xmlExpRef() and use xmlExpFree() to release them,
 * this is true even in case of failure (unless ctxt == NULL).
 *
 * Returns the node or NULL in case of error
 */
xmlExpNodePtr xmlExpNewOr(xmlExpCtxtPtr ctxt, xmlExpNodePtr left, xmlExpNodePtr right)
{
	if(!ctxt)
		return 0;
	if((left == NULL) || (right == NULL)) {
		xmlExpFree(ctxt, left);
		xmlExpFree(ctxt, right);
		return 0;
	}
	return (xmlExpHashGetEntry(ctxt, XML_EXP_OR, left, right, NULL, 0, 0));
}
/**
 * xmlExpNewSeq:
 * @ctxt: the expression context
 * @left: left expression
 * @right: right expression
 *
 * Get the atom associated to the sequence @left , @right
 * Note that @left and @right are consumed in the operation, to keep
 * an handle on them use xmlExpRef() and use xmlExpFree() to release them,
 * this is true even in case of failure (unless ctxt == NULL).
 *
 * Returns the node or NULL in case of error
 */
xmlExpNodePtr xmlExpNewSeq(xmlExpCtxtPtr ctxt, xmlExpNodePtr left, xmlExpNodePtr right)
{
	if(!ctxt)
		return 0;
	if((left == NULL) || (right == NULL)) {
		xmlExpFree(ctxt, left);
		xmlExpFree(ctxt, right);
		return 0;
	}
	return (xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, left, right, NULL, 0, 0));
}

/**
 * xmlExpNewRange:
 * @ctxt: the expression context
 * @subset: the expression to be repeated
 * @min: the lower bound for the repetition
 * @max: the upper bound for the repetition, -1 means infinite
 *
 * Get the atom associated to the range (@subset){@min, @max}
 * Note that @subset is consumed in the operation, to keep
 * an handle on it use xmlExpRef() and use xmlExpFree() to release it,
 * this is true even in case of failure (unless ctxt == NULL).
 *
 * Returns the node or NULL in case of error
 */
xmlExpNodePtr xmlExpNewRange(xmlExpCtxtPtr ctxt, xmlExpNodePtr subset, int min, int max)
{
	if(!ctxt)
		return 0;
	if((subset == NULL) || (min < 0) || (max < -1) || ((max >= 0) && (min > max))) {
		xmlExpFree(ctxt, subset);
		return 0;
	}
	return xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, subset, NULL, NULL, min, max);
}

/************************************************************************
*									*
*		Public API for operations on expressions		*
*									*
************************************************************************/

static int xmlExpGetLanguageInt(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, const xmlChar** list, int len, int nb)
{
	int tmp, tmp2;
tail:
	switch(exp->type) {
		case XML_EXP_EMPTY:
		    return 0;
		case XML_EXP_ATOM:
		    for(tmp = 0; tmp < nb; tmp++)
			    if(list[tmp] == exp->exp_str)
				    return 0;
		    if(nb >= len)
			    return -2;
		    list[nb] = exp->exp_str;
		    return 1;
		case XML_EXP_COUNT:
		    exp = exp->exp_left;
		    goto tail;
		case XML_EXP_SEQ:
		case XML_EXP_OR:
		    tmp = xmlExpGetLanguageInt(ctxt, exp->exp_left, list, len, nb);
		    if(tmp < 0)
			    return tmp;
		    tmp2 = xmlExpGetLanguageInt(ctxt, exp->exp_right, list, len,
		    nb + tmp);
		    if(tmp2 < 0)
			    return tmp2;
		    return (tmp + tmp2);
	}
	return -1;
}

/**
 * xmlExpGetLanguage:
 * @ctxt: the expression context
 * @exp: the expression
 * @langList: where to store the tokens
 * @len: the allocated length of @list
 *
 * Find all the strings used in @exp and store them in @list
 *
 * Returns the number of unique strings found, -1 in case of errors and
 *    -2 if there is more than @len strings
 */
int xmlExpGetLanguage(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, const xmlChar** langList, int len)
{
	if(!ctxt || (exp == NULL) || (langList == NULL) || (len <= 0))
		return -1;
	return (xmlExpGetLanguageInt(ctxt, exp, langList, len, 0));
}

static int xmlExpGetStartInt(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, const xmlChar** list, int len, int nb)
{
	int tmp, tmp2;
tail:
	switch(exp->type) {
		case XML_EXP_FORBID:
		    return 0;
		case XML_EXP_EMPTY:
		    return 0;
		case XML_EXP_ATOM:
		    for(tmp = 0; tmp < nb; tmp++)
			    if(list[tmp] == exp->exp_str)
				    return 0;
		    if(nb >= len)
			    return -2;
		    list[nb] = exp->exp_str;
		    return 1;
		case XML_EXP_COUNT:
		    exp = exp->exp_left;
		    goto tail;
		case XML_EXP_SEQ:
		    tmp = xmlExpGetStartInt(ctxt, exp->exp_left, list, len, nb);
		    if(tmp < 0)
			    return tmp;
		    if(IS_NILLABLE(exp->exp_left)) {
			    tmp2 = xmlExpGetStartInt(ctxt, exp->exp_right, list, len,
			    nb + tmp);
			    if(tmp2 < 0)
				    return tmp2;
			    tmp += tmp2;
		    }
		    return tmp;
		case XML_EXP_OR:
		    tmp = xmlExpGetStartInt(ctxt, exp->exp_left, list, len, nb);
		    if(tmp < 0)
			    return tmp;
		    tmp2 = xmlExpGetStartInt(ctxt, exp->exp_right, list, len,
		    nb + tmp);
		    if(tmp2 < 0)
			    return tmp2;
		    return (tmp + tmp2);
	}
	return -1;
}

/**
 * xmlExpGetStart:
 * @ctxt: the expression context
 * @exp: the expression
 * @tokList: where to store the tokens
 * @len: the allocated length of @list
 *
 * Find all the strings that appears at the start of the languages
 * accepted by @exp and store them in @list. E.g. for (a, b) | c
 * it will return the list [a, c]
 *
 * Returns the number of unique strings found, -1 in case of errors and
 *    -2 if there is more than @len strings
 */
int xmlExpGetStart(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, const xmlChar** tokList, int len)
{
	if(!ctxt || !exp || !tokList || (len <= 0))
		return -1;
	return xmlExpGetStartInt(ctxt, exp, tokList, len, 0);
}

/**
 * xmlExpIsNillable:
 * @exp: the expression
 *
 * Finds if the expression is nillable, i.e. if it accepts the empty sequqnce
 *
 * Returns 1 if nillable, 0 if not and -1 in case of error
 */
int xmlExpIsNillable(xmlExpNodePtr exp)
{
	return exp ? (IS_NILLABLE(exp) != 0) : -1;
}

static xmlExpNodePtr xmlExpStringDeriveInt(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, const xmlChar * str)
{
	xmlExpNodePtr ret;

	switch(exp->type) {
		case XML_EXP_EMPTY:
		    return (forbiddenExp);
		case XML_EXP_FORBID:
		    return (forbiddenExp);
		case XML_EXP_ATOM:
		    if(exp->exp_str == str) {
#ifdef DEBUG_DERIV
			    printf("deriv atom: equal => Empty\n");
#endif
			    ret = emptyExp;
		    }
		    else {
#ifdef DEBUG_DERIV
			    printf("deriv atom: mismatch => forbid\n");
#endif
			    /* @todo wildcards here */
			    ret = forbiddenExp;
		    }
		    return ret;
		case XML_EXP_OR: {
		    xmlExpNodePtr tmp;

#ifdef DEBUG_DERIV
		    printf("deriv or: => or(derivs)\n");
#endif
		    tmp = xmlExpStringDeriveInt(ctxt, exp->exp_left, str);
		    if(!tmp) {
			    return 0;
		    }
		    ret = xmlExpStringDeriveInt(ctxt, exp->exp_right, str);
		    if(!ret) {
			    xmlExpFree(ctxt, tmp);
			    return 0;
		    }
		    ret = xmlExpHashGetEntry(ctxt, XML_EXP_OR, tmp, ret,
			    NULL, 0, 0);
		    return ret;
	    }
		case XML_EXP_SEQ:
#ifdef DEBUG_DERIV
		    printf("deriv seq: starting with left\n");
#endif
		    ret = xmlExpStringDeriveInt(ctxt, exp->exp_left, str);
		    if(!ret) {
			    return 0;
		    }
		    else if(ret == forbiddenExp) {
			    if(IS_NILLABLE(exp->exp_left)) {
#ifdef DEBUG_DERIV
				    printf("deriv seq: left failed but nillable\n");
#endif
				    ret = xmlExpStringDeriveInt(ctxt, exp->exp_right, str);
			    }
		    }
		    else {
#ifdef DEBUG_DERIV
			    printf("deriv seq: left match => sequence\n");
#endif
			    exp->exp_right->ref++;
			    ret = xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, ret, exp->exp_right,
			    NULL, 0, 0);
		    }
		    return ret;
		case XML_EXP_COUNT: {
		    int min, max;
		    xmlExpNodePtr tmp;
		    if(exp->exp_max == 0)
			    return (forbiddenExp);
		    ret = xmlExpStringDeriveInt(ctxt, exp->exp_left, str);
		    if(!ret)
			    return 0;
		    if(ret == forbiddenExp) {
#ifdef DEBUG_DERIV
			    printf("deriv count: pattern mismatch => forbid\n");
#endif
			    return ret;
		    }
		    if(exp->exp_max == 1)
			    return ret;
		    if(exp->exp_max < 0) /* unbounded */
			    max = -1;
		    else
			    max = exp->exp_max - 1;
		    if(exp->exp_min > 0)
			    min = exp->exp_min - 1;
		    else
			    min = 0;
		    exp->exp_left->ref++;
		    tmp = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, exp->exp_left, NULL, NULL, min, max);
		    if(ret == emptyExp) {
#ifdef DEBUG_DERIV
			    printf("deriv count: match to empty => new count\n");
#endif
			    return tmp;
		    }
#ifdef DEBUG_DERIV
		    printf("deriv count: match => sequence with new count\n");
#endif
		    return xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, ret, tmp, NULL, 0, 0);
	    }
	}
	return 0;
}

/**
 * xmlExpStringDerive:
 * @ctxt: the expression context
 * @exp: the expression
 * @str: the string
 * @len: the string len in bytes if available
 *
 * Do one step of Brzozowski derivation of the expression @exp with
 * respect to the input string
 *
 * Returns the resulting expression or NULL in case of internal error
 */
xmlExpNodePtr xmlExpStringDerive(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, const xmlChar * str, int len)
{
	const xmlChar * input;
	if(!exp || !ctxt || !str)
		return 0;
	/*
	 * check the string is in the dictionnary, if yes use an interned
	 * copy, otherwise we know it's not an acceptable input
	 */
	input = xmlDictExists(ctxt->dict, str, len);
	if(!input) {
		return (forbiddenExp);
	}
	return (xmlExpStringDeriveInt(ctxt, exp, input));
}

static int xmlExpCheckCard(xmlExpNodePtr exp, xmlExpNodePtr sub)
{
	int ret = 1;
	if(sub->c_max == -1) {
		if(exp->c_max != -1)
			ret = 0;
	}
	else if((exp->c_max >= 0) && (exp->c_max < sub->c_max)) {
		ret = 0;
	}
#if 0
	if((IS_NILLABLE(sub)) && (!IS_NILLABLE(exp)))
		ret = 0;
#endif
	return ret;
}

static xmlExpNodePtr xmlExpExpDeriveInt(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, xmlExpNodePtr sub);
/**
 * xmlExpDivide:
 * @ctxt: the expressions context
 * @exp: the englobing expression
 * @sub: the subexpression
 * @mult: the multiple expression
 * @remain: the remain from the derivation of the multiple
 *
 * Check if exp is a multiple of sub, i.e. if there is a finite number n
 * so that sub{n} subsume exp
 *
 * Returns the multiple value if successful, 0 if it is not a multiple
 *    and -1 in case of internel error.
 */
static int xmlExpDivide(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, xmlExpNodePtr sub, xmlExpNodePtr * mult, xmlExpNodePtr * remain)
{
	int i;
	xmlExpNodePtr tmp, tmp2;
	ASSIGN_PTR(mult, NULL);
	ASSIGN_PTR(remain, NULL);
	if(exp->c_max == -1) return 0;
	if(IS_NILLABLE(exp) && (!IS_NILLABLE(sub))) 
		return 0;
	for(i = 1; i <= exp->c_max; i++) {
		sub->ref++;
		tmp = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, sub, NULL, NULL, i, i);
		if(!tmp) {
			return -1;
		}
		if(!xmlExpCheckCard(tmp, exp)) {
			xmlExpFree(ctxt, tmp);
			continue;
		}
		tmp2 = xmlExpExpDeriveInt(ctxt, tmp, exp);
		if(tmp2 == NULL) {
			xmlExpFree(ctxt, tmp);
			return -1;
		}
		if((tmp2 != forbiddenExp) && (IS_NILLABLE(tmp2))) {
			if(remain)
				*remain = tmp2;
			else
				xmlExpFree(ctxt, tmp2);
			if(mult)
				*mult = tmp;
			else
				xmlExpFree(ctxt, tmp);
#ifdef DEBUG_DERIV
			printf("Divide succeeded %d\n", i);
#endif
			return (i);
		}
		xmlExpFree(ctxt, tmp);
		xmlExpFree(ctxt, tmp2);
	}
#ifdef DEBUG_DERIV
	printf("Divide failed\n");
#endif
	return 0;
}
/**
 * xmlExpExpDeriveInt:
 * @ctxt: the expressions context
 * @exp: the englobing expression
 * @sub: the subexpression
 *
 * Try to do a step of Brzozowski derivation but at a higher level
 * the input being a subexpression.
 *
 * Returns the resulting expression or NULL in case of internal error
 */
static xmlExpNodePtr xmlExpExpDeriveInt(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, xmlExpNodePtr sub)
{
	xmlExpNodePtr ret, tmp, tmp2, tmp3;
	const xmlChar ** tab;
	int len, i;
	/*
	 * In case of equality and if the expression can only consume a finite
	 * amount, then the derivation is empty
	 */
	if((exp == sub) && (exp->c_max >= 0)) {
#ifdef DEBUG_DERIV
		printf("Equal(exp, sub) and finite -> Empty\n");
#endif
		return (emptyExp);
	}
	/*
	 * decompose sub sequence first
	 */
	if(sub->type == XML_EXP_EMPTY) {
#ifdef DEBUG_DERIV
		printf("Empty(sub) -> Empty\n");
#endif
		exp->ref++;
		return (exp);
	}
	if(sub->type == XML_EXP_SEQ) {
#ifdef DEBUG_DERIV
		printf("Seq(sub) -> decompose\n");
#endif
		tmp = xmlExpExpDeriveInt(ctxt, exp, sub->exp_left);
		if(!tmp)
			return 0;
		if(tmp == forbiddenExp)
			return tmp;
		ret = xmlExpExpDeriveInt(ctxt, tmp, sub->exp_right);
		xmlExpFree(ctxt, tmp);
		return ret;
	}
	if(sub->type == XML_EXP_OR) {
#ifdef DEBUG_DERIV
		printf("Or(sub) -> decompose\n");
#endif
		tmp = xmlExpExpDeriveInt(ctxt, exp, sub->exp_left);
		if(tmp == forbiddenExp)
			return tmp;
		if(!tmp)
			return 0;
		ret = xmlExpExpDeriveInt(ctxt, exp, sub->exp_right);
		if((ret == NULL) || (ret == forbiddenExp)) {
			xmlExpFree(ctxt, tmp);
			return ret;
		}
		return (xmlExpHashGetEntry(ctxt, XML_EXP_OR, tmp, ret, NULL, 0, 0));
	}
	if(!xmlExpCheckCard(exp, sub)) {
#ifdef DEBUG_DERIV
		printf("CheckCard(exp, sub) failed -> Forbid\n");
#endif
		return (forbiddenExp);
	}
	switch(exp->type) {
		case XML_EXP_EMPTY:
		    if(sub == emptyExp)
			    return (emptyExp);
#ifdef DEBUG_DERIV
		    printf("Empty(exp) -> Forbid\n");
#endif
		    return (forbiddenExp);
		case XML_EXP_FORBID:
#ifdef DEBUG_DERIV
		    printf("Forbid(exp) -> Forbid\n");
#endif
		    return (forbiddenExp);
		case XML_EXP_ATOM:
		    if(sub->type == XML_EXP_ATOM) {
			    /* @todo handle wildcards */
			    if(exp->exp_str == sub->exp_str) {
#ifdef DEBUG_DERIV
				    printf("Atom match -> Empty\n");
#endif
				    return (emptyExp);
			    }
#ifdef DEBUG_DERIV
			    printf("Atom mismatch -> Forbid\n");
#endif
			    return (forbiddenExp);
		    }
		    if((sub->type == XML_EXP_COUNT) && (sub->exp_max == 1) && (sub->exp_left->type == XML_EXP_ATOM)) {
			    /* @todo handle wildcards */
			    if(exp->exp_str == sub->exp_left->exp_str) {
#ifdef DEBUG_DERIV
				    printf("Atom match -> Empty\n");
#endif
				    return (emptyExp);
			    }
#ifdef DEBUG_DERIV
			    printf("Atom mismatch -> Forbid\n");
#endif
			    return (forbiddenExp);
		    }
#ifdef DEBUG_DERIV
		    printf("Compex exp vs Atom -> Forbid\n");
#endif
		    return (forbiddenExp);
		case XML_EXP_SEQ:
		    /* try to get the sequence consumed only if possible */
		    if(xmlExpCheckCard(exp->exp_left, sub)) {
			    /* See if the sequence can be consumed directly */
#ifdef DEBUG_DERIV
			    printf("Seq trying left only\n");
#endif
			    ret = xmlExpExpDeriveInt(ctxt, exp->exp_left, sub);
			    if((ret != forbiddenExp) && ret) {
#ifdef DEBUG_DERIV
				    printf("Seq trying left only worked\n");
#endif
				    /*
				 * @todo assumption here that we are determinist
				 *  i.e. we won't get to a nillable exp left
				 *  subset which could be matched by the right
				 *  part too.
				 * e.g.: (a | b)+,(a | c) and 'a+,a'
				     */
				    exp->exp_right->ref++;
				    return (xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, ret,
					    exp->exp_right, NULL, 0, 0));
			    }
#ifdef DEBUG_DERIV
		    }
		    else {
			    printf("Seq: left too short\n");
#endif
		    }
		    /* Try instead to decompose */
		    if(sub->type == XML_EXP_COUNT) {
			    int min, max;

#ifdef DEBUG_DERIV
			    printf("Seq: sub is a count\n");
#endif
			    ret = xmlExpExpDeriveInt(ctxt, exp->exp_left, sub->exp_left);
			    if(!ret)
				    return 0;
			    if(ret != forbiddenExp) {
#ifdef DEBUG_DERIV
				    printf("Seq , Count match on left\n");
#endif
				    if(sub->exp_max < 0)
					    max = -1;
				    else
					    max = (sub->exp_max -1);
				    if(sub->exp_min > 0)
					    min = (sub->exp_min -1);
				    else
					    min = 0;
				    exp->exp_right->ref++;
				    tmp = xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, ret,
				    exp->exp_right, NULL, 0, 0);
				    if(!tmp)
					    return 0;

				    sub->exp_left->ref++;
				    tmp2 = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT,
				    sub->exp_left, NULL, NULL, min, max);
				    if(tmp2 == NULL) {
					    xmlExpFree(ctxt, tmp);
					    return 0;
				    }
				    ret = xmlExpExpDeriveInt(ctxt, tmp, tmp2);
				    xmlExpFree(ctxt, tmp);
				    xmlExpFree(ctxt, tmp2);
				    return ret;
			    }
		    }
		    /* we made no progress on structured operations */
		    break;
		case XML_EXP_OR:
#ifdef DEBUG_DERIV
		    printf("Or , trying both side\n");
#endif
		    ret = xmlExpExpDeriveInt(ctxt, exp->exp_left, sub);
		    if(!ret)
			    return 0;
		    tmp = xmlExpExpDeriveInt(ctxt, exp->exp_right, sub);
		    if(!tmp) {
			    xmlExpFree(ctxt, ret);
			    return 0;
		    }
		    return (xmlExpHashGetEntry(ctxt, XML_EXP_OR, ret, tmp, NULL, 0, 0));
		case XML_EXP_COUNT: {
		    int min, max;

		    if(sub->type == XML_EXP_COUNT) {
			    /*
			 * Try to see if the loop is completely subsumed
			     */
			    tmp = xmlExpExpDeriveInt(ctxt, exp->exp_left, sub->exp_left);
			    if(!tmp)
				    return 0;
			    if(tmp == forbiddenExp) {
				    int mult;

#ifdef DEBUG_DERIV
				    printf("Count, Count inner don't subsume\n");
#endif
				    mult = xmlExpDivide(ctxt, sub->exp_left, exp->exp_left, NULL, &tmp);
				    if(mult <= 0) {
#ifdef DEBUG_DERIV
					    printf("Count, Count not multiple => forbidden\n");
#endif
					    return (forbiddenExp);
				    }
				    if(sub->exp_max == -1) {
					    max = -1;
					    if(exp->exp_max == -1) {
						    if(exp->exp_min <= sub->exp_min * mult)
							    min = 0;
						    else
							    min = (exp->exp_min - sub->exp_min * mult);
					    }
					    else {
#ifdef DEBUG_DERIV
						    printf("Count, Count finite can't subsume infinite\n");
#endif
						    xmlExpFree(ctxt, tmp);
						    return (forbiddenExp);
					    }
				    }
				    else {
					    if(exp->exp_max == -1) {
#ifdef DEBUG_DERIV
						    printf("Infinite loop consume mult finite loop\n");
#endif
						    if(exp->exp_min > sub->exp_min * mult) {
							    max = -1;
							    min = exp->exp_min - sub->exp_min * mult;
						    }
						    else {
							    max = -1;
							    min = 0;
						    }
					    }
					    else {
						    if(exp->exp_max < sub->exp_max * mult) {
#ifdef DEBUG_DERIV
							    printf("loops max mult mismatch => forbidden\n");
#endif
							    xmlExpFree(ctxt, tmp);
							    return (forbiddenExp);
						    }
						    if(sub->exp_max * mult > exp->exp_min)
							    min = 0;
						    else
							    min = exp->exp_min - sub->exp_max * mult;
						    max = exp->exp_max - sub->exp_max * mult;
					    }
				    }
			    }
			    else if(!IS_NILLABLE(tmp)) {
				    /*
				 * @todo loop here to try to grow if working on finite
				 *  blocks.
				     */
#ifdef DEBUG_DERIV
				    printf("Count, Count remain not nillable => forbidden\n");
#endif
				    xmlExpFree(ctxt, tmp);
				    return (forbiddenExp);
			    }
			    else if(sub->exp_max == -1) {
				    if(exp->exp_max == -1) {
					    if(exp->exp_min <= sub->exp_min) {
#ifdef DEBUG_DERIV
						    printf("Infinite loops Okay => COUNT(0,Inf)\n");
#endif
						    max = -1;
						    min = 0;
					    }
					    else {
#ifdef DEBUG_DERIV
						    printf("Infinite loops min => Count(X,Inf)\n");
#endif
						    max = -1;
						    min = exp->exp_min - sub->exp_min;
					    }
				    }
				    else if(exp->exp_min > sub->exp_min) {
#ifdef DEBUG_DERIV
					    printf("loops min mismatch 1 => forbidden ???\n");
#endif
					    xmlExpFree(ctxt, tmp);
					    return (forbiddenExp);
				    }
				    else {
					    max = -1;
					    min = 0;
				    }
			    }
			    else {
				    if(exp->exp_max == -1) {
#ifdef DEBUG_DERIV
					    printf("Infinite loop consume finite loop\n");
#endif
					    if(exp->exp_min > sub->exp_min) {
						    max = -1;
						    min = exp->exp_min - sub->exp_min;
					    }
					    else {
						    max = -1;
						    min = 0;
					    }
				    }
				    else {
					    if(exp->exp_max < sub->exp_max) {
#ifdef DEBUG_DERIV
						    printf("loops max mismatch => forbidden\n");
#endif
						    xmlExpFree(ctxt, tmp);
						    return (forbiddenExp);
					    }
					    if(sub->exp_max > exp->exp_min)
						    min = 0;
					    else
						    min = exp->exp_min - sub->exp_max;
					    max = exp->exp_max - sub->exp_max;
				    }
			    }
#ifdef DEBUG_DERIV
			    printf("loops match => SEQ(COUNT())\n");
#endif
			    exp->exp_left->ref++;
			    tmp2 = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, exp->exp_left, NULL, NULL, min, max);
			    if(tmp2 == NULL) {
				    return 0;
			    }
			    ret = xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, tmp, tmp2, NULL, 0, 0);
			    return ret;
		    }
		    tmp = xmlExpExpDeriveInt(ctxt, exp->exp_left, sub);
		    if(!tmp)
			    return 0;
		    if(tmp == forbiddenExp) {
#ifdef DEBUG_DERIV
			    printf("loop mismatch => forbidden\n");
#endif
			    return (forbiddenExp);
		    }
		    if(exp->exp_min > 0)
			    min = exp->exp_min - 1;
		    else
			    min = 0;
		    if(exp->exp_max < 0)
			    max = -1;
		    else
			    max = exp->exp_max - 1;

#ifdef DEBUG_DERIV
		    printf("loop match => SEQ(COUNT())\n");
#endif
		    exp->exp_left->ref++;
		    tmp2 = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, exp->exp_left, NULL, NULL, min, max);
		    if(tmp2 == NULL)
			    return 0;
		    ret = xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, tmp, tmp2, NULL, 0, 0);
		    return ret;
	    }
	}

#ifdef DEBUG_DERIV
	printf("Fallback to derivative\n");
#endif
	if(IS_NILLABLE(sub)) {
		if(!(IS_NILLABLE(exp)))
			return (forbiddenExp);
		else
			ret = emptyExp;
	}
	else
		ret = NULL;
	/*
	 * here the structured derivation made no progress so
	 * we use the default token based derivation to force one more step
	 */
	if(ctxt->tabSize == 0)
		ctxt->tabSize = 40;
	tab = static_cast<const xmlChar **>(SAlloc::M(ctxt->tabSize * sizeof(const xmlChar *)));
	if(tab == NULL) {
		return 0;
	}
	/*
	 * collect all the strings accepted by the subexpression on input
	 */
	len = xmlExpGetStartInt(ctxt, sub, tab, ctxt->tabSize, 0);
	while(len < 0) {
		const xmlChar ** temp;
		temp = static_cast<const xmlChar **>(SAlloc::R((xmlChar **)tab, ctxt->tabSize * 2 * sizeof(const xmlChar *)));
		if(temp == NULL) {
			SAlloc::F((xmlChar **)tab);
			return 0;
		}
		tab = temp;
		ctxt->tabSize *= 2;
		len = xmlExpGetStartInt(ctxt, sub, tab, ctxt->tabSize, 0);
	}
	for(i = 0; i < len; i++) {
		tmp = xmlExpStringDeriveInt(ctxt, exp, tab[i]);
		if(!tmp || tmp == forbiddenExp) {
			xmlExpFree(ctxt, ret);
			SAlloc::F((xmlChar **)tab);
			return tmp;
		}
		tmp2 = xmlExpStringDeriveInt(ctxt, sub, tab[i]);
		if((tmp2 == NULL) || (tmp2 == forbiddenExp)) {
			xmlExpFree(ctxt, tmp);
			xmlExpFree(ctxt, ret);
			SAlloc::F((xmlChar **)tab);
			return tmp;
		}
		tmp3 = xmlExpExpDeriveInt(ctxt, tmp, tmp2);
		xmlExpFree(ctxt, tmp);
		xmlExpFree(ctxt, tmp2);

		if((tmp3 == NULL) || (tmp3 == forbiddenExp)) {
			xmlExpFree(ctxt, ret);
			SAlloc::F((xmlChar **)tab);
			return (tmp3);
		}

		if(!ret)
			ret = tmp3;
		else {
			ret = xmlExpHashGetEntry(ctxt, XML_EXP_OR, ret, tmp3, NULL, 0, 0);
			if(!ret) {
				SAlloc::F((xmlChar **)tab);
				return 0;
			}
		}
	}
	SAlloc::F((xmlChar **)tab);
	return ret;
}

/**
 * xmlExpExpDerive:
 * @ctxt: the expressions context
 * @exp: the englobing expression
 * @sub: the subexpression
 *
 * Evaluates the expression resulting from @exp consuming a sub expression @sub
 * Based on algebraic derivation and sometimes direct Brzozowski derivation
 * it usually tatkes less than linear time and can handle expressions generating
 * infinite languages.
 *
 * Returns the resulting expression or NULL in case of internal error, the
 *    result must be freed
 */
xmlExpNodePtr xmlExpExpDerive(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, xmlExpNodePtr sub)
{
	if(!exp || !ctxt || !sub)
		return 0;
	/*
	 * O(1) speedups
	 */
	if(IS_NILLABLE(sub) && (!IS_NILLABLE(exp))) {
#ifdef DEBUG_DERIV
		printf("Sub nillable and not exp : can't subsume\n");
#endif
		return (forbiddenExp);
	}
	if(xmlExpCheckCard(exp, sub) == 0) {
#ifdef DEBUG_DERIV
		printf("sub generate longuer sequances than exp : can't subsume\n");
#endif
		return (forbiddenExp);
	}
	return (xmlExpExpDeriveInt(ctxt, exp, sub));
}
/**
 * xmlExpSubsume:
 * @ctxt: the expressions context
 * @exp: the englobing expression
 * @sub: the subexpression
 *
 * Check whether @exp accepts all the languages accexpted by @sub
 * the input being a subexpression.
 *
 * Returns 1 if true 0 if false and -1 in case of failure.
 */
int xmlExpSubsume(xmlExpCtxtPtr ctxt, xmlExpNodePtr exp, xmlExpNodePtr sub)
{
	xmlExpNodePtr tmp;
	if((exp == NULL) || (ctxt == NULL) || (sub == NULL))
		return -1;
	/*
	 * @todo speedup by checking the language of sub is a subset of the
	 *  language of exp
	 */
	/*
	 * O(1) speedups
	 */
	if(IS_NILLABLE(sub) && (!IS_NILLABLE(exp))) {
#ifdef DEBUG_DERIV
		printf("Sub nillable and not exp : can't subsume\n");
#endif
		return 0;
	}
	if(xmlExpCheckCard(exp, sub) == 0) {
#ifdef DEBUG_DERIV
		printf("sub generate longuer sequances than exp : can't subsume\n");
#endif
		return 0;
	}
	tmp = xmlExpExpDeriveInt(ctxt, exp, sub);
#ifdef DEBUG_DERIV
	printf("Result derivation :\n");
	PRINT_EXP(tmp);
#endif
	if(!tmp)
		return -1;
	if(tmp == forbiddenExp)
		return 0;
	if(tmp == emptyExp)
		return 1;
	if(tmp && (IS_NILLABLE(tmp))) {
		xmlExpFree(ctxt, tmp);
		return 1;
	}
	xmlExpFree(ctxt, tmp);
	return 0;
}
//
// Parsing expression
//
static xmlExpNodePtr xmlExpParseExpr(xmlExpCtxtPtr ctxt);

#undef CUR
#define CUR (*ctxt->cur)
#undef NEXT
#define NEXT ctxt->cur++;
#undef IS_BLANK
#define IS_BLANK(c) oneof4(c, ' ', '\t', '\n', '\r')
#define SKIP_BLANKS while(IS_BLANK(*ctxt->cur)) ctxt->cur++;

static int xmlExpParseNumber(xmlExpCtxtPtr ctxt)
{
	int ret = 0;
	SKIP_BLANKS
	if(CUR == '*') {
		NEXT
		return -1;
	}
	if((CUR < '0') || (CUR > '9'))
		return -1;
	while((CUR >= '0') && (CUR <= '9')) {
		ret = ret * 10 + (CUR - '0');
		NEXT
	}
	return ret;
}

static xmlExpNodePtr xmlExpParseOr(xmlExpCtxtPtr ctxt)
{
	const char * base;
	xmlExpNodePtr ret;
	const xmlChar * val;
	SKIP_BLANKS
	    base = ctxt->cur;
	if(*ctxt->cur == '(') {
		NEXT
		    ret = xmlExpParseExpr(ctxt);
		SKIP_BLANKS
		if(*ctxt->cur != ')') {
			fprintf(stderr, "unbalanced '(' : %s\n", base);
			xmlExpFree(ctxt, ret);
			return 0;
		}
		NEXT;
		SKIP_BLANKS
		goto parse_quantifier;
	}
	while((CUR != 0) && (!(IS_BLANK(CUR))) && (CUR != '(') &&
	    (CUR != ')') && (CUR != '|') && (CUR != ',') && (CUR != '{') &&
	    (CUR != '*') && (CUR != '+') && (CUR != '?') && (CUR != '}'))
		NEXT;
	val = xmlDictLookup(ctxt->dict, BAD_CAST base, ctxt->cur - base);
	if(!val)
		return 0;
	ret = xmlExpHashGetEntry(ctxt, XML_EXP_ATOM, NULL, NULL, val, 0, 0);
	if(!ret)
		return 0;
	SKIP_BLANKS
parse_quantifier:
	if(CUR == '{') {
		int min, max;

		NEXT
		    min = xmlExpParseNumber(ctxt);
		if(min < 0) {
			xmlExpFree(ctxt, ret);
			return 0;
		}
		SKIP_BLANKS
		if(CUR == ',') {
			NEXT
			    max = xmlExpParseNumber(ctxt);
			SKIP_BLANKS
		}
		else
			max = min;
		if(CUR != '}') {
			xmlExpFree(ctxt, ret);
			return 0;
		}
		NEXT
		    ret = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, ret, NULL, NULL,
		    min, max);
		SKIP_BLANKS
	}
	else if(CUR == '?') {
		NEXT
		    ret = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, ret, NULL, NULL,
		    0, 1);
		SKIP_BLANKS
	}
	else if(CUR == '+') {
		NEXT
		    ret = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, ret, NULL, NULL,
		    1, -1);
		SKIP_BLANKS
	}
	else if(CUR == '*') {
		NEXT
		    ret = xmlExpHashGetEntry(ctxt, XML_EXP_COUNT, ret, NULL, NULL,
		    0, -1);
		SKIP_BLANKS
	}
	return ret;
}

static xmlExpNodePtr xmlExpParseSeq(xmlExpCtxtPtr ctxt)
{
	xmlExpNodePtr right;
	xmlExpNodePtr ret = xmlExpParseOr(ctxt);
	SKIP_BLANKS
	while(CUR == '|') {
		NEXT
		    right = xmlExpParseOr(ctxt);
		if(right == NULL) {
			xmlExpFree(ctxt, ret);
			return 0;
		}
		ret = xmlExpHashGetEntry(ctxt, XML_EXP_OR, ret, right, NULL, 0, 0);
		if(!ret)
			return 0;
	}
	return ret;
}

static xmlExpNodePtr xmlExpParseExpr(xmlExpCtxtPtr ctxt)
{
	xmlExpNodePtr right;
	xmlExpNodePtr ret = xmlExpParseSeq(ctxt);
	SKIP_BLANKS
	while(CUR == ',') {
		NEXT
		    right = xmlExpParseSeq(ctxt);
		if(right == NULL) {
			xmlExpFree(ctxt, ret);
			return 0;
		}
		ret = xmlExpHashGetEntry(ctxt, XML_EXP_SEQ, ret, right, NULL, 0, 0);
		if(!ret)
			return 0;
	}
	return ret;
}
/**
 * xmlExpParse:
 * @ctxt: the expressions context
 * @expr: the 0 terminated string
 *
 * Minimal parser for regexps, it understand the following constructs
 *  - string terminals
 *  - choice operator |
 *  - sequence operator ,
 *  - subexpressions (...)
 *  - usual cardinality operators + * and ?
 *  - finite sequences  { min, max }
 *  - infinite sequences { min, * }
 * There is minimal checkings made especially no checking on strings values
 *
 * Returns a new expression or NULL in case of failure
 */
xmlExpNodePtr xmlExpParse(xmlExpCtxtPtr ctxt, const char * expr)
{
	ctxt->expr = expr;
	ctxt->cur = expr;
	xmlExpNodePtr ret = xmlExpParseExpr(ctxt);
	SKIP_BLANKS
	if(*ctxt->cur) {
		xmlExpFree(ctxt, ret);
		ret = 0;
	}
	return ret;
}

static void FASTCALL xmlExpDumpInt(xmlBuffer * buf, xmlExpNodePtr expr, int glob)
{
	xmlExpNodePtr c;
	if(expr == NULL)
		return;
	if(glob)
		xmlBufferWriteChar(buf, "(");
	switch(expr->type) {
		case XML_EXP_EMPTY: xmlBufferWriteChar(buf, "empty"); break;
		case XML_EXP_FORBID: xmlBufferWriteChar(buf, "forbidden"); break;
		case XML_EXP_ATOM: xmlBufferWriteCHAR(buf, expr->exp_str); break;
		case XML_EXP_SEQ:
		    c = expr->exp_left;
			xmlExpDumpInt(buf, c, oneof2(c->type, XML_EXP_SEQ, XML_EXP_OR) ? 1 : 0);
		    xmlBufferWriteChar(buf, " , ");
		    c = expr->exp_right;
			xmlExpDumpInt(buf, c, oneof2(c->type, XML_EXP_SEQ, XML_EXP_OR) ? 1 : 0);
		    break;
		case XML_EXP_OR:
		    c = expr->exp_left;
		    if((c->type == XML_EXP_SEQ) || (c->type == XML_EXP_OR))
			    xmlExpDumpInt(buf, c, 1);
		    else
			    xmlExpDumpInt(buf, c, 0);
		    xmlBufferWriteChar(buf, " | ");
		    c = expr->exp_right;
		    if((c->type == XML_EXP_SEQ) || (c->type == XML_EXP_OR))
			    xmlExpDumpInt(buf, c, 1);
		    else
			    xmlExpDumpInt(buf, c, 0);
		    break;
		case XML_EXP_COUNT: {
		    char rep[40];
		    c = expr->exp_left;
		    if((c->type == XML_EXP_SEQ) || (c->type == XML_EXP_OR))
			    xmlExpDumpInt(buf, c, 1);
		    else
			    xmlExpDumpInt(buf, c, 0);
		    if((expr->exp_min == 0) && (expr->exp_max == 1)) {
			    rep[0] = '?';
			    rep[1] = 0;
		    }
		    else if((expr->exp_min == 0) && (expr->exp_max == -1)) {
			    rep[0] = '*';
			    rep[1] = 0;
		    }
		    else if((expr->exp_min == 1) && (expr->exp_max == -1)) {
			    rep[0] = '+';
			    rep[1] = 0;
		    }
		    else if(expr->exp_max == expr->exp_min) {
			    snprintf(rep, 39, "{%d}", expr->exp_min);
		    }
		    else if(expr->exp_max < 0) {
			    snprintf(rep, 39, "{%d,inf}", expr->exp_min);
		    }
		    else {
			    snprintf(rep, 39, "{%d,%d}", expr->exp_min, expr->exp_max);
		    }
		    rep[39] = 0;
		    xmlBufferWriteChar(buf, rep);
		    break;
	    }
		default:
		    fprintf(stderr, "Error in tree\n");
	}
	if(glob)
		xmlBufferWriteChar(buf, ")");
}

/**
 * xmlExpDump:
 * @buf:  a buffer to receive the output
 * @expr:  the compiled expression
 *
 * Serialize the expression as compiled to the buffer
 */
void xmlExpDump(xmlBuffer * buf, xmlExpNodePtr expr)
{
	if(buf && expr)
		xmlExpDumpInt(buf, expr, 0);
}
/**
 * xmlExpMaxToken:
 * @expr: a compiled expression
 *
 * Indicate the maximum number of input a expression can accept
 *
 * Returns the maximum length or -1 in case of error
 */
int xmlExpMaxToken(xmlExpNodePtr expr) { return expr ? expr->c_max : -1; }
/**
 * xmlExpCtxtNbNodes:
 * @ctxt: an expression context
 *
 * Debugging facility provides the number of allocated nodes at a that point
 *
 * Returns the number of nodes in use or -1 in case of error
 */
int xmlExpCtxtNbNodes(xmlExpCtxtPtr ctxt) { return ctxt ? ctxt->nb_nodes : -1; }
/**
 * xmlExpCtxtNbCons:
 * @ctxt: an expression context
 *
 * Debugging facility provides the number of allocated nodes over lifetime
 *
 * Returns the number of nodes ever allocated or -1 in case of error
 */
int xmlExpCtxtNbCons(xmlExpCtxtPtr ctxt) { return ctxt ? ctxt->nb_cons : -1; }

#endif /* LIBXML_EXPR_ENABLED */
#define bottom_xmlregexp
//#include "elfgcchack.h"
#endif /* LIBXML_REGEXP_ENABLED */
