/*
 * xpointer.c : Code to handle XML Pointer
 *
 * Base implementation was made accordingly to
 * W3C Candidate Recommendation 7 June 2000
 * http://www.w3.org/TR/2000/CR-xptr-20000607
 *
 * Added support for the element() scheme described in:
 * W3C Proposed Recommendation 13 November 2002
 * http://www.w3.org/TR/2002/PR-xptr-element-20021113/
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */
#define IN_LIBXML
#include "libxml.h"
#pragma hdrstop
/*
 * @todo better handling of error cases, the full expression should
 *  be parsed beforehand instead of a progressive evaluation
 * @todo Access into entities references are not supported now ...
 *  need a start to be able to pop out of entities refs since
 *  parent is the endity declaration, not the ref.
 */
#ifdef LIBXML_XPTR_ENABLED
#define XPTR_XMLNS_SCHEME // Add support of the xmlns() xpointer scheme to initialize the namespaces 
/* #define DEBUG_RANGES */
#define TODO    xmlGenericError(0, "Unimplemented block at %s:%d\n", __FILE__, __LINE__);
#define STRANGE xmlGenericError(0, "Internal error at %s:%d\n", __FILE__, __LINE__);
// 
// Some factorized error routines
// 
/**
 * xmlXPtrErrMemory:
 * @extra:  extra informations
 *
 * Handle a redefinition of attribute error
 */
static void FASTCALL xmlXPtrErrMemory(const char * extra)
{
	__xmlRaiseError(0, 0, 0, 0, 0, XML_FROM_XPOINTER, XML_ERR_NO_MEMORY, XML_ERR_ERROR, NULL, 0, extra, 0, 0, 0, 0, "Memory allocation failed : %s\n", extra);
}
/**
 * xmlXPtrErr:
 * @ctxt:  an XPTR evaluation context
 * @extra:  extra informations
 *
 * Handle a redefinition of attribute error
 */
static void FASTCALL xmlXPtrErr(xmlXPathParserContextPtr ctxt, int error, const char * msg, const xmlChar * extra)
{
	if(ctxt)
		ctxt->error = error;
	if(!ctxt || !ctxt->context) {
		__xmlRaiseError(0, 0, 0, 0, 0, XML_FROM_XPOINTER, error, XML_ERR_ERROR, NULL, 0, (const char *)extra, NULL, NULL, 0, 0, msg, extra);
	}
	else {
		ctxt->context->lastError.domain = XML_FROM_XPOINTER;
		ctxt->context->lastError.code = error;
		ctxt->context->lastError.level = XML_ERR_ERROR;
		ctxt->context->lastError.str1 = (char *)sstrdup(ctxt->base);
		ctxt->context->lastError.int1 = ctxt->cur - ctxt->base;
		ctxt->context->lastError.P_Node = ctxt->context->debugNode;
		if(ctxt->context->error) {
			ctxt->context->error(ctxt->context->userData, &ctxt->context->lastError);
		}
		else {
			__xmlRaiseError(0, 0, 0, 0, ctxt->context->debugNode, XML_FROM_XPOINTER, error, XML_ERR_ERROR, NULL, 0, (const char *)extra, (const char *)ctxt->base, NULL,
				ctxt->cur - ctxt->base, 0, msg, extra);
		}
	}
}

/************************************************************************
*									*
*		A few helper functions for child sequences		*
*									*
************************************************************************/
/* xmlXPtrAdvanceNode is a private function, but used by xinclude.c */
xmlNode * xmlXPtrAdvanceNode(xmlNode * cur, int * level);
/**
 * xmlXPtrGetArity:
 * @cur:  the node
 *
 * Returns the number of child for an element, -1 in case of error
 */
static int FASTCALL xmlXPtrGetArity(xmlNode * cur) 
{
	int i;
	if(!cur || (cur->type == XML_NAMESPACE_DECL))
		return -1;
	cur = cur->children;
	for(i = 0; cur; cur = cur->next) {
		if(oneof3(cur->type, XML_ELEMENT_NODE, XML_DOCUMENT_NODE, XML_HTML_DOCUMENT_NODE)) {
			i++;
		}
	}
	return i;
}
/**
 * xmlXPtrGetIndex:
 * @cur:  the node
 *
 * Returns the index of the node in its parent children list, -1
 *    in case of error
 */
static int xmlXPtrGetIndex(xmlNode * cur) 
{
	int i;
	if(!cur || (cur->type == XML_NAMESPACE_DECL))
		return -1;
	for(i = 1; cur; cur = cur->prev) {
		if(oneof3(cur->type, XML_ELEMENT_NODE, XML_DOCUMENT_NODE, XML_HTML_DOCUMENT_NODE)) {
			i++;
		}
	}
	return (i);
}
/**
 * xmlXPtrGetNthChild:
 * @cur:  the node
 * @no:  the child number
 *
 * Returns the @no'th element child of @cur or NULL
 */
static xmlNode * FASTCALL xmlXPtrGetNthChild(xmlNode * cur, int no)
{
	if(cur && cur->type != XML_NAMESPACE_DECL) {
		cur = cur->children;
		for(int i = 0; i <= no; cur = cur->next) {
			if(!cur)
				return cur;
			else {
				if(oneof3(cur->type, XML_ELEMENT_NODE, XML_DOCUMENT_NODE, XML_HTML_DOCUMENT_NODE)) {
					i++;
					if(i == no)
						break;
				}
			}
		}
	}
	return cur;
}

/************************************************************************
*									*
*		Handling of XPointer specific types			*
*									*
************************************************************************/

/**
 * xmlXPtrCmpPoints:
 * @node1:  the first node
 * @index1:  the first index
 * @node2:  the second node
 * @index2:  the second index
 *
 * Compare two points w.r.t document order
 *
 * Returns -2 in case of error 1 if first point < second point, 0 if
 *    that's the same point, -1 otherwise
 */
static int xmlXPtrCmpPoints(xmlNode * node1, int index1, xmlNode * node2, int index2) 
{
	if(!node1 || !node2)
		return -2;
	else {
		// 
		// a couple of optimizations which will avoid computations in most cases
		// 
		if(node1 == node2) {
			if(index1 < index2)
				return 1;
			else if(index1 > index2)
				return -1;
			else
				return 0;
		}
		else
			return xmlXPathCmpNodes(node1, node2);
	}
}

/**
 * xmlXPtrNewPoint:
 * @node:  the xmlNodePtr
 * @indx:  the indx within the node
 *
 * Create a new xmlXPathObjectPtr of type point
 *
 * Returns the newly created object.
 */
static xmlXPathObjectPtr xmlXPtrNewPoint(xmlNode * P_Node, int indx)
{
	xmlXPathObjectPtr ret;
	if(!P_Node)
		return 0;
	if(indx < 0)
		return 0;
	ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
	if(!ret) {
		xmlXPtrErrMemory("allocating point");
		return 0;
	}
	memzero(ret, sizeof(xmlXPathObject));
	ret->type = XPATH_POINT;
	ret->user = (void *)P_Node;
	ret->index = indx;
	return ret;
}

/**
 * xmlXPtrRangeCheckOrder:
 * @range:  an object range
 *
 * Make sure the points in the range are in the right order
 */
static void xmlXPtrRangeCheckOrder(xmlXPathObjectPtr range)
{
	int tmp;
	xmlNode * tmp2;
	if(range == NULL)
		return;
	if(range->type != XPATH_RANGE)
		return;
	if(range->user2 == NULL)
		return;
	tmp = xmlXPtrCmpPoints((xmlNode *)range->user, range->index, (xmlNode *)range->user2, range->index2);
	if(tmp == -1) {
		tmp2 = (xmlNode *)range->user;
		range->user = range->user2;
		range->user2 = tmp2;
		tmp = range->index;
		range->index = range->index2;
		range->index2 = tmp;
	}
}

/**
 * xmlXPtrRangesEqual:
 * @range1:  the first range
 * @range2:  the second range
 *
 * Compare two ranges
 *
 * Returns 1 if equal, 0 otherwise
 */
static int xmlXPtrRangesEqual(xmlXPathObjectPtr range1, xmlXPathObjectPtr range2) 
{
	if(range1 == range2)
		return 1;
	if((range1 == NULL) || (range2 == NULL))
		return 0;
	if(range1->type != range2->type)
		return 0;
	if(range1->type != XPATH_RANGE)
		return 0;
	if(range1->user != range2->user)
		return 0;
	if(range1->index != range2->index)
		return 0;
	if(range1->user2 != range2->user2)
		return 0;
	if(range1->index2 != range2->index2)
		return 0;
	return 1;
}

