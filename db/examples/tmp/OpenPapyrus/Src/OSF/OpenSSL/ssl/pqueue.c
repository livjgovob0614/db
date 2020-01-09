/*
 * Copyright 2005-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "ssl_locl.h"
#pragma hdrstop

struct pqueue_st {
	pitem * items;
	int count;
};

pitem * pitem_new(uchar * prio64be, void * data)
{
	pitem * item = (pitem*)OPENSSL_malloc(sizeof(*item));
	if(item) {
		memcpy(item->priority, prio64be, sizeof(item->priority));
		item->data = data;
		item->next = NULL;
	}
	return item;
}

void pitem_free(pitem * item)
{
	OPENSSL_free(item);
}

pqueue * pqueue_new()
{
	pqueue * pq = (pqueue*)OPENSSL_zalloc(sizeof(*pq));
	return pq;
}

void pqueue_free(pqueue * pq)
{
	OPENSSL_free(pq);
}

pitem * pqueue_insert(pqueue * pq, pitem * item)
{
	pitem * curr, * next;
	if(pq->items == NULL) {
		pq->items = item;
		return item;
	}
	for(curr = NULL, next = pq->items; next; curr = next, next = next->next) {
		/*
		 * we can compare 64-bit value in big-endian encoding with memcmp:-)
		 */
		int cmp = memcmp(next->priority, item->priority, 8);
		if(cmp > 0) {   /* next > item */
			item->next = next;
			if(curr == NULL)
				pq->items = item;
			else
				curr->next = item;
			return item;
		}
		else if(cmp == 0) /* duplicates not allowed */
			return NULL;
	}
	item->next = NULL;
	curr->next = item;
	return item;
}

pitem * pqueue_peek(pqueue * pq)
{
	return pq->items;
}

pitem * pqueue_pop(pqueue * pq)
{
	pitem * item = pq->items;
	if(pq->items != NULL)
		pq->items = pq->items->next;
	return item;
}

pitem * pqueue_find(pqueue * pq, uchar * prio64be)
{
	pitem * next;
	pitem * found = NULL;
	if(pq->items == NULL)
		return NULL;
	for(next = pq->items; next->next; next = next->next) {
		if(memcmp(next->priority, prio64be, 8) == 0) {
			found = next;
			break;
		}
	}
	/* check the one last node */
	if(memcmp(next->priority, prio64be, 8) == 0)
		found = next;
	return found;
}

pitem * pqueue_iterator(pqueue * pq)
{
	return pqueue_peek(pq);
}

pitem * pqueue_next(pitem ** item)
{
	pitem * ret;
	if(item == NULL || *item == NULL)
		return NULL;
	/* *item != NULL */
	ret = *item;
	*item = (*item)->next;
	return ret;
}

int pqueue_size(pqueue * pq)
{
	int count = 0;
	for(pitem * item = pq->items; item; item = item->next)
		count++;
	return count;
}

