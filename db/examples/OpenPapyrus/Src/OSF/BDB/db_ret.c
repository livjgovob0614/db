/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * __db_ret --
 *	Build return DBT.
 */
int __db_ret(DBC * dbc, PAGE * h, uint32 indx, DBT * dbt, void ** memp, uint32 * memsize)
{
	BKEYDATA * bk;
	BOVERFLOW * bo;
	HEAPHDR * hdr;
	HOFFPAGE ho;
	uint32 len;
	uint8 * hk;
	void * data;
	if(F_ISSET(dbt, DB_DBT_READONLY))
		return 0;
	else {
		DB * dbp = dbc->dbp;
		switch(TYPE(h)) {
			case P_HASH_UNSORTED:
			case P_HASH:
				hk = P_ENTRY(dbp, h, indx);
				if(HPAGE_PTYPE(hk) == H_OFFPAGE) {
					memcpy(&ho, hk, sizeof(HOFFPAGE));
					return __db_goff(dbc, dbt, ho.tlen, ho.pgno, memp, memsize);
				}
				len = LEN_HKEYDATA(dbp, h, dbp->pgsize, indx);
				data = HKEYDATA_DATA(hk);
				break;
			case P_HEAP:
				hdr = (HEAPHDR *)P_ENTRY(dbp, h, indx);
				if(F_ISSET(hdr, (HEAP_RECSPLIT|HEAP_RECFIRST)))
					return __heapc_gsplit(dbc, dbt, memp, memsize);
				len = hdr->size;
				data = (uint8 *)hdr+sizeof(HEAPHDR);
				break;
			case P_LBTREE:
			case P_LDUP:
			case P_LRECNO:
				bk = GET_BKEYDATA(dbp, h, indx);
				if(B_TYPE(bk->type) == B_OVERFLOW) {
					bo = (BOVERFLOW *)bk;
					return __db_goff(dbc, dbt, bo->tlen, bo->pgno, memp, memsize);
				}
				len = bk->len;
				data = bk->data;
				break;
			default:
				return __db_pgfmt(dbp->env, h->pgno);
		}
		return __db_retcopy(dbp->env, dbt, data, len, memp, memsize);
	}
}
/*
 * __db_retcopy --
 *	Copy the returned data into the user's DBT, handling special flags.
 */
int __db_retcopy(ENV * env, DBT * dbt, void * data, uint32 len, void ** memp, uint32 * memsize)
{
	int ret = 0;
	if(!F_ISSET(dbt, DB_DBT_READONLY)) {
		// If returning a partial record, reset the length. 
		if(F_ISSET(dbt, DB_DBT_PARTIAL)) {
			data = (uint8 *)data+dbt->doff;
			if(len > dbt->doff) {
				len -= dbt->doff;
				if(len > dbt->dlen)
					len = dbt->dlen;
			}
			else
				len = 0;
		}
		/*
		 * Allocate memory to be owned by the application: DB_DBT_MALLOC,
		 * DB_DBT_REALLOC.
		 *
		 * !!!
		 * We always allocate memory, even if we're copying out 0 bytes. This
		 * guarantees consistency, i.e., the application can always free memory
		 * without concern as to how many bytes of the record were requested.
		 *
		 * Use the memory specified by the application: DB_DBT_USERMEM.
		 *
		 * !!!
		 * If the length we're going to copy is 0, the application-supplied
		 * memory pointer is allowed to be NULL.
		 */
		if(F_ISSET(dbt, DB_DBT_USERCOPY)) {
			dbt->size = len;
			return (len == 0) ? 0 : env->dbt_usercopy(dbt, 0, data, len, DB_USERCOPY_SETDATA);
		}
		else if(F_ISSET(dbt, DB_DBT_MALLOC))
			ret = __os_umalloc(env, len, &dbt->data);
		else if(F_ISSET(dbt, DB_DBT_REALLOC)) {
			if(dbt->data == NULL || dbt->size == 0 || dbt->size < len)
				ret = __os_urealloc(env, len, &dbt->data);
		}
		else if(F_ISSET(dbt, DB_DBT_USERMEM)) {
			if(len != 0 && (dbt->data == NULL || dbt->ulen < len))
				ret = DB_BUFFER_SMALL;
		}
		else if(memp == NULL || memsize == NULL)
			ret = EINVAL;
		else {
			if(len && (*memsize == 0 || *memsize < len)) {
				*memsize = ((ret = __os_realloc(env, len, memp)) == 0) ? len : 0;
			}
			if(!ret)
				dbt->data = *memp;
		}
		if(ret == 0 && len != 0)
			memcpy(dbt->data, data, len);
		/*
		 * Return the length of the returned record in the DBT size field.
		 * This satisfies the requirement that if we're using user memory
		 * and insufficient memory was provided, return the amount necessary
		 * in the size field.
		 */
		dbt->size = len;
	}
	return ret;
}