/**
 * xmlXPtrNewRange:
 * @start:  the starting node
 * @startindex:  the start index
 * @end:  the ending point
 * @endindex:  the ending index
 *
 * Create a new xmlXPathObjectPtr of type range
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewRange(xmlNode * start, int startindex, xmlNode * end, int endindex)
{
	xmlXPathObject * ret = 0;
	if(start && end && startindex >= 0 && endindex >= 0) {
		ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
		if(!ret) {
			xmlXPtrErrMemory("allocating range");
		}
		else {
			memzero(ret, sizeof(xmlXPathObject));
			ret->type = XPATH_RANGE;
			ret->user = start;
			ret->index = startindex;
			ret->user2 = end;
			ret->index2 = endindex;
			xmlXPtrRangeCheckOrder(ret);
		}
	}
	return ret;
}
/**
 * xmlXPtrNewRangePoints:
 * @start:  the starting point
 * @end:  the ending point
 *
 * Create a new xmlXPathObjectPtr of type range using 2 Points
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewRangePoints(xmlXPathObjectPtr start, xmlXPathObjectPtr end)
{
	xmlXPathObject * ret = 0;
	if(start && end && start->type == XPATH_POINT && end->type == XPATH_POINT) {
		ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
		if(!ret) {
			xmlXPtrErrMemory("allocating range");
		}
		else {
			memzero(ret, sizeof(xmlXPathObject));
			ret->type = XPATH_RANGE;
			ret->user = start->user;
			ret->index = start->index;
			ret->user2 = end->user;
			ret->index2 = end->index;
			xmlXPtrRangeCheckOrder(ret);
		}
	}
	return ret;
}

/**
 * xmlXPtrNewRangePointNode:
 * @start:  the starting point
 * @end:  the ending node
 *
 * Create a new xmlXPathObjectPtr of type range from a point to a node
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewRangePointNode(xmlXPathObjectPtr start, xmlNode * end)
{
	xmlXPathObject * ret = 0;
	if(start && end && start->type == XPATH_POINT) {
		ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
		if(!ret) {
			xmlXPtrErrMemory("allocating range");
		}
		else {
			memzero(ret, sizeof(xmlXPathObject));
			ret->type = XPATH_RANGE;
			ret->user = start->user;
			ret->index = start->index;
			ret->user2 = end;
			ret->index2 = -1;
			xmlXPtrRangeCheckOrder(ret);
		}
	}
	return ret;
}
/**
 * xmlXPtrNewRangeNodePoint:
 * @start:  the starting node
 * @end:  the ending point
 *
 * Create a new xmlXPathObjectPtr of type range from a node to a point
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewRangeNodePoint(xmlNode * start, xmlXPathObjectPtr end)
{
	xmlXPathObject * ret = 0;
	if(start && end && start->type == XPATH_POINT && end->type == XPATH_POINT) {
		ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
		if(!ret) {
			xmlXPtrErrMemory("allocating range");
		}
		else {
			memzero(ret, sizeof(xmlXPathObject));
			ret->type = XPATH_RANGE;
			ret->user = start;
			ret->index = -1;
			ret->user2 = end->user;
			ret->index2 = end->index;
			xmlXPtrRangeCheckOrder(ret);
		}
	}
	return ret;
}
/**
 * xmlXPtrNewRangeNodes:
 * @start:  the starting node
 * @end:  the ending node
 *
 * Create a new xmlXPathObjectPtr of type range using 2 nodes
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewRangeNodes(xmlNode * start, xmlNode * end)
{
	xmlXPathObject * ret = 0;
	if(start && end) {
		ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
		if(!ret) {
			xmlXPtrErrMemory("allocating range");
		}
		else {
			memzero(ret, sizeof(xmlXPathObject));
			ret->type = XPATH_RANGE;
			ret->user = start;
			ret->index = -1;
			ret->user2 = end;
			ret->index2 = -1;
			xmlXPtrRangeCheckOrder(ret);
		}
	}
	return ret;
}
/**
 * xmlXPtrNewCollapsedRange:
 * @start:  the starting and ending node
 *
 * Create a new xmlXPathObjectPtr of type range using a single nodes
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewCollapsedRange(xmlNode * start)
{
	xmlXPathObject * ret = 0;
	if(start) {
		ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
		if(!ret) {
			xmlXPtrErrMemory("allocating range");
		}
		else {
			memzero(ret, sizeof(xmlXPathObject));
			ret->type = XPATH_RANGE;
			ret->user = start;
			ret->index = -1;
			ret->user2 = NULL;
			ret->index2 = -1;
		}
	}
	return ret;
}

/**
 * xmlXPtrNewRangeNodeObject:
 * @start:  the starting node
 * @end:  the ending object
 *
 * Create a new xmlXPathObjectPtr of type range from a not to an object
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewRangeNodeObject(xmlNode * start, xmlXPathObjectPtr end)
{
	xmlXPathObject * ret = 0;
	if(start && end) {
		switch(end->type) {
			case XPATH_POINT:
			case XPATH_RANGE:
				break;
			case XPATH_NODESET:
				/*
				* Empty set ...
				*/
				if(end->nodesetval->nodeNr <= 0)
					return 0;
				break;
			default:
				/* @todo */
				return 0;
		}
		ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
		if(!ret) {
			xmlXPtrErrMemory("allocating range");
		}
		else {
			memzero(ret, sizeof(xmlXPathObject));
			ret->type = XPATH_RANGE;
			ret->user = start;
			ret->index = -1;
			switch(end->type) {
				case XPATH_POINT:
					ret->user2 = end->user;
					ret->index2 = end->index;
					break;
				case XPATH_RANGE:
					ret->user2 = end->user2;
					ret->index2 = end->index2;
					break;
				case XPATH_NODESET: {
					ret->user2 = end->nodesetval->PP_NodeTab[end->nodesetval->nodeNr - 1];
					ret->index2 = -1;
					break;
				}
				default:
					STRANGE
					return 0;
			}
			xmlXPtrRangeCheckOrder(ret);
		}
	}
	return ret;
}

#define XML_RANGESET_DEFAULT    10
/**
 * xmlXPtrLocationSetCreate:
 * @val:  an initial xmlXPathObjectPtr, or NULL
 *
 * Create a new xmlLocationSetPtr of type double and of value @val
 *
 * Returns the newly created object.
 */
xmlLocationSet * xmlXPtrLocationSetCreate(xmlXPathObject * val)
{
	xmlLocationSet * ret = (xmlLocationSet *)SAlloc::M(sizeof(xmlLocationSet));
	if(!ret) {
		xmlXPtrErrMemory("allocating locationset");
	}
	else {
		memzero(ret, sizeof(xmlLocationSet));
		if(val) {
			ret->locTab = (xmlXPathObjectPtr*)SAlloc::M(XML_RANGESET_DEFAULT * sizeof(xmlXPathObject *));
			if(ret->locTab == NULL) {
				xmlXPtrErrMemory("allocating locationset");
				SAlloc::F(ret);
				return 0;
			}
			memzero(ret->locTab, XML_RANGESET_DEFAULT * sizeof(xmlXPathObject *));
			ret->locMax = XML_RANGESET_DEFAULT;
			ret->locTab[ret->locNr++] = val;
		}
	}
	return ret;
}

/**
 * xmlXPtrLocationSetAdd:
 * @cur:  the initial range set
 * @val:  a new xmlXPathObjectPtr
 *
 * add a new xmlXPathObjectPtr to an existing LocationSet
 * If the location already exist in the set @val is freed.
 */
void xmlXPtrLocationSetAdd(xmlLocationSet * cur, xmlXPathObjectPtr val)
{
	if(cur && val) {
		/*
		* check against doublons
		*/
		int i;
		for(i = 0; i < cur->locNr; i++) {
			if(xmlXPtrRangesEqual(cur->locTab[i], val)) {
				xmlXPathFreeObject(val);
				return;
			}
		}
		/*
		* grow the locTab if needed
		*/
		if(cur->locMax == 0) {
			cur->locTab = (xmlXPathObjectPtr*)SAlloc::M(XML_RANGESET_DEFAULT * sizeof(xmlXPathObject *));
			if(cur->locTab == NULL) {
				xmlXPtrErrMemory("adding location to set");
				return;
			}
			memzero(cur->locTab, XML_RANGESET_DEFAULT * sizeof(xmlXPathObject *));
			cur->locMax = XML_RANGESET_DEFAULT;
		}
		else if(cur->locNr == cur->locMax) {
			xmlXPathObjectPtr * temp;
			cur->locMax *= 2;
			temp = (xmlXPathObjectPtr*)SAlloc::R(cur->locTab, cur->locMax * sizeof(xmlXPathObject *));
			if(temp == NULL) {
				xmlXPtrErrMemory("adding location to set");
				return;
			}
			cur->locTab = temp;
		}
		cur->locTab[cur->locNr++] = val;
	}
}

/**
 * xmlXPtrLocationSetMerge:
 * @val1:  the first LocationSet
 * @val2:  the second LocationSet
 *
 * Merges two rangesets, all ranges from @val2 are added to @val1
 *
 * Returns val1 once extended or NULL in case of error.
 */
xmlLocationSet * xmlXPtrLocationSetMerge(xmlLocationSet * val1, xmlLocationSet * val2)
{
	if(val1 == NULL) return 0;
	if(val2 == NULL) return val1;
	/*
	 * !!!!! this can be optimized a lot, knowing that both
	 *  val1 and val2 already have unicity of their values.
	 */
	for(int i = 0; i < val2->locNr; i++)
		xmlXPtrLocationSetAdd(val1, val2->locTab[i]);
	return val1;
}
/**
 * xmlXPtrLocationSetDel:
 * @cur:  the initial range set
 * @val:  an xmlXPathObjectPtr
 *
 * Removes an xmlXPathObjectPtr from an existing LocationSet
 */
void xmlXPtrLocationSetDel(xmlLocationSet * cur, xmlXPathObjectPtr val)
{
	if(cur && val) {
		/*
		* check against doublons
		*/
		int i;
		for(i = 0; i < cur->locNr; i++)
			if(cur->locTab[i] == val) break;
		if(i >= cur->locNr) {
#ifdef DEBUG
			xmlGenericError(0, "xmlXPtrLocationSetDel: Range wasn't found in RangeList\n");
#endif
			return;
		}
		cur->locNr--;
		for(; i < cur->locNr; i++)
			cur->locTab[i] = cur->locTab[i + 1];
		cur->locTab[cur->locNr] = NULL;
	}
}
/**
 * xmlXPtrLocationSetRemove:
 * @cur:  the initial range set
 * @val:  the index to remove
 *
 * Removes an entry from an existing LocationSet list.
 */
void xmlXPtrLocationSetRemove(xmlLocationSet * cur, int val)
{
	if(cur) {
		if(val < cur->locNr) {
			cur->locNr--;
			for(; val < cur->locNr; val++)
				cur->locTab[val] = cur->locTab[val + 1];
			cur->locTab[cur->locNr] = NULL;
		}
	}
}
/**
 * xmlXPtrFreeLocationSet:
 * @obj:  the xmlLocationSetPtr to free
 *
 * Free the LocationSet compound (not the actual ranges !).
 */
void xmlXPtrFreeLocationSet(xmlLocationSet * obj)
{
	if(obj) {
		if(obj->locTab) {
			for(int i = 0; i < obj->locNr; i++) {
				xmlXPathFreeObject(obj->locTab[i]);
			}
			SAlloc::F(obj->locTab);
		}
		SAlloc::F(obj);
	}
}

/**
 * xmlXPtrNewLocationSetNodes:
 * @start:  the start NodePtr value
 * @end:  the end NodePtr value or NULL
 *
 * Create a new xmlXPathObjectPtr of type LocationSet and initialize
 * it with the single range made of the two nodes @start and @end
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewLocationSetNodes(xmlNode * start, xmlNode * end)
{
	xmlXPathObject * ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
	if(!ret) {
		xmlXPtrErrMemory("allocating locationset");
	}
	else {
		memzero(ret, sizeof(xmlXPathObject));
		ret->type = XPATH_LOCATIONSET;
		ret->user = end ? xmlXPtrLocationSetCreate(xmlXPtrNewRangeNodes(start, end)) : xmlXPtrLocationSetCreate(xmlXPtrNewCollapsedRange(start));
	}
	return ret;
}
/**
 * xmlXPtrNewLocationSetNodeSet:
 * @set:  a node set
 *
 * Create a new xmlXPathObjectPtr of type LocationSet and initialize
 * it with all the nodes from @set
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrNewLocationSetNodeSet(xmlNodeSet * set)
{
	xmlXPathObject * ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
	if(!ret) {
		xmlXPtrErrMemory("allocating locationset");
	}
	else {
		memzero(ret, sizeof(xmlXPathObject));
		ret->type = XPATH_LOCATIONSET;
		if(set) {
			xmlLocationSet * newset = xmlXPtrLocationSetCreate(NULL);
			if(newset) {
				for(int i = 0; i < set->nodeNr; i++)
					xmlXPtrLocationSetAdd(newset, xmlXPtrNewCollapsedRange(set->PP_NodeTab[i]));
				ret->user = newset;
			}
		}
	}
	return ret;
}

/**
 * xmlXPtrWrapLocationSet:
 * @val:  the LocationSet value
 *
 * Wrap the LocationSet @val in a new xmlXPathObjectPtr
 *
 * Returns the newly created object.
 */
xmlXPathObjectPtr xmlXPtrWrapLocationSet(xmlLocationSet * val)
{
	xmlXPathObject * ret = static_cast<xmlXPathObject *>(SAlloc::M(sizeof(xmlXPathObject)));
	if(!ret)
		xmlXPtrErrMemory("allocating locationset");
	else {
		memzero(ret, sizeof(xmlXPathObject));
		ret->type = XPATH_LOCATIONSET;
		ret->user = (void *)val;
	}
	return ret;
}

/************************************************************************
*									*
*			The parser					*
*									*
************************************************************************/

static void xmlXPtrEvalChildSeq(xmlXPathParserContextPtr ctxt, xmlChar * name);
/*
 * Macros for accessing the content. Those should be used only by the parser, and not exported.
 *
 * Dirty macros, i.e. one need to make assumption on the context to use them
 *
 * CUR_PTR return the current pointer to the xmlChar to be parsed.
 * CUR     returns the current xmlChar value, i.e. a 8 bit value
 *      in ISO-Latin or UTF-8.
 *      This should be used internally by the parser
 *      only to compare to ASCII values otherwise it would break when
 *      running with UTF-8 encoding.
 * NXT(n)  returns the n'th next xmlChar. Same as CUR is should be used only
 *      to compare on ASCII based substring.
 * SKIP(n) Skip n xmlChar, and must also be used only to skip ASCII defined
 *      strings within the parser.
 * CURRENT Returns the current char value, with the full decoding of
 *      UTF-8 if we are using this mode. It returns an int.
 * NEXT    Skip to the next character, this does the proper decoding
 *      in UTF-8 mode. It also pop-up unfinished entities on the fly.
 *      It returns the pointer to the current xmlChar.
 */
#define CUR (*ctxt->cur)
#define SKIP(val) ctxt->cur += (val)
#define NXT(val) ctxt->cur[(val)]
#define CUR_PTR ctxt->cur
#define SKIP_BLANKS while(IS_BLANK_CH(*(ctxt->cur))) NEXT
#define CURRENT (*ctxt->cur)
#define NEXT ((*ctxt->cur) ?  ctxt->cur++ : ctxt->cur)
/*
 * xmlXPtrGetChildNo:
 * @ctxt:  the XPointer Parser context
 * @index:  the child number
 *
 * Move the current node of the nodeset on the stack to the
 * given child if found
 */
static void xmlXPtrGetChildNo(xmlXPathParserContextPtr ctxt, int indx)
{
	xmlNode * cur = NULL;
	xmlXPathObject * obj;
	xmlNodeSet * oldset;
	CHECK_TYPE(XPATH_NODESET);
	obj = valuePop(ctxt);
	oldset = obj->nodesetval;
	if((indx <= 0) || (oldset == NULL) || (oldset->nodeNr != 1)) {
		xmlXPathFreeObject(obj);
		valuePush(ctxt, xmlXPathNewNodeSet(NULL));
		return;
	}
	cur = xmlXPtrGetNthChild(oldset->PP_NodeTab[0], indx);
	if(!cur) {
		xmlXPathFreeObject(obj);
		valuePush(ctxt, xmlXPathNewNodeSet(NULL));
		return;
	}
	oldset->PP_NodeTab[0] = cur;
	valuePush(ctxt, obj);
}

/**
 * xmlXPtrEvalXPtrPart:
 * @ctxt:  the XPointer Parser context
 * @name:  the preparsed Scheme for the XPtrPart
 *
 * XPtrPart ::= 'xpointer' '(' XPtrExpr ')'
 *       | Scheme '(' SchemeSpecificExpr ')'
 *
 * Scheme   ::=  NCName - 'xpointer' [VC: Non-XPointer schemes]
 *
 * SchemeSpecificExpr ::= StringWithBalancedParens
 *
 * StringWithBalancedParens ::=
 *         [^()]* ('(' StringWithBalancedParens ')' [^()]*)*
 *         [VC: Parenthesis escaping]
 *
 * XPtrExpr ::= Expr [VC: Parenthesis escaping]
 *
 * VC: Parenthesis escaping:
 * The end of an XPointer part is signaled by the right parenthesis ")"
 * character that is balanced with the left parenthesis "(" character
 * that began the part. Any unbalanced parenthesis character inside the
 * expression, even within literals, must be escaped with a circumflex (^)
 * character preceding it. If the expression contains any literal
 * occurrences of the circumflex, each must be escaped with an additional
 * circumflex (that is, ^^). If the unescaped parentheses in the expression
 * are not balanced, a syntax error results.
 *
 * Parse and evaluate an XPtrPart. Basically it generates the unescaped
 * string and if the scheme is 'xpointer' it will call the XPath interpreter.
 *
 * @todo there is no new scheme registration mechanism
 */

static void xmlXPtrEvalXPtrPart(xmlXPathParserContextPtr ctxt, xmlChar * name)
{
	xmlChar * buffer, * cur;
	int len;
	int level;
	if(!name)
		name = xmlXPathParseName(ctxt);
	if(!name)
		XP_ERROR(XPATH_EXPR_ERROR);
	if(CUR != '(')
		XP_ERROR(XPATH_EXPR_ERROR);
	NEXT;
	level = 1;
	len = sstrlen(ctxt->cur);
	len++;
	buffer = static_cast<xmlChar *>(SAlloc::M(len * sizeof(xmlChar)));
	if(!buffer) {
		xmlXPtrErrMemory("allocating buffer");
		return;
	}
	cur = buffer;
	while(CUR != 0) {
		if(CUR == ')') {
			level--;
			if(level == 0) {
				NEXT;
				break;
			}
		}
		else if(CUR == '(') {
			level++;
		}
		else if(CUR == '^') {
			if((NXT(1) == ')') || (NXT(1) == '(') || (NXT(1) == '^')) {
				NEXT;
			}
		}
		*cur++ = CUR;
		NEXT;
	}
	*cur = 0;
	if((level != 0) && (CUR == 0)) {
		SAlloc::F(buffer);
		XP_ERROR(XPTR_SYNTAX_ERROR);
	}
	if(sstreq(name, (xmlChar *)"xpointer")) {
		const xmlChar * left = CUR_PTR;
		CUR_PTR = buffer;
		/*
		 * To evaluate an xpointer scheme element (4.3) we need:
		 * context initialized to the root
		 * context position initalized to 1
		 * context size initialized to 1
		 */
		ctxt->context->P_Node = (xmlNode *)ctxt->context->doc;
		ctxt->context->proximityPosition = 1;
		ctxt->context->contextSize = 1;
		xmlXPathEvalExpr(ctxt);
		CUR_PTR = left;
	}
	else if(sstreq(name, (xmlChar *)"element")) {
		const xmlChar * left = CUR_PTR;
		xmlChar * name2;
		CUR_PTR = buffer;
		if(buffer[0] == '/') {
			xmlXPathRoot(ctxt);
			xmlXPtrEvalChildSeq(ctxt, 0);
		}
		else {
			name2 = xmlXPathParseName(ctxt);
			if(name2 == NULL) {
				CUR_PTR = left;
				SAlloc::F(buffer);
				XP_ERROR(XPATH_EXPR_ERROR);
			}
			xmlXPtrEvalChildSeq(ctxt, name2);
		}
		CUR_PTR = left;
#ifdef XPTR_XMLNS_SCHEME
	}
	else if(sstreq(name, (xmlChar *)"xmlns")) {
		const xmlChar * left = CUR_PTR;
		xmlChar * prefix;
		xmlChar * URI;
		xmlURI * value;
		CUR_PTR = buffer;
		prefix = xmlXPathParseNCName(ctxt);
		if(prefix == NULL) {
			SAlloc::F(buffer);
			SAlloc::F(name);
			XP_ERROR(XPTR_SYNTAX_ERROR);
		}
		SKIP_BLANKS;
		if(CUR != '=') {
			SAlloc::F(prefix);
			SAlloc::F(buffer);
			SAlloc::F(name);
			XP_ERROR(XPTR_SYNTAX_ERROR);
		}
		NEXT;
		SKIP_BLANKS;
		/* @@ check escaping in the XPointer WD */
		value = xmlParseURI((const char *)ctxt->cur);
		if(!value) {
			SAlloc::F(prefix);
			SAlloc::F(buffer);
			SAlloc::F(name);
			XP_ERROR(XPTR_SYNTAX_ERROR);
		}
		URI = xmlSaveUri(value);
		xmlFreeURI(value);
		if(URI == NULL) {
			SAlloc::F(prefix);
			SAlloc::F(buffer);
			SAlloc::F(name);
			XP_ERROR(XPATH_MEMORY_ERROR);
		}
		xmlXPathRegisterNs(ctxt->context, prefix, URI);
		CUR_PTR = left;
		SAlloc::F(URI);
		SAlloc::F(prefix);
#endif /* XPTR_XMLNS_SCHEME */
	}
	else {
		xmlXPtrErr(ctxt, XML_XPTR_UNKNOWN_SCHEME, "unsupported scheme '%s'\n", name);
	}
	SAlloc::F(buffer);
	SAlloc::F(name);
}

/**
 * xmlXPtrEvalFullXPtr:
 * @ctxt:  the XPointer Parser context
 * @name:  the preparsed Scheme for the first XPtrPart
 *
 * FullXPtr ::= XPtrPart (S? XPtrPart)*
 *
 * As the specs says:
 * -----------
 * When multiple XPtrParts are provided, they must be evaluated in
 * left-to-right order. If evaluation of one part fails, the nexti
 * is evaluated. The following conditions cause XPointer part failure:
 *
 * - An unknown scheme
 * - A scheme that does not locate any sub-resource present in the resource
 * - A scheme that is not applicable to the media type of the resource
 *
 * The XPointer application must consume a failed XPointer part and
 * attempt to evaluate the next one, if any. The result of the first
 * XPointer part whose evaluation succeeds is taken to be the fragment
 * located by the XPointer as a whole. If all the parts fail, the result
 * for the XPointer as a whole is a sub-resource error.
 * -----------
 *
 * Parse and evaluate a Full XPtr i.e. possibly a cascade of XPath based
 * expressions or other schemes.
 */
static void xmlXPtrEvalFullXPtr(xmlXPathParserContextPtr ctxt, xmlChar * name)
{
	SETIFZ(name, xmlXPathParseName(ctxt));
	if(!name)
		XP_ERROR(XPATH_EXPR_ERROR);
	while(name) {
		ctxt->error = XPATH_EXPRESSION_OK;
		xmlXPtrEvalXPtrPart(ctxt, name);
		/* in case of syntax error, break here */
		if((ctxt->error != XPATH_EXPRESSION_OK) && (ctxt->error != XML_XPTR_UNKNOWN_SCHEME))
			return;
		/*
		 * If the returned value is a non-empty nodeset
		 * or location set, return here.
		 */
		if(ctxt->value) {
			xmlXPathObject * obj = ctxt->value;
			switch(obj->type) {
				case XPATH_LOCATIONSET: {
				    xmlLocationSet * loc = (xmlLocationSet *)ctxt->value->user;
				    if(loc && (loc->locNr > 0))
					    return;
				    break;
			    }
				case XPATH_NODESET: {
				    xmlNodeSet * loc = ctxt->value->nodesetval;
				    if(loc && (loc->nodeNr > 0))
					    return;
				    break;
			    }
				default:
				    break;
			}
			/*
			 * Evaluating to improper values is equivalent to
			 * a sub-resource error, clean-up the stack
			 */
			do {
				obj = valuePop(ctxt);
				xmlXPathFreeObject(obj);
			} while(obj);
		}
		/*
		 * Is there another XPointer part.
		 */
		SKIP_BLANKS;
		name = xmlXPathParseName(ctxt);
	}
}

/**
 * xmlXPtrEvalChildSeq:
 * @ctxt:  the XPointer Parser context
 * @name:  a possible ID name of the child sequence
 *
 *  ChildSeq ::= '/1' ('/' [0-9]*)*
 *        | Name ('/' [0-9]*)+
 *
 * Parse and evaluate a Child Sequence. This routine also handle the
 * case of a Bare Name used to get a document ID.
 */
static void xmlXPtrEvalChildSeq(xmlXPathParserContextPtr ctxt, xmlChar * name)
{
	/*
	 * XPointer don't allow by syntax to address in mutirooted trees
	 * this might prove useful in some cases, warn about it.
	 */
	if(!name && (CUR == '/') && (NXT(1) != '1')) {
		xmlXPtrErr(ctxt, XML_XPTR_CHILDSEQ_START, "warning: ChildSeq not starting by /1\n", 0);
	}
	if(name) {
		valuePush(ctxt, xmlXPathNewString(name));
		SAlloc::F(name);
		xmlXPathIdFunction(ctxt, 1);
		CHECK_ERROR;
	}
	while(CUR == '/') {
		int child = 0;
		NEXT;
		while((CUR >= '0') && (CUR <= '9')) {
			child = child * 10 + (CUR - '0');
			NEXT;
		}
		xmlXPtrGetChildNo(ctxt, child);
	}
}

/**
 * xmlXPtrEvalXPointer:
 * @ctxt:  the XPointer Parser context
 *
 *  XPointer ::= Name
 *        | ChildSeq
 *        | FullXPtr
 *
 * Parse and evaluate an XPointer
 */
static void xmlXPtrEvalXPointer(xmlXPathParserContextPtr ctxt)
{
	if(ctxt->valueTab == NULL) {
		/* Allocate the value stack */
		ctxt->valueTab = (xmlXPathObjectPtr*)SAlloc::M(10 * sizeof(xmlXPathObject *));
		if(ctxt->valueTab == NULL) {
			xmlXPtrErrMemory("allocating evaluation context");
			return;
		}
		ctxt->valueNr = 0;
		ctxt->valueMax = 10;
		ctxt->value = NULL;
		ctxt->valueFrame = 0;
	}
	SKIP_BLANKS;
	if(CUR == '/') {
		xmlXPathRoot(ctxt);
		xmlXPtrEvalChildSeq(ctxt, 0);
	}
	else {
		xmlChar * name = xmlXPathParseName(ctxt);
		if(!name)
			XP_ERROR(XPATH_EXPR_ERROR);
		if(CUR == '(') {
			xmlXPtrEvalFullXPtr(ctxt, name);
			/* Short evaluation */
			return;
		}
		else {
			/* this handle both Bare Names and Child Sequences */
			xmlXPtrEvalChildSeq(ctxt, name);
		}
	}
	SKIP_BLANKS;
	if(CUR != 0)
		XP_ERROR(XPATH_EXPR_ERROR);
}

/************************************************************************
*									*
*			General routines				*
*									*
************************************************************************/

static void xmlXPtrStringRangeFunction(xmlXPathParserContextPtr ctxt, int nargs);
static void xmlXPtrStartPointFunction(xmlXPathParserContextPtr ctxt, int nargs);
static void xmlXPtrEndPointFunction(xmlXPathParserContextPtr ctxt, int nargs);
static void xmlXPtrHereFunction(xmlXPathParserContextPtr ctxt, int nargs);
static void xmlXPtrOriginFunction(xmlXPathParserContextPtr ctxt, int nargs);
static void xmlXPtrRangeInsideFunction(xmlXPathParserContextPtr ctxt, int nargs);
static void xmlXPtrRangeFunction(xmlXPathParserContextPtr ctxt, int nargs);

/**
 * xmlXPtrNewContext:
 * @doc:  the XML document
 * @here:  the node that directly contains the XPointer being evaluated or NULL
 * @origin:  the element from which a user or program initiated traversal of
 *      the link, or NULL.
 *
 * Create a new XPointer context
 *
 * Returns the xmlXPathContext just allocated.
 */
xmlXPathContextPtr xmlXPtrNewContext(xmlDoc * doc, xmlNode * here, xmlNode * origin)
{
	xmlXPathContextPtr ret = xmlXPathNewContext(doc);
	if(ret) {
		ret->xptr = 1;
		ret->here = here;
		ret->origin = origin;
		xmlXPathRegisterFunc(ret, (xmlChar *)"range-to", xmlXPtrRangeToFunction);
		xmlXPathRegisterFunc(ret, (xmlChar *)"range", xmlXPtrRangeFunction);
		xmlXPathRegisterFunc(ret, (xmlChar *)"range-inside", xmlXPtrRangeInsideFunction);
		xmlXPathRegisterFunc(ret, (xmlChar *)"string-range", xmlXPtrStringRangeFunction);
		xmlXPathRegisterFunc(ret, (xmlChar *)"start-point", xmlXPtrStartPointFunction);
		xmlXPathRegisterFunc(ret, (xmlChar *)"end-point", xmlXPtrEndPointFunction);
		xmlXPathRegisterFunc(ret, (xmlChar *)"here", xmlXPtrHereFunction);
		xmlXPathRegisterFunc(ret, (xmlChar *)" origin", xmlXPtrOriginFunction);
	}
	return ret;
}

/**
 * xmlXPtrEval:
 * @str:  the XPointer expression
 * @ctx:  the XPointer context
 *
 * Evaluate the XPath Location Path in the given context.
 *
 * Returns the xmlXPathObjectPtr resulting from the evaluation or NULL.
 *    the caller has to free the object.
 */
xmlXPathObjectPtr xmlXPtrEval(const xmlChar * str, xmlXPathContextPtr ctx)
{
	xmlXPathParserContextPtr ctxt;
	xmlXPathObjectPtr res = NULL, tmp;
	xmlXPathObjectPtr init = NULL;
	int stack = 0;
	xmlXPathInit();
	if(!ctx || (str == NULL))
		return 0;
	ctxt = xmlXPathNewParserContext(str, ctx);
	if(!ctxt)
		return 0;
	ctxt->xptr = 1;
	xmlXPtrEvalXPointer(ctxt);
	if(ctxt->value && (ctxt->value->type != XPATH_NODESET) && (ctxt->value->type != XPATH_LOCATIONSET)) {
		xmlXPtrErr(ctxt, XML_XPTR_EVAL_FAILED, "xmlXPtrEval: evaluation failed to return a node set\n", 0);
	}
	else {
		res = valuePop(ctxt);
	}
	do {
		tmp = valuePop(ctxt);
		if(tmp) {
			if(tmp != init) {
				if(tmp->type == XPATH_NODESET) {
					/*
					 * Evaluation may push a root nodeset which is unused
					 */
					xmlNodeSet * set;
					set = tmp->nodesetval;
					if((set->nodeNr != 1) || (set->PP_NodeTab[0] != (xmlNode *)ctx->doc))
						stack++;
				}
				else
					stack++;
			}
			xmlXPathFreeObject(tmp);
		}
	} while(tmp);
	if(stack != 0) {
		xmlXPtrErr(ctxt, XML_XPTR_EXTRA_OBJECTS, "xmlXPtrEval: object(s) left on the eval stack\n", 0);
	}
	if(ctxt->error != XPATH_EXPRESSION_OK) {
		xmlXPathFreeObject(res);
		res = NULL;
	}
	xmlXPathFreeParserContext(ctxt);
	return res;
}

/**
 * xmlXPtrBuildRangeNodeList:
 * @range:  a range object
 *
 * Build a node list tree copy of the range
 *
 * Returns an xmlNodePtr list or NULL.
 *    the caller has to free the node tree.
 */
static xmlNode * xmlXPtrBuildRangeNodeList(xmlXPathObjectPtr range)
{
	/* pointers to generated nodes */
	xmlNode * list = NULL;
	xmlNode * last = NULL;
	xmlNode * parent = NULL;
	xmlNode * tmp;
	/* pointers to traversal nodes */
	xmlNode * start;
	xmlNode * cur;
	xmlNode * end;
	int index1, index2;
	if(range == NULL)
		return 0;
	if(range->type != XPATH_RANGE)
		return 0;
	start = (xmlNode *)range->user;
	if((start == NULL) || (start->type == XML_NAMESPACE_DECL))
		return 0;
	end = (xmlNode *)range->user2;
	if(end == NULL)
		return (xmlCopyNode(start, 1));
	if(end->type == XML_NAMESPACE_DECL)
		return 0;
	cur = start;
	index1 = range->index;
	index2 = range->index2;
	while(cur) {
		if(cur == end) {
			if(cur->type == XML_TEXT_NODE) {
				const xmlChar * content = cur->content;
				int len;
				if(content == NULL) {
					tmp = xmlNewTextLen(NULL, 0);
				}
				else {
					len = index2;
					if((cur == start) && (index1 > 1)) {
						content += (index1 - 1);
						len -= (index1 - 1);
						index1 = 0;
					}
					else {
						len = index2;
					}
					tmp = xmlNewTextLen(content, len);
				}
				/* single sub text node selection */
				if(list == NULL)
					return tmp;
				/* prune and return full set */
				if(last)
					xmlAddNextSibling(last, tmp);
				else
					xmlAddChild(parent, tmp);
				return (list);
			}
			else {
				tmp = xmlCopyNode(cur, 0);
				if(list == NULL)
					list = tmp;
				else {
					if(last)
						xmlAddNextSibling(last, tmp);
					else
						xmlAddChild(parent, tmp);
				}
				last = NULL;
				parent = tmp;

				if(index2 > 1) {
					end = xmlXPtrGetNthChild(cur, index2 - 1);
					index2 = 0;
				}
				if((cur == start) && (index1 > 1)) {
					cur = xmlXPtrGetNthChild(cur, index1 - 1);
					index1 = 0;
				}
				else {
					cur = cur->children;
				}
				/*
				 * Now gather the remaining nodes from cur to end
				 */
				continue; /* while */
			}
		}
		else if((cur == start) && (list == NULL) /* looks superfluous but ... */) {
			if((cur->type == XML_TEXT_NODE) || (cur->type == XML_CDATA_SECTION_NODE)) {
				const xmlChar * content = cur->content;
				if(content == NULL) {
					tmp = xmlNewTextLen(NULL, 0);
				}
				else {
					if(index1 > 1) {
						content += (index1 - 1);
					}
					tmp = xmlNewText(content);
				}
				last = list = tmp;
			}
			else {
				if((cur == start) && (index1 > 1)) {
					tmp = xmlCopyNode(cur, 0);
					list = tmp;
					parent = tmp;
					last = NULL;
					cur = xmlXPtrGetNthChild(cur, index1 - 1);
					index1 = 0;
					/*
					 * Now gather the remaining nodes from cur to end
					 */
					continue; /* while */
				}
				tmp = xmlCopyNode(cur, 1);
				list = tmp;
				parent = NULL;
				last = tmp;
			}
		}
		else {
			tmp = NULL;
			switch(cur->type) {
				case XML_DTD_NODE:
				case XML_ELEMENT_DECL:
				case XML_ATTRIBUTE_DECL:
				case XML_ENTITY_NODE:
				    /* Do not copy DTD informations */
				    break;
				case XML_ENTITY_DECL:
				    TODO /* handle crossing entities -> stack needed */
				    break;
				case XML_XINCLUDE_START:
				case XML_XINCLUDE_END:
				    /* don't consider it part of the tree content */
				    break;
				case XML_ATTRIBUTE_NODE:
				    /* Humm, should not happen ! */
				    STRANGE
				    break;
				default:
				    tmp = xmlCopyNode(cur, 1);
				    break;
			}
			if(tmp) {
				if((list == NULL) || ((last == NULL) && (parent == NULL))) {
					STRANGE
					return 0;
				}
				if(last)
					xmlAddNextSibling(last, tmp);
				else {
					xmlAddChild(parent, tmp);
					last = tmp;
				}
			}
		}
		/*
		 * Skip to next node in document order
		 */
		if((list == NULL) || ((last == NULL) && (parent == NULL))) {
			STRANGE
			return 0;
		}
		cur = xmlXPtrAdvanceNode(cur, 0);
	}
	return (list);
}

/**
 * xmlXPtrBuildNodeList:
 * @obj:  the XPointer result from the evaluation.
 *
 * Build a node list tree copy of the XPointer result.
 * This will drop Attributes and Namespace declarations.
 *
 * Returns an xmlNodePtr list or NULL.
 *    the caller has to free the node tree.
 */
xmlNode * xmlXPtrBuildNodeList(xmlXPathObject * obj)
{
	xmlNode * list = NULL;
	xmlNode * last = NULL;
	int i;
	if(obj == NULL)
		return 0;
	switch(obj->type) {
		case XPATH_NODESET: {
		    xmlNodeSet * set = obj->nodesetval;
		    if(set == NULL)
			    return 0;
		    for(i = 0; i < set->nodeNr; i++) {
			    if(set->PP_NodeTab[i] == NULL)
				    continue;
			    switch(set->PP_NodeTab[i]->type) {
				    case XML_TEXT_NODE:
				    case XML_CDATA_SECTION_NODE:
				    case XML_ELEMENT_NODE:
				    case XML_ENTITY_REF_NODE:
				    case XML_ENTITY_NODE:
				    case XML_PI_NODE:
				    case XML_COMMENT_NODE:
				    case XML_DOCUMENT_NODE:
				    case XML_HTML_DOCUMENT_NODE:
#ifdef LIBXML_DOCB_ENABLED
				    case XML_DOCB_DOCUMENT_NODE:
#endif
				    case XML_XINCLUDE_START:
				    case XML_XINCLUDE_END:
					break;
				    case XML_ATTRIBUTE_NODE:
				    case XML_NAMESPACE_DECL:
				    case XML_DOCUMENT_TYPE_NODE:
				    case XML_DOCUMENT_FRAG_NODE:
				    case XML_NOTATION_NODE:
				    case XML_DTD_NODE:
				    case XML_ELEMENT_DECL:
				    case XML_ATTRIBUTE_DECL:
				    case XML_ENTITY_DECL:
					continue; /* for */
			    }
			    if(last == NULL)
				    list = last = xmlCopyNode(set->PP_NodeTab[i], 1);
			    else {
				    xmlAddNextSibling(last, xmlCopyNode(set->PP_NodeTab[i], 1));
				    if(last->next)
					    last = last->next;
			    }
		    }
		    break;
	    }
		case XPATH_LOCATIONSET: {
		    xmlLocationSet * set = (xmlLocationSet *)obj->user;
		    if(set == NULL)
			    return 0;
		    for(i = 0; i < set->locNr; i++) {
			    if(last == NULL)
				    list = last = xmlXPtrBuildNodeList(set->locTab[i]);
			    else
				    xmlAddNextSibling(last, xmlXPtrBuildNodeList(set->locTab[i]));
			    if(last) {
				    while(last->next)
					    last = last->next;
			    }
		    }
		    break;
	    }
		case XPATH_RANGE:
		    return (xmlXPtrBuildRangeNodeList(obj));
		case XPATH_POINT:
		    return xmlCopyNode((xmlNode *)obj->user, 0);
		default:
		    break;
	}
	return (list);
}

/************************************************************************
*									*
*			XPointer functions				*
*									*
************************************************************************/
/**
 * xmlXPtrNbLocChildren:
 * @node:  an xmlNodePtr
 *
 * Count the number of location children of @node or the length of the
 * string value in case of text/PI/Comments nodes
 *
 * Returns the number of location children
 */
static int xmlXPtrNbLocChildren(xmlNode * P_Node)
{
	int ret = 0;
	if(!P_Node)
		return -1;
	switch(P_Node->type) {
		case XML_HTML_DOCUMENT_NODE:
		case XML_DOCUMENT_NODE:
		case XML_ELEMENT_NODE:
		    P_Node = P_Node->children;
		    while(P_Node) {
			    if(P_Node->type == XML_ELEMENT_NODE)
				    ret++;
			    P_Node = P_Node->next;
		    }
		    break;
		case XML_ATTRIBUTE_NODE:
		    return -1;

		case XML_PI_NODE:
		case XML_COMMENT_NODE:
		case XML_TEXT_NODE:
		case XML_CDATA_SECTION_NODE:
		case XML_ENTITY_REF_NODE:
		    ret = sstrlen(P_Node->content);
		    break;
		default:
		    return -1;
	}
	return ret;
}

/**
 * xmlXPtrHereFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Function implementing here() operation
 * as described in 5.4.3
 */
static void xmlXPtrHereFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	CHECK_ARITY(0);
	if(ctxt->context->here == NULL)
		XP_ERROR(XPTR_SYNTAX_ERROR);
	valuePush(ctxt, xmlXPtrNewLocationSetNodes(ctxt->context->here, NULL));
}

/**
 * xmlXPtrOriginFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Function implementing origin() operation
 * as described in 5.4.3
 */
static void xmlXPtrOriginFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	CHECK_ARITY(0);
	if(ctxt->context->origin == NULL)
		XP_ERROR(XPTR_SYNTAX_ERROR);
	valuePush(ctxt, xmlXPtrNewLocationSetNodes(ctxt->context->origin, NULL));
}

/**
 * xmlXPtrStartPointFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Function implementing start-point() operation
 * as described in 5.4.3
 * ----------------
 * location-set start-point(location-set)
 *
 * For each location x in the argument location-set, start-point adds a
 * location of type point to the result location-set. That point represents
 * the start point of location x and is determined by the following rules:
 *
 * - If x is of type point, the start point is x.
 * - If x is of type range, the start point is the start point of x.
 * - If x is of type root, element, text, comment, or processing instruction,
 * - the container node of the start point is x and the index is 0.
 * - If x is of type attribute or namespace, the function must signal a
 * syntax error.
 * ----------------
 *
 */
static void xmlXPtrStartPointFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	xmlXPathObjectPtr tmp, obj, point;
	xmlLocationSet * newset = NULL;
	xmlLocationSet * oldset = NULL;
	CHECK_ARITY(1);
	if(!ctxt->value || ((ctxt->value->type != XPATH_LOCATIONSET) && (ctxt->value->type != XPATH_NODESET)))
		XP_ERROR(XPATH_INVALID_TYPE)
		obj = valuePop(ctxt);
	if(obj->type == XPATH_NODESET) {
		/*
		 * First convert to a location set
		 */
		tmp = xmlXPtrNewLocationSetNodeSet(obj->nodesetval);
		xmlXPathFreeObject(obj);
		if(!tmp)
			XP_ERROR(XPATH_MEMORY_ERROR)
			obj = tmp;
	}

	newset = xmlXPtrLocationSetCreate(NULL);
	if(newset == NULL) {
		xmlXPathFreeObject(obj);
		XP_ERROR(XPATH_MEMORY_ERROR);
	}
	oldset = (xmlLocationSet *)obj->user;
	if(oldset) {
		int i;
		for(i = 0; i < oldset->locNr; i++) {
			tmp = oldset->locTab[i];
			if(tmp) {
				point = NULL;
				switch(tmp->type) {
					case XPATH_POINT:
						point = xmlXPtrNewPoint((xmlNode *)tmp->user, tmp->index);
						break;
					case XPATH_RANGE: {
						xmlNode * P_Node = (xmlNode *)tmp->user;
						if(P_Node) {
							if(P_Node->type == XML_ATTRIBUTE_NODE) {
								/* @todo Namespace Nodes ??? */
								xmlXPathFreeObject(obj);
								xmlXPtrFreeLocationSet(newset);
								XP_ERROR(XPTR_SYNTAX_ERROR);
							}
							point = xmlXPtrNewPoint(P_Node, tmp->index);
						}
						break;
					}
					default:
						/*** Should we raise an error ?
						xmlXPathFreeObject(obj);
						xmlXPathFreeObject(newset);
						XP_ERROR(XPATH_INVALID_TYPE)
						***/
						break;
				}
				if(point)
					xmlXPtrLocationSetAdd(newset, point);
			}
		}
	}
	xmlXPathFreeObject(obj);
	valuePush(ctxt, xmlXPtrWrapLocationSet(newset));
}

/**
 * xmlXPtrEndPointFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Function implementing end-point() operation
 * as described in 5.4.3
 * ----------------------------
 * location-set end-point(location-set)
 *
 * For each location x in the argument location-set, end-point adds a
 * location of type point to the result location-set. That point represents
 * the end point of location x and is determined by the following rules:
 *
 * - If x is of type point, the resulting point is x.
 * - If x is of type range, the resulting point is the end point of x.
 * - If x is of type root or element, the container node of the resulting
 * point is x and the index is the number of location children of x.
 * - If x is of type text, comment, or processing instruction, the container
 * node of the resulting point is x and the index is the length of the
 * string-value of x.
 * - If x is of type attribute or namespace, the function must signal a
 * syntax error.
 * ----------------------------
 */
static void xmlXPtrEndPointFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	xmlXPathObjectPtr tmp, obj, point;
	xmlLocationSet * newset = NULL;
	xmlLocationSet * oldset = NULL;
	CHECK_ARITY(1);
	if(!ctxt->value || ((ctxt->value->type != XPATH_LOCATIONSET) && (ctxt->value->type != XPATH_NODESET)))
		XP_ERROR(XPATH_INVALID_TYPE)

		obj = valuePop(ctxt);
	if(obj->type == XPATH_NODESET) {
		/*
		 * First convert to a location set
		 */
		tmp = xmlXPtrNewLocationSetNodeSet(obj->nodesetval);
		xmlXPathFreeObject(obj);
		if(!tmp)
			XP_ERROR(XPATH_MEMORY_ERROR)
			obj = tmp;
	}
	newset = xmlXPtrLocationSetCreate(NULL);
	if(newset == NULL) {
		xmlXPathFreeObject(obj);
		XP_ERROR(XPATH_MEMORY_ERROR);
	}
	oldset = (xmlLocationSet *)obj->user;
	if(oldset) {
		int i;
		for(i = 0; i < oldset->locNr; i++) {
			tmp = oldset->locTab[i];
			if(!tmp)
				continue;
			point = NULL;
			switch(tmp->type) {
				case XPATH_POINT:
				    point = xmlXPtrNewPoint((xmlNode *)tmp->user, tmp->index);
				    break;
				case XPATH_RANGE: {
				    xmlNode * P_Node = (xmlNode *)tmp->user2;
				    if(P_Node) {
					    if(P_Node->type == XML_ATTRIBUTE_NODE) {
						    /* @todo Namespace Nodes ??? */
						    xmlXPathFreeObject(obj);
						    xmlXPtrFreeLocationSet(newset);
						    XP_ERROR(XPTR_SYNTAX_ERROR);
					    }
					    point = xmlXPtrNewPoint(P_Node, tmp->index2);
				    }
				    else if(tmp->user == NULL) {
					    point = xmlXPtrNewPoint(P_Node, xmlXPtrNbLocChildren(P_Node));
				    }
				    break;
			    }
				default:
				    /*** Should we raise an error ?
				       xmlXPathFreeObject(obj);
				       xmlXPathFreeObject(newset);
				       XP_ERROR(XPATH_INVALID_TYPE)
				     ***/
				    break;
			}
			if(point)
				xmlXPtrLocationSetAdd(newset, point);
		}
	}
	xmlXPathFreeObject(obj);
	valuePush(ctxt, xmlXPtrWrapLocationSet(newset));
}
/**
 * xmlXPtrCoveringRange:
 * @ctxt:  the XPointer Parser context
 * @loc:  the location for which the covering range must be computed
 *
 * A covering range is a range that wholly encompasses a location
 * Section 5.3.3. Covering Ranges for All Location Types
 *   http://www.w3.org/TR/xptr#N2267
 *
 * Returns a new location or NULL in case of error
 */
static xmlXPathObjectPtr xmlXPtrCoveringRange(xmlXPathParserContextPtr ctxt, xmlXPathObjectPtr loc)
{
	if(loc == NULL)
		return 0;
	if(!ctxt || (ctxt->context == NULL) || (ctxt->context->doc == NULL))
		return 0;
	switch(loc->type) {
		case XPATH_POINT:
		    return xmlXPtrNewRange((xmlNode *)loc->user, loc->index, (xmlNode *)loc->user, loc->index);
		case XPATH_RANGE:
		    if(loc->user2) {
			    return xmlXPtrNewRange((xmlNode *)loc->user, loc->index, (xmlNode *)loc->user2, loc->index2);
		    }
		    else {
			    xmlNode * P_Node = (xmlNode *)loc->user;
			    if(P_Node == (xmlNode *)ctxt->context->doc) {
				    return xmlXPtrNewRange(P_Node, 0, P_Node, xmlXPtrGetArity(P_Node));
			    }
			    else {
				    switch(P_Node->type) {
					    case XML_ATTRIBUTE_NODE:
						/* !!! our model is slightly different than XPath */
						return xmlXPtrNewRange(P_Node, 0, P_Node, xmlXPtrGetArity(P_Node));
					    case XML_ELEMENT_NODE:
					    case XML_TEXT_NODE:
					    case XML_CDATA_SECTION_NODE:
					    case XML_ENTITY_REF_NODE:
					    case XML_PI_NODE:
					    case XML_COMMENT_NODE:
					    case XML_DOCUMENT_NODE:
					    case XML_NOTATION_NODE:
					    case XML_HTML_DOCUMENT_NODE: {
						int indx = xmlXPtrGetIndex(P_Node);
						P_Node = P_Node->P_ParentNode;
						return (xmlXPtrNewRange(P_Node, indx - 1, P_Node, indx + 1));
					}
					    default:
						return 0;
				    }
			    }
		    }
		default:
		    TODO /* missed one case ??? */
	}
	return 0;
}

/**
 * xmlXPtrRangeFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Function implementing the range() function 5.4.3
 *  location-set range(location-set )
 *
 *  The range function returns ranges covering the locations in
 *  the argument location-set. For each location x in the argument
 *  location-set, a range location representing the covering range of
 *  x is added to the result location-set.
 */
static void xmlXPtrRangeFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	int i;
	xmlXPathObjectPtr set;
	xmlLocationSet * oldset;
	xmlLocationSet * newset;
	CHECK_ARITY(1);
	if(!ctxt->value || ((ctxt->value->type != XPATH_LOCATIONSET) && (ctxt->value->type != XPATH_NODESET)))
		XP_ERROR(XPATH_INVALID_TYPE)
		set = valuePop(ctxt);
	if(set->type == XPATH_NODESET) {
		/*
		 * First convert to a location set
		 */
		xmlXPathObjectPtr tmp = xmlXPtrNewLocationSetNodeSet(set->nodesetval);
		xmlXPathFreeObject(set);
		if(!tmp)
			XP_ERROR(XPATH_MEMORY_ERROR)
			set = tmp;
	}
	oldset = (xmlLocationSet *)set->user;
	/*
	 * The loop is to compute the covering range for each item and add it
	 */
	newset = xmlXPtrLocationSetCreate(NULL);
	if(newset == NULL) {
		xmlXPathFreeObject(set);
		XP_ERROR(XPATH_MEMORY_ERROR);
	}
	for(i = 0; i < oldset->locNr; i++) {
		xmlXPtrLocationSetAdd(newset, xmlXPtrCoveringRange(ctxt, oldset->locTab[i]));
	}
	/*
	 * Save the new value and cleanup
	 */
	valuePush(ctxt, xmlXPtrWrapLocationSet(newset));
	xmlXPathFreeObject(set);
}

/**
 * xmlXPtrInsideRange:
 * @ctxt:  the XPointer Parser context
 * @loc:  the location for which the inside range must be computed
 *
 * A inside range is a range described in the range-inside() description
 *
 * Returns a new location or NULL in case of error
 */
static xmlXPathObjectPtr xmlXPtrInsideRange(xmlXPathParserContextPtr ctxt, xmlXPathObjectPtr loc)
{
	if(loc == NULL)
		return 0;
	if(!ctxt || (ctxt->context == NULL) || (ctxt->context->doc == NULL))
		return 0;
	switch(loc->type) {
		case XPATH_POINT: {
		    xmlNode * P_Node = (xmlNode *)loc->user;
		    switch(P_Node->type) {
			    case XML_PI_NODE:
			    case XML_COMMENT_NODE:
			    case XML_TEXT_NODE:
			    case XML_CDATA_SECTION_NODE: 
					return xmlXPtrNewRange(P_Node, 0, P_Node, sstrlen(P_Node->content));
			    case XML_ATTRIBUTE_NODE:
			    case XML_ELEMENT_NODE:
			    case XML_ENTITY_REF_NODE:
			    case XML_DOCUMENT_NODE:
			    case XML_NOTATION_NODE:
			    case XML_HTML_DOCUMENT_NODE:
					return xmlXPtrNewRange(P_Node, 0, P_Node, xmlXPtrGetArity(P_Node));
			    default:
				break;
		    }
		    return 0;
	    }
		case XPATH_RANGE: {
		    xmlNode * P_Node = (xmlNode *)loc->user;
		    if(loc->user2) {
			    return xmlXPtrNewRange(P_Node, loc->index, (xmlNode *)loc->user2, loc->index2);
		    }
		    else {
			    switch(P_Node->type) {
				    case XML_PI_NODE:
				    case XML_COMMENT_NODE:
				    case XML_TEXT_NODE:
				    case XML_CDATA_SECTION_NODE: 
						return xmlXPtrNewRange(P_Node, 0, P_Node, sstrlen(P_Node->content));
				    case XML_ATTRIBUTE_NODE:
				    case XML_ELEMENT_NODE:
				    case XML_ENTITY_REF_NODE:
				    case XML_DOCUMENT_NODE:
				    case XML_NOTATION_NODE:
				    case XML_HTML_DOCUMENT_NODE:
						return xmlXPtrNewRange(P_Node, 0, P_Node, xmlXPtrGetArity(P_Node));
				    default:
						break;
			    }
			    return 0;
		    }
	    }
		default:
		    TODO /* missed one case ??? */
	}
	return 0;
}
/**
 * xmlXPtrRangeInsideFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Function implementing the range-inside() function 5.4.3
 *  location-set range-inside(location-set )
 *
 *  The range-inside function returns ranges covering the contents of
 *  the locations in the argument location-set. For each location x in
 *  the argument location-set, a range location is added to the result
 *  location-set. If x is a range location, then x is added to the
 *  result location-set. If x is not a range location, then x is used
 *  as the container location of the start and end points of the range
 *  location to be added; the index of the start point of the range is
 *  zero; if the end point is a character point then its index is the
 *  length of the string-value of x, and otherwise is the number of
 *  location children of x.
 *
 */
static void xmlXPtrRangeInsideFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	int i;
	xmlXPathObjectPtr set;
	xmlLocationSet * oldset;
	xmlLocationSet * newset;
	CHECK_ARITY(1);
	if(!ctxt->value || ((ctxt->value->type != XPATH_LOCATIONSET) && (ctxt->value->type != XPATH_NODESET)))
		XP_ERROR(XPATH_INVALID_TYPE)
		set = valuePop(ctxt);
	if(set->type == XPATH_NODESET) {
		/*
		 * First convert to a location set
		 */
		xmlXPathObjectPtr tmp = xmlXPtrNewLocationSetNodeSet(set->nodesetval);
		xmlXPathFreeObject(set);
		if(!tmp)
			XP_ERROR(XPATH_MEMORY_ERROR)
			set = tmp;
	}
	oldset = (xmlLocationSet *)set->user;
	/*
	 * The loop is to compute the covering range for each item and add it
	 */
	newset = xmlXPtrLocationSetCreate(NULL);
	if(newset == NULL) {
		xmlXPathFreeObject(set);
		XP_ERROR(XPATH_MEMORY_ERROR);
	}
	for(i = 0; i < oldset->locNr; i++) {
		xmlXPtrLocationSetAdd(newset, xmlXPtrInsideRange(ctxt, oldset->locTab[i]));
	}
	/*
	 * Save the new value and cleanup
	 */
	valuePush(ctxt, xmlXPtrWrapLocationSet(newset));
	xmlXPathFreeObject(set);
}
/**
 * xmlXPtrRangeToFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Implement the range-to() XPointer function
 */
void xmlXPtrRangeToFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	xmlXPathObjectPtr range;
	const xmlChar * cur;
	xmlXPathObjectPtr res, obj;
	xmlXPathObjectPtr tmp;
	xmlLocationSet * newset = NULL;
	xmlNodeSet * oldset;
	int i;
	if(!ctxt)
		return;
	CHECK_ARITY(1);
	/*
	 * Save the expression pointer since we will have to evaluate
	 * it multiple times. Initialize the new set.
	 */
	CHECK_TYPE(XPATH_NODESET);
	obj = valuePop(ctxt);
	oldset = obj->nodesetval;
	ctxt->context->P_Node = NULL;
	cur = ctxt->cur;
	newset = xmlXPtrLocationSetCreate(NULL);
	for(i = 0; i < oldset->nodeNr; i++) {
		ctxt->cur = cur;
		/*
		 * Run the evaluation with a node list made of a single item
		 * in the nodeset.
		 */
		ctxt->context->P_Node = oldset->PP_NodeTab[i];
		tmp = xmlXPathNewNodeSet(ctxt->context->P_Node);
		valuePush(ctxt, tmp);

		xmlXPathEvalExpr(ctxt);
		CHECK_ERROR;

		/*
		 * The result of the evaluation need to be tested to
		 * decided whether the filter succeeded or not
		 */
		res = valuePop(ctxt);
		range = xmlXPtrNewRangeNodeObject(oldset->PP_NodeTab[i], res);
		if(range) {
			xmlXPtrLocationSetAdd(newset, range);
		}
		/*
		 * Cleanup
		 */
		xmlXPathFreeObject(res);
		if(ctxt->value == tmp) {
			res = valuePop(ctxt);
			xmlXPathFreeObject(res);
		}
		ctxt->context->P_Node = NULL;
	}
	/*
	 * The result is used as the new evaluation set.
	 */
	xmlXPathFreeObject(obj);
	ctxt->context->P_Node = NULL;
	ctxt->context->contextSize = -1;
	ctxt->context->proximityPosition = -1;
	valuePush(ctxt, xmlXPtrWrapLocationSet(newset));
}

/**
 * xmlXPtrAdvanceNode:
 * @cur:  the node
 * @level: incremented/decremented to show level in tree
 *
 * Advance to the next element or text node in document order
 * @todo add a stack for entering/exiting entities
 *
 * Returns -1 in case of failure, 0 otherwise
 */
xmlNode * xmlXPtrAdvanceNode(xmlNode * cur, int * level)
{
next:
	if(!cur || (cur->type == XML_NAMESPACE_DECL))
		return 0;
	if(cur->children) {
		cur = cur->children;
		if(level)
			(*level)++;
		goto found;
	}
skip:           /* This label should only be needed if something is wrong! */
	if(cur->next) {
		cur = cur->next;
		goto found;
	}
	do {
		cur = cur->P_ParentNode;
		if(level)
			(*level)--;
		if(!cur) 
			return 0;
		if(cur->next) {
			cur = cur->next;
			goto found;
		}
	} while(cur);
found:
	if(!oneof5(cur->type, XML_ELEMENT_NODE, XML_TEXT_NODE, XML_DOCUMENT_NODE, XML_HTML_DOCUMENT_NODE, XML_CDATA_SECTION_NODE)) {
		if(cur->type == XML_ENTITY_REF_NODE) {  /* Shouldn't happen */
			TODO
			goto skip;
		}
		goto next;
	}
	return cur;
}
/**
 * xmlXPtrAdvanceChar:
 * @node:  the node
 * @indx:  the indx
 * @bytes:  the number of bytes
 *
 * Advance a point of the associated number of bytes (not UTF8 chars)
 *
 * Returns -1 in case of failure, 0 otherwise
 */
static int xmlXPtrAdvanceChar(xmlNode ** ppNode, int * indx, int bytes)
{
	int len;
	if(ppNode && indx) {
		xmlNode * cur = *ppNode;
		if(cur && (cur->type != XML_NAMESPACE_DECL)) {
			int pos = *indx;
			while(bytes >= 0) {
				//
				// First position to the beginning of the first text node
				// corresponding to this point
				//
				while(cur && oneof3(cur->type, XML_ELEMENT_NODE, XML_DOCUMENT_NODE, XML_HTML_DOCUMENT_NODE)) {
					if(pos > 0) {
						cur = xmlXPtrGetNthChild(cur, pos);
						pos = 0;
					}
					else {
						cur = xmlXPtrAdvanceNode(cur, 0);
						pos = 0;
					}
				}
				if(!cur) {
					*ppNode = NULL;
					*indx = 0;
					return -1;
				}
				else {
					//
					// if there is no move needed return the current value.
					//
					SETIFZ(pos, 1);
					if(bytes == 0) {
						*ppNode = cur;
						*indx = pos;
						return 0;
					}
					else {
						//
						// We should have a text (or cdata) node ...
						//
						len = 0;
						if((cur->type != XML_ELEMENT_NODE) && cur->content) {
							len = sstrlen(cur->content);
						}
						if(pos > len) {
							// Strange, the indx in the text node is greater than it's len
							STRANGE
								pos = len;
						}
						if((pos + bytes) >= len) {
							bytes -= (len - pos);
							cur = xmlXPtrAdvanceNode(cur, 0);
							pos = 0;
						}
						else if((pos + bytes) < len) {
							pos += bytes;
							*ppNode = cur;
							*indx = pos;
							return 0;
						}
					}
				}
			}
		}
	}
	return -1;
}

/**
 * xmlXPtrMatchString:
 * @string:  the string to search
 * @start:  the start textnode
 * @startindex:  the start index
 * @end:  the end textnode IN/OUT
 * @endindex:  the end index IN/OUT
 *
 * Check whether the document contains @string at the position
 * (@start, @startindex) and limited by the (@end, @endindex) point
 *
 * Returns -1 in case of failure, 0 if not found, 1 if found in which case
 *       (@start, @startindex) will indicate the position of the beginning
 *       of the range and (@end, @endindex) will indicate the end
 *       of the range
 */
static int xmlXPtrMatchString(const xmlChar * string, xmlNode * start, int startindex, xmlNode ** end, int * endindex)
{
	xmlNode * cur;
	int pos; /* 0 based */
	int len; /* in bytes */
	int stringlen; /* in bytes */
	int match;
	if(string == NULL)
		return -1;
	if((start == NULL) || (start->type == XML_NAMESPACE_DECL))
		return -1;
	if((end == NULL) || (*end == NULL) || ((*end)->type == XML_NAMESPACE_DECL) || (endindex == NULL))
		return -1;
	cur = start;
	pos = startindex - 1;
	stringlen = sstrlen(string);
	while(stringlen > 0) {
		if((cur == *end) && (pos + stringlen > *endindex))
			return 0;
		if((cur->type != XML_ELEMENT_NODE) && cur->content) {
			len = sstrlen(cur->content);
			if(len >= pos + stringlen) {
				match = (!xmlStrncmp(&cur->content[pos], string, stringlen));
				if(match) {
#ifdef DEBUG_RANGES
					xmlGenericError(0, "found range %d bytes at index %d of ->", stringlen, pos + 1);
					xmlDebugDumpString(stdout, cur->content);
					xmlGenericError(0, "\n");
#endif
					*end = cur;
					*endindex = pos + stringlen;
					return 1;
				}
				else {
					return 0;
				}
			}
			else {
				int sub = len - pos;
				match = (!xmlStrncmp(&cur->content[pos], string, sub));
				if(match) {
#ifdef DEBUG_RANGES
					xmlGenericError(0, "found subrange %d bytes at index %d of ->", sub, pos + 1);
					xmlDebugDumpString(stdout, cur->content);
					xmlGenericError(0, "\n");
#endif
					string = &string[sub];
					stringlen -= sub;
				}
				else {
					return 0;
				}
			}
		}
		cur = xmlXPtrAdvanceNode(cur, 0);
		if(!cur)
			return 0;
		pos = 0;
	}
	return 1;
}

/**
 * xmlXPtrSearchString:
 * @string:  the string to search
 * @start:  the start textnode IN/OUT
 * @startindex:  the start index IN/OUT
 * @end:  the end textnode
 * @endindex:  the end index
 *
 * Search the next occurrence of @string within the document content
 * until the (@end, @endindex) point is reached
 *
 * Returns -1 in case of failure, 0 if not found, 1 if found in which case
 *       (@start, @startindex) will indicate the position of the beginning
 *       of the range and (@end, @endindex) will indicate the end
 *       of the range
 */
static int xmlXPtrSearchString(const xmlChar * string, xmlNode ** start, int * startindex, xmlNode ** end, int * endindex)
{
	xmlNode * cur;
	const xmlChar * str;
	int pos; /* 0 based */
	int len; /* in bytes */
	xmlChar first;
	if(string == NULL)
		return -1;
	if((start == NULL) || (*start == NULL) || ((*start)->type == XML_NAMESPACE_DECL) || (startindex == NULL))
		return -1;
	if((end == NULL) || (endindex == NULL))
		return -1;
	cur = *start;
	pos = *startindex - 1;
	first = string[0];
	while(cur) {
		if((cur->type != XML_ELEMENT_NODE) && cur->content) {
			len = sstrlen(cur->content);
			while(pos <= len) {
				if(first != 0) {
					str = xmlStrchr(&cur->content[pos], first);
					if(str) {
						pos = (str - (xmlChar *)(cur->content));
#ifdef DEBUG_RANGES
						xmlGenericError(0, "found '%c' at index %d of ->", first, pos + 1);
						xmlDebugDumpString(stdout, cur->content);
						xmlGenericError(0, "\n");
#endif
						if(xmlXPtrMatchString(string, cur, pos + 1, end, endindex)) {
							*start = cur;
							*startindex = pos + 1;
							return 1;
						}
						pos++;
					}
					else {
						pos = len + 1;
					}
				}
				else {
					/*
					 * An empty string is considered to match before each
					 * character of the string-value and after the final
					 * character.
					 */
#ifdef DEBUG_RANGES
					xmlGenericError(0, "found '' at index %d of ->", pos + 1);
					xmlDebugDumpString(stdout, cur->content);
					xmlGenericError(0, "\n");
#endif
					*start = cur;
					*startindex = pos + 1;
					*end = cur;
					*endindex = pos + 1;
					return 1;
				}
			}
		}
		if((cur == *end) && (pos >= *endindex))
			return 0;
		cur = xmlXPtrAdvanceNode(cur, 0);
		if(!cur)
			return 0;
		pos = 1;
	}
	return 0;
}

/**
 * xmlXPtrGetLastChar:
 * @node:  the node
 * @index:  the index
 *
 * Computes the point coordinates of the last char of this point
 *
 * Returns -1 in case of failure, 0 otherwise
 */
static int xmlXPtrGetLastChar(xmlNode ** P_Node, int * indx) 
{
	xmlNode * cur;
	int pos, len = 0;
	if(!P_Node || (*P_Node == NULL) || ((*P_Node)->type == XML_NAMESPACE_DECL) || (indx == NULL))
		return -1;
	cur = *P_Node;
	pos = *indx;
	if(oneof3(cur->type, XML_ELEMENT_NODE, XML_DOCUMENT_NODE, XML_HTML_DOCUMENT_NODE)) {
		if(pos > 0) {
			cur = xmlXPtrGetNthChild(cur, pos);
		}
	}
	while(cur) {
		if(cur->last)
			cur = cur->last;
		else if((cur->type != XML_ELEMENT_NODE) && cur->content) {
			len = sstrlen(cur->content);
			break;
		}
		else {
			return -1;
		}
	}
	if(!cur)
		return -1;
	*P_Node = cur;
	*indx = len;
	return 0;
}

/**
 * xmlXPtrGetStartPoint:
 * @obj:  an range
 * @node:  the resulting node
 * @indx:  the resulting index
 *
 * read the object and return the start point coordinates.
 *
 * Returns -1 in case of failure, 0 otherwise
 */
static int xmlXPtrGetStartPoint(xmlXPathObject * obj, xmlNode ** P_Node, int * indx)
{
	if(obj && P_Node && indx) {
		switch(obj->type) {
			case XPATH_POINT:
				*P_Node = (xmlNode *)obj->user;
				if(obj->index <= 0)
					*indx = 0;
				else
					*indx = obj->index;
				return 0;
			case XPATH_RANGE:
				*P_Node = (xmlNode *)obj->user;
				if(obj->index <= 0)
					*indx = 0;
				else
					*indx = obj->index;
				return 0;
			default:
				break;
		}
	}
	return -1;
}

/**
 * xmlXPtrGetEndPoint:
 * @obj:  an range
 * @node:  the resulting node
 * @indx:  the resulting indx
 *
 * read the object and return the end point coordinates.
 *
 * Returns -1 in case of failure, 0 otherwise
 */
static int xmlXPtrGetEndPoint(xmlXPathObject * obj, xmlNode ** P_Node, int * indx)
{
	if(!obj || !P_Node || !indx)
		return -1;
	switch(obj->type) {
		case XPATH_POINT:
		    *P_Node = (xmlNode *)obj->user;
		    if(obj->index <= 0)
			    *indx = 0;
		    else
			    *indx = obj->index;
		    return 0;
		case XPATH_RANGE:
		    *P_Node = (xmlNode *)obj->user;
		    if(obj->index <= 0)
			    *indx = 0;
		    else
			    *indx = obj->index;
		    return 0;
		default:
		    break;
	}
	return -1;
}
/**
 * xmlXPtrStringRangeFunction:
 * @ctxt:  the XPointer Parser context
 * @nargs:  the number of args
 *
 * Function implementing the string-range() function
 * range as described in 5.4.2
 *
 * ------------------------------
 * [Definition: For each location in the location-set argument,
 * string-range returns a set of string ranges, a set of substrings in a
 * string. Specifically, the string-value of the location is searched for
 * substrings that match the string argument, and the resulting location-set
 * will contain a range location for each non-overlapping match.]
 * An empty string is considered to match before each character of the
 * string-value and after the final character. Whitespace in a string
 * is matched literally, with no normalization except that provided by
 * XML for line ends. The third argument gives the position of the first
 * character to be in the resulting range, relative to the start of the
 * match. The default value is 1, which makes the range start immediately
 * before the first character of the matched string. The fourth argument
 * gives the number of characters in the range; the default is that the
 * range extends to the end of the matched string.
 *
 * Element boundaries, as well as entire embedded nodes such as processing
 * instructions and comments, are ignored as defined in [XPath].
 *
 * If the string in the second argument is not found in the string-value
 * of the location, or if a value in the third or fourth argument indicates
 * a string that is beyond the beginning or end of the document, the
 * expression fails.
 *
 * The points of the range-locations in the returned location-set will
 * all be character points.
 * ------------------------------
 */
static void xmlXPtrStringRangeFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
	int i, startindex, endindex = 0, fendindex;
	xmlNode * start;
	xmlNode * end = 0;
	xmlNode * fend;
	xmlXPathObjectPtr set;
	xmlLocationSet * oldset;
	xmlLocationSet * newset;
	xmlXPathObjectPtr string;
	xmlXPathObjectPtr position = NULL;
	xmlXPathObjectPtr number = NULL;
	int found, pos = 0, num = 0;
	/*
	 * Grab the arguments
	 */
	if((nargs < 2) || (nargs > 4))
		XP_ERROR(XPATH_INVALID_ARITY);
	if(nargs >= 4) {
		CHECK_TYPE(XPATH_NUMBER);
		number = valuePop(ctxt);
		if(number)
			num = (int)number->floatval;
	}
	if(nargs >= 3) {
		CHECK_TYPE(XPATH_NUMBER);
		position = valuePop(ctxt);
		if(position)
			pos = (int)position->floatval;
	}
	CHECK_TYPE(XPATH_STRING);
	string = valuePop(ctxt);
	if(!ctxt->value || ((ctxt->value->type != XPATH_LOCATIONSET) && (ctxt->value->type != XPATH_NODESET)))
		XP_ERROR(XPATH_INVALID_TYPE)
		set = valuePop(ctxt);
	newset = xmlXPtrLocationSetCreate(NULL);
	if(newset == NULL) {
		xmlXPathFreeObject(set);
		XP_ERROR(XPATH_MEMORY_ERROR);
	}
	if(set->nodesetval == NULL) {
		goto error;
	}
	if(set->type == XPATH_NODESET) {
		/*
		 * First convert to a location set
		 */
		xmlXPathObjectPtr tmp = xmlXPtrNewLocationSetNodeSet(set->nodesetval);
		xmlXPathFreeObject(set);
		if(!tmp)
			XP_ERROR(XPATH_MEMORY_ERROR)
			set = tmp;
	}
	oldset = (xmlLocationSet *)set->user;
	/*
	 * The loop is to search for each element in the location set
	 * the list of location set corresponding to that search
	 */
	for(i = 0; i < oldset->locNr; i++) {
#ifdef DEBUG_RANGES
		xmlXPathDebugDumpObject(stdout, oldset->locTab[i], 0);
#endif
		xmlXPtrGetStartPoint(oldset->locTab[i], &start, &startindex);
		xmlXPtrGetEndPoint(oldset->locTab[i], &end, &endindex);
		xmlXPtrAdvanceChar(&start, &startindex, 0);
		xmlXPtrGetLastChar(&end, &endindex);
#ifdef DEBUG_RANGES
		xmlGenericError(0, "from index %d of ->", startindex);
		xmlDebugDumpString(stdout, start->content);
		xmlGenericError(0, "\n");
		xmlGenericError(0, "to index %d of ->", endindex);
		xmlDebugDumpString(stdout, end->content);
		xmlGenericError(0, "\n");
#endif
		do {
			fend = end;
			fendindex = endindex;
			found = xmlXPtrSearchString(string->stringval, &start, &startindex, &fend, &fendindex);
			if(found == 1) {
				if(position == NULL) {
					xmlXPtrLocationSetAdd(newset, xmlXPtrNewRange(start, startindex, fend, fendindex));
				}
				else if(xmlXPtrAdvanceChar(&start, &startindex, pos - 1) == 0) {
					if(number && num > 0) {
						xmlNode * rend = start;
						int rindx = startindex - 1;
						if(xmlXPtrAdvanceChar(&rend, &rindx, num) == 0) {
							xmlXPtrLocationSetAdd(newset, xmlXPtrNewRange(start, startindex, rend, rindx));
						}
					}
					else if(number && (num <= 0)) {
						xmlXPtrLocationSetAdd(newset, xmlXPtrNewRange(start, startindex, start, startindex));
					}
					else {
						xmlXPtrLocationSetAdd(newset, xmlXPtrNewRange(start, startindex, fend, fendindex));
					}
				}
				start = fend;
				startindex = fendindex;
				if(string->stringval[0] == 0)
					startindex++;
			}
		} while(found == 1);
	}
	/*
	 * Save the new value and cleanup
	 */
error:
	valuePush(ctxt, xmlXPtrWrapLocationSet(newset));
	xmlXPathFreeObject(set);
	xmlXPathFreeObject(string);
	xmlXPathFreeObject(position);
	xmlXPathFreeObject(number);
}

/**
 * xmlXPtrEvalRangePredicate:
 * @ctxt:  the XPointer Parser context
 *
 *  [8]   Predicate ::=   '[' PredicateExpr ']'
 *  [9]   PredicateExpr ::=   Expr
 *
 * Evaluate a predicate as in xmlXPathEvalPredicate() but for
 * a Location Set instead of a node set
 */
void xmlXPtrEvalRangePredicate(xmlXPathParserContextPtr ctxt)
{
	const xmlChar * cur;
	xmlXPathObjectPtr res;
	xmlXPathObject * obj;
	xmlXPathObject * tmp;
	xmlLocationSet * newset = NULL;
	xmlLocationSet * oldset;
	int i;
	if(ctxt) {
		SKIP_BLANKS;
		if(CUR != '[') {
			XP_ERROR(XPATH_INVALID_PREDICATE_ERROR);
		}
		NEXT;
		SKIP_BLANKS;
		/*
		* Extract the old set, and then evaluate the result of the
		* expression for all the element in the set. use it to grow
		* up a new set.
		*/
		CHECK_TYPE(XPATH_LOCATIONSET);
		obj = valuePop(ctxt);
		oldset = (xmlLocationSet *)obj->user;
		ctxt->context->P_Node = NULL;
		if((oldset == NULL) || (oldset->locNr == 0)) {
			ctxt->context->contextSize = 0;
			ctxt->context->proximityPosition = 0;
			xmlXPathEvalExpr(ctxt);
			res = valuePop(ctxt);
			xmlXPathFreeObject(res);
			valuePush(ctxt, obj);
			CHECK_ERROR;
		}
		else {
			/*
			* Save the expression pointer since we will have to evaluate
			* it multiple times. Initialize the new set.
			*/
			cur = ctxt->cur;
			newset = xmlXPtrLocationSetCreate(NULL);
			for(i = 0; i < oldset->locNr; i++) {
				ctxt->cur = cur;
				/*
				* Run the evaluation with a node list made of a single item
				* in the nodeset.
				*/
				ctxt->context->P_Node = (xmlNode *)oldset->locTab[i]->user;
				tmp = xmlXPathNewNodeSet(ctxt->context->P_Node);
				valuePush(ctxt, tmp);
				ctxt->context->contextSize = oldset->locNr;
				ctxt->context->proximityPosition = i + 1;
				xmlXPathEvalExpr(ctxt);
				CHECK_ERROR;
				/*
				* The result of the evaluation need to be tested to
				* decided whether the filter succeeded or not
				*/
				res = valuePop(ctxt);
				if(xmlXPathEvaluatePredicateResult(ctxt, res)) {
					xmlXPtrLocationSetAdd(newset, xmlXPathObjectCopy(oldset->locTab[i]));
				}
				/*
				* Cleanup
				*/
				xmlXPathFreeObject(res);
				if(ctxt->value == tmp) {
					res = valuePop(ctxt);
					xmlXPathFreeObject(res);
				}
				ctxt->context->P_Node = NULL;
			}
			/*
			* The result is used as the new evaluation set.
			*/
			xmlXPathFreeObject(obj);
			ctxt->context->P_Node = NULL;
			ctxt->context->contextSize = -1;
			ctxt->context->proximityPosition = -1;
			valuePush(ctxt, xmlXPtrWrapLocationSet(newset));
		}
		if(CUR != ']') {
			XP_ERROR(XPATH_INVALID_PREDICATE_ERROR);
		}
		NEXT;
		SKIP_BLANKS;
	}
}

#define bottom_xpointer
//#include "elfgcchack.h"
#endif
