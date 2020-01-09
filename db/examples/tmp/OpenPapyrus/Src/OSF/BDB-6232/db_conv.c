/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
#include "dbinc/crypto.h"
#include "dbinc/hmac.h"
#include "dbinc/db_page.h"
#include "dbinc/db_swap.h"
#include "dbinc/btree.h"
#include "dbinc/fop.h"
#include "dbinc/hash.h"
#include "dbinc/heap.h"
#include "dbinc/qam.h"

static int __db_convert_extent __P((ENV *, const char *, uint32, uint32));
static int __db_convert_extent_names __P((DB *, DBMETA *, char *, char ***));
static int __db_swap __P((DB *, char *, uint32, DB_FH *, PAGE *, int *));

/*
 * __db_pgin --
 *	Primary page-swap routine.
 *
 * PUBLIC: int __db_pgin __P((DB_ENV *, db_pgno_t, void *, DBT *));
 */
int __db_pgin(DB_ENV *dbenv, db_pgno_t pg, void * pp, DBT * cookie)
{
	DB dummydb, * dbp;
	DB_CIPHER * db_cipher;
	DB_LSN not_used;
	size_t sum_len;
	int is_hmac, ret;
	uint8 * chksum;
	DB_PGINFO * pginfo = (DB_PGINFO*)cookie->data;
	ENV * env = dbenv->env;
	PAGE * pagep = (PAGE*)pp;
	ret = is_hmac = 0;
	chksum = NULL;
	memzero(&dummydb, sizeof(DB));
	dbp = &dummydb;
	dbp->dbenv = dbenv;
	dbp->env = env;
	dbp->flags = pginfo->flags;
	dbp->pgsize = pginfo->db_pagesize;
	db_cipher = env->crypto_handle;
	switch(pagep->type) {
		case P_HASHMETA:
		case P_HEAPMETA:
		case P_BTREEMETA:
		case P_QAMMETA:
		    /*
		     * If checksumming is set on the meta-page, we must set
		     * it in the dbp.
		     */
		    if(FLD_ISSET(((DBMETA*)pp)->metaflags, DBMETA_CHKSUM))
			    F_SET(dbp, DB_AM_CHKSUM);
		    else
			    F_CLR(dbp, DB_AM_CHKSUM);
		    if(((DBMETA*)pp)->encrypt_alg != 0 ||
			F_ISSET(dbp, DB_AM_ENCRYPT))
			    is_hmac = 1;
		    /*
		     * !!!
		     * For all meta pages it is required that the chksum
		     * be at the same location.  Use BTMETA to get to it
		     * for any meta type.
		     */
		    chksum = ((BTMETA*)pp)->chksum;
		    sum_len = DBMETASIZE;
		    break;
		case P_INVALID:
		    /*
		     * We assume that we've read a file hole if we have
		     * a zero LSN, zero page number and P_INVALID.  Otherwise
		     * we have an invalid page that might contain real data.
		     */
		    if(IS_ZERO_LSN(LSN(pagep)) && pagep->pgno == PGNO_INVALID) {
			    sum_len = 0;
			    break;
		    }
		/* FALLTHROUGH */
		default:
		    chksum = P_CHKSUM(dbp, pagep);
		    sum_len = pginfo->db_pagesize;
		    /*
		     * If we are reading in a non-meta page, then if we have
		     * a db_cipher then we are using hmac.
		     */
		    is_hmac = CRYPTO_ON(env) ? 1 : 0;
		    break;
	}

	/*
	 * We expect a checksum error if there was a configuration problem.
	 * If there is no configuration problem and we don't get a match,
	 * it's fatal: panic the system.
	 */
	if(F_ISSET(dbp, DB_AM_CHKSUM) && sum_len != 0) {
		if(F_ISSET(dbp, DB_AM_SWAP) && is_hmac == 0)
			P_32_SWAP(chksum);
		switch(ret = __db_check_chksum(
			    env, NULL, db_cipher, chksum, pp, sum_len, is_hmac)) {
			case 0:
			    break;
			case -1:
			    if(DBENV_LOGGING(env))
				    (void)__db_cksum_log(env, NULL, &not_used, DB_FLUSH);
			    __db_errx(env, DB_STR_A("0684", "checksum error: page %lu: catastrophic recovery required", "%lu"), (u_long)pg);
			    return (__env_panic(env, DB_RUNRECOVERY));
			default:
			    return ret;
		}
	}
	if((ret = __db_decrypt_pg(env, dbp, pagep)) != 0)
		return ret;
	switch(pagep->type) {
		case P_INVALID:
		    if(pginfo->type == DB_QUEUE)
			    return (__qam_pgin_out(env, pg, pp, cookie));
		    else if(pginfo->type == DB_HEAP)
			    return (__heap_pgin(dbp, pg, pp, cookie));
		    /*
		     * This page is either newly allocated from the end of the
		     * file, or from the free list, or it is an as-yet unwritten
		     * hash bucket page. In this last case it needs to be
		     * initialized, but never byte-swapped. Otherwise the header
		     * may need swapping. It will not be a metadata page, so the
		     * byte swapping code of __ham_pgin is adequate.  If hash
		     * is not configured fall back to btree swapping.
		     */
#ifdef HAVE_HASH
		    return (__ham_pgin(dbp, pg, pp, cookie));
#else
		    return (__bam_pgin(dbp, pg, pp, cookie));
#endif
		    /* NOTREACHED. */
		    break;
		case P_HASH_UNSORTED:
		case P_HASH:
		case P_HASHMETA:
		    return (__ham_pgin(dbp, pg, pp, cookie));
		case P_HEAP:
		case P_HEAPMETA:
		case P_IHEAP:
		    return (__heap_pgin(dbp, pg, pp, cookie));
		case P_BTREEMETA:
		case P_IBTREE:
		case P_IRECNO:
		case P_LBTREE:
		case P_LDUP:
		case P_LRECNO:
		case P_OVERFLOW:
		    return (__bam_pgin(dbp, pg, pp, cookie));
		case P_QAMMETA:
		case P_QAMDATA:
		    return (__qam_pgin_out(env, pg, pp, cookie));
		default:
		    break;
	}
	return (__db_pgfmt(env, pg));
}

/*
 * __db_pgout --
 *	Primary page-swap routine.
 *
 * PUBLIC: int __db_pgout __P((DB_ENV *, db_pgno_t, void *, DBT *));
 */
int __db_pgout(DB_ENV *dbenv, db_pgno_t pg, void * pp, DBT * cookie)
{
	DB dummydb, * dbp;
	DB_PGINFO * pginfo;
	ENV * env;
	PAGE * pagep;
	int ret;
	pginfo = (DB_PGINFO*)cookie->data;
	env = dbenv->env;
	pagep = (PAGE*)pp;
	memzero(&dummydb, sizeof(DB));
	dbp = &dummydb;
	dbp->dbenv = dbenv;
	dbp->env = env;
	dbp->flags = pginfo->flags;
	dbp->pgsize = pginfo->db_pagesize;
	ret = 0;
	switch(pagep->type) {
		case P_INVALID:
		    switch(pginfo->type) {
			    case DB_QUEUE:
				ret = __qam_pgin_out(env, pg, pp, cookie);
				break;
#ifdef HAVE_HASH
			    case DB_HASH:
				ret = __ham_pgout(dbp, pg, pp, cookie);
				break;
#endif
#ifdef HAVE_HEAP
			    case DB_HEAP:
				ret = __heap_pgout(dbp, pg, pp, cookie);
				break;
#endif
			    case DB_BTREE:
			    case DB_RECNO:
				ret = __bam_pgout(dbp, pg, pp, cookie);
				break;
			    default:
				return (__db_pgfmt(env, pg));
		    }
		    break;
		case P_HASH:
		case P_HASH_UNSORTED:
		/*
		 * Support pgout of unsorted hash pages - since online
		 * replication upgrade can cause pages of this type to be
		 * written out.
		 *
		 * FALLTHROUGH
		 */
		case P_HASHMETA:
		    ret = __ham_pgout(dbp, pg, pp, cookie);
		    break;
		case P_HEAP:
		case P_HEAPMETA:
		case P_IHEAP:
		    ret = __heap_pgout(dbp, pg, pp, cookie);
		    break;
		case P_BTREEMETA:
		case P_IBTREE:
		case P_IRECNO:
		case P_LBTREE:
		case P_LDUP:
		case P_LRECNO:
		case P_OVERFLOW:
		    ret = __bam_pgout(dbp, pg, pp, cookie);
		    break;
		case P_QAMMETA:
		case P_QAMDATA:
		    ret = __qam_pgin_out(env, pg, pp, cookie);
		    break;
		default:
		    return (__db_pgfmt(env, pg));
	}
	if(ret)
		return ret;

	return (__db_encrypt_and_checksum_pg(env, dbp, pagep));
}
/*
 * __db_decrypt_pg --
 *      Utility function to decrypt a db page.
 *
 * PUBLIC: int __db_decrypt_pg __P((ENV *, DB *, PAGE *));
 */
int __db_decrypt_pg(ENV *env, DB * dbp, PAGE * pagep)
{
	size_t pg_len, pg_off;
	uint8 * iv = 0;
	int ret = 0;
	DB_CIPHER * db_cipher = env->crypto_handle;
	if(F_ISSET(dbp, DB_AM_ENCRYPT)) {
		DB_ASSERT(env, db_cipher != NULL);
		DB_ASSERT(env, F_ISSET(dbp, DB_AM_CHKSUM));

		pg_off = P_OVERHEAD(dbp);
		DB_ASSERT(env, db_cipher->adj_size(pg_off) == 0);

		switch(pagep->type) {
			case P_HASHMETA:
			case P_HEAPMETA:
			case P_BTREEMETA:
			case P_QAMMETA:
			    /*
			     * !!!
			     * For all meta pages it is required that the iv
			     * be at the same location.  Use BTMETA to get to it
			     * for any meta type.
			     */
			    iv = ((BTMETA*)pagep)->iv;
			    pg_len = DBMETASIZE;
			    break;
			case P_INVALID:
			    if(IS_ZERO_LSN(LSN(pagep)) && pagep->pgno == PGNO_INVALID) {
				    pg_len = 0;
				    break;
			    }
			/* FALLTHROUGH */
			default:
			    iv = P_IV(dbp, pagep);
			    pg_len = dbp->pgsize;
			    break;
		}
		if(pg_len != 0)
			ret = db_cipher->decrypt(env, db_cipher->data, iv, ((uint8*)pagep) + pg_off, pg_len - pg_off);
	}
	return ret;
}

/*
 * __db_encrypt_and_checksum_pg --
 *	Utility function to encrypt and checksum a db page.
 *
 * PUBLIC: int __db_encrypt_and_checksum_pg
 * PUBLIC:     __P((ENV *, DB *, PAGE *));
 */
int __db_encrypt_and_checksum_pg(ENV *env, DB * dbp, PAGE * pagep)
{
	DB_CIPHER * db_cipher;
	int ret;
	size_t pg_off, pg_len, sum_len;
	uint8 * chksum, * iv, * key;
	chksum = iv = key = NULL;
	db_cipher = env->crypto_handle;
	if(F_ISSET(dbp, DB_AM_ENCRYPT)) {
		DB_ASSERT(env, db_cipher != NULL);
		DB_ASSERT(env, F_ISSET(dbp, DB_AM_CHKSUM));

		pg_off = P_OVERHEAD(dbp);
		DB_ASSERT(env, db_cipher->adj_size(pg_off) == 0);

		key = db_cipher->mac_key;

		switch(pagep->type) {
			case P_HASHMETA:
			case P_HEAPMETA:
			case P_BTREEMETA:
			case P_QAMMETA:
			    /*
			     * !!!
			     * For all meta pages it is required that the iv
			     * be at the same location.  Use BTMETA to get to it
			     * for any meta type.
			     */
			    iv = ((BTMETA*)pagep)->iv;
			    pg_len = DBMETASIZE;
			    break;
			default:
			    iv = P_IV(dbp, pagep);
			    pg_len = dbp->pgsize;
			    break;
		}
		if((ret = db_cipher->encrypt(env, db_cipher->data,
		    iv, ((uint8*)pagep) + pg_off, pg_len - pg_off)) != 0)
			return ret;
	}
	if(F_ISSET(dbp, DB_AM_CHKSUM)) {
		switch(pagep->type) {
			case P_HASHMETA:
			case P_HEAPMETA:
			case P_BTREEMETA:
			case P_QAMMETA:
			    /*
			     * !!!
			     * For all meta pages it is required that the chksum
			     * be at the same location.  Use BTMETA to get to it
			     * for any meta type.
			     */
			    chksum = ((BTMETA*)pagep)->chksum;
			    sum_len = DBMETASIZE;
			    break;
			default:
			    chksum = P_CHKSUM(dbp, pagep);
			    sum_len = dbp->pgsize;
			    break;
		}
		__db_chksum(NULL, (uint8*)pagep, sum_len, key, chksum);
		if(F_ISSET(dbp, DB_AM_SWAP) && !F_ISSET(dbp, DB_AM_ENCRYPT))
			P_32_SWAP(chksum);
	}
	return 0;
}

/*
 * __db_metaswap --
 *	Byteswap the common part of the meta-data page.
 *
 * PUBLIC: void __db_metaswap __P((PAGE *));
 */
void __db_metaswap(PAGE *pg)
{
	uint8 * p = (uint8*)pg;
	/* Swap the meta-data information. */
	SWAP32(p);      /* lsn.file */
	SWAP32(p);      /* lsn.offset */
	SWAP32(p);      /* pgno */
	SWAP32(p);      /* magic */
	SWAP32(p);      /* version */
	SWAP32(p);      /* pagesize */
	p += 4;         /* unused, page type, unused, unused */
	SWAP32(p);      /* free */
	SWAP32(p);      /* alloc_lsn part 1 */
	SWAP32(p);      /* alloc_lsn part 2 */
	SWAP32(p);      /* cached key count */
	SWAP32(p);      /* cached record count */
	SWAP32(p);      /* flags */
}

/*
 * __db_byteswap --
 *	Byteswap an ordinary database page.
 *
 * PUBLIC: int __db_byteswap
 * PUBLIC:         __P((DB *, db_pgno_t, PAGE *, size_t, int));
 */
int __db_byteswap(DB *dbp, db_pgno_t pg, PAGE * h, size_t pagesize, int pgin)
{
	ENV * env;
	BINTERNAL * bi;
	BBLOB * bl;
	BKEYDATA * bk;
	BOVERFLOW * bo;
	HEAPBLOBHDR * bhdr;
	HEAPHDR * hh;
	HEAPSPLITHDR * hsh;
	RINTERNAL * ri;
	db_indx_t i, * inp, len, tmp;
	uint8 * end, * p, * pgend;

	if(pagesize == 0)
		return 0;

	if(pgin) {
		M_32_SWAP(h->lsn.file);
		M_32_SWAP(h->lsn.offset);
		M_32_SWAP(h->pgno);
		if(TYPE(h) == P_HEAP) {
			M_32_SWAP(((HEAPPG*)h)->high_pgno);
			M_16_SWAP(((HEAPPG*)h)->high_indx);
			M_16_SWAP(((HEAPPG*)h)->free_indx);
		}
		else {
			M_32_SWAP(h->prev_pgno);
			M_32_SWAP(h->next_pgno);
		}
		M_16_SWAP(h->entries);
		M_16_SWAP(h->hf_offset);
	}

	if(dbp == NULL)
		return 0;
	env = dbp->env;

	pgend = (uint8*)h + pagesize;

	inp = P_INP(dbp, h);
	if((uint8*)inp >= pgend)
		goto out;

	switch(TYPE(h)) {
		case P_HASH_UNSORTED:
		case P_HASH:
		    for(i = 0; i < NUM_ENT(h); i++) {
			    if(pgin)
				    M_16_SWAP(inp[i]);

			    if(P_ENTRY(dbp, h, i) >= pgend)
				    continue;

			    switch(HPAGE_TYPE(dbp, h, i)) {
				    case H_BLOB:
					p = HBLOB_ID(P_ENTRY(dbp, h, i));
					SWAP64(p);              /* id */
					SWAP64(p);              /* size */
					p = HBLOB_FILE_ID(P_ENTRY(dbp, h, i));
					SWAP64(p);              /* file id */
					SWAP64(p);              /* sdb id */
					break;
				    case H_KEYDATA:
					break;
				    case H_DUPLICATE:
					len = LEN_HKEYDATA(dbp, h, pagesize, i);
					p = HKEYDATA_DATA(P_ENTRY(dbp, h, i));
					for(end = p + len; p < end;) {
						if(pgin) {
							P_16_SWAP(p);
							memcpy(&tmp,
							    p, sizeof(db_indx_t));
							p += sizeof(db_indx_t);
						}
						else {
							memcpy(&tmp,
							    p, sizeof(db_indx_t));
							SWAP16(p);
						}
						p += tmp;
						SWAP16(p);
					}
					break;
				    case H_OFFDUP:
					p = HOFFPAGE_PGNO(P_ENTRY(dbp, h, i));
					SWAP32(p);              /* pgno */
					break;
				    case H_OFFPAGE:
					p = HOFFPAGE_PGNO(P_ENTRY(dbp, h, i));
					SWAP32(p);              /* pgno */
					SWAP32(p);              /* tlen */
					break;
				    default:
					return (__db_pgfmt(env, pg));
			    }
		    }

		    /*
		     * The offsets in the inp array are used to determine
		     * the size of entries on a page; therefore they
		     * cannot be converted until we've done all the
		     * entries.
		     */
		    if(!pgin)
			    for(i = 0; i < NUM_ENT(h); i++)
				    M_16_SWAP(inp[i]);
		    break;
		case P_LBTREE:
		case P_LDUP:
		case P_LRECNO:
		    for(i = 0; i < NUM_ENT(h); i++) {
			    if(pgin)
				    M_16_SWAP(inp[i]);

			    /*
			     * In the case of on-page duplicates, key information
			     * should only be swapped once.
			     */
			    if(h->type == P_LBTREE && i > 1) {
				    if(pgin) {
					    if(inp[i] == inp[i - 2])
						    continue;
				    }
				    else {
					    M_16_SWAP(inp[i]);
					    if(inp[i] == inp[i - 2])
						    continue;
					    M_16_SWAP(inp[i]);
				    }
			    }

			    bk = GET_BKEYDATA(dbp, h, i);
			    if((uint8*)bk >= pgend)
				    continue;
			    switch(B_TYPE(bk->type)) {
				    case B_BLOB:
					bl = (BBLOB*)bk;
					M_16_SWAP(bl->len);
					M_64_SWAP(bl->id);      /* id */
					M_64_SWAP(bl->size);    /* size */
					M_64_SWAP(bl->file_id); /* file id */
					M_64_SWAP(bl->sdb_id);  /* sdb id */
					break;
				    case B_KEYDATA:
					M_16_SWAP(bk->len);
					break;
				    case B_DUPLICATE:
				    case B_OVERFLOW:
					bo = (BOVERFLOW*)bk;
					M_32_SWAP(bo->pgno);
					M_32_SWAP(bo->tlen);
					break;
				    default:
					return (__db_pgfmt(env, pg));
			    }

			    if(!pgin)
				    M_16_SWAP(inp[i]);
		    }
		    break;
		case P_IBTREE:
		    for(i = 0; i < NUM_ENT(h); i++) {
			    if(pgin)
				    M_16_SWAP(inp[i]);

			    bi = GET_BINTERNAL(dbp, h, i);
			    if((uint8*)bi >= pgend)
				    continue;

			    M_16_SWAP(bi->len);
			    M_32_SWAP(bi->pgno);
			    M_32_SWAP(bi->nrecs);

			    switch(B_TYPE(bi->type)) {
				    case B_KEYDATA:
					break;
				    case B_DUPLICATE:
				    case B_OVERFLOW:
					bo = (BOVERFLOW*)bi->data;
					M_32_SWAP(bo->pgno);
					M_32_SWAP(bo->tlen);
					break;
				    default:
					return (__db_pgfmt(env, pg));
			    }

			    if(!pgin)
				    M_16_SWAP(inp[i]);
		    }
		    break;
		case P_IRECNO:
		    for(i = 0; i < NUM_ENT(h); i++) {
			    if(pgin)
				    M_16_SWAP(inp[i]);

			    ri = GET_RINTERNAL(dbp, h, i);
			    if((uint8*)ri >= pgend)
				    continue;

			    M_32_SWAP(ri->pgno);
			    M_32_SWAP(ri->nrecs);

			    if(!pgin)
				    M_16_SWAP(inp[i]);
		    }
		    break;
		case P_HEAP:
		    for(i = 0; i <= HEAP_HIGHINDX(h); i++) {
			    if(pgin)
				    M_16_SWAP(inp[i]);
			    if(inp[i] == 0)
				    continue;

			    hh = (HEAPHDR*)P_ENTRY(dbp, h, i);
			    if((uint8*)hh >= pgend)
				    continue;
			    M_16_SWAP(hh->size);
			    if(F_ISSET(hh, HEAP_RECSPLIT)) {
				    hsh = (HEAPSPLITHDR*)hh;
				    M_32_SWAP(hsh->tsize);
				    M_32_SWAP(hsh->nextpg);
				    M_16_SWAP(hsh->nextindx);
			    }
			    else if(F_ISSET(hh, HEAP_RECBLOB)) {
				    bhdr = (HEAPBLOBHDR*)hh;
				    M_64_SWAP(bhdr->id);        /* id */
				    M_64_SWAP(bhdr->size);      /* size */
				    M_64_SWAP(bhdr->file_id);   /* file id */
			    }

			    if(!pgin)
				    M_16_SWAP(inp[i]);
		    }
		    break;
		case P_IHEAP:
		case P_INVALID:
		case P_OVERFLOW:
		case P_QAMDATA:
		    /* Nothing to do. */
		    break;
		default:
		    return (__db_pgfmt(env, pg));
	}

out:    if(!pgin) {
		/* Swap the header information. */
		M_32_SWAP(h->lsn.file);
		M_32_SWAP(h->lsn.offset);
		M_32_SWAP(h->pgno);
		if(TYPE(h) == P_HEAP) {
			M_32_SWAP(((HEAPPG*)h)->high_pgno);
			M_16_SWAP(((HEAPPG*)h)->high_indx);
			M_16_SWAP(((HEAPPG*)h)->free_indx);
		}
		else {
			M_32_SWAP(h->prev_pgno);
			M_32_SWAP(h->next_pgno);
		}
		M_16_SWAP(h->entries);
		M_16_SWAP(h->hf_offset);
	}
	return 0;
}

/*
 * __db_pageswap --
 *	Byteswap any database page.  Normally, the page to be swapped will be
 *	referenced by the "pp" argument and the pdata argument will be NULL.
 *	This function is also called by automatically generated log functions,
 *	where the page may be split into separate header and data parts.  In
 *	that case, pdata is not NULL we reconsitute
 *
 * PUBLIC: int __db_pageswap(ENV *, DB *, void *, size_t, DBT *, int);
 */
int __db_pageswap(ENV *env, DB * dbp, void * pp, size_t len, DBT * pdata, int pgin)
{
	db_pgno_t pg;
	size_t pgsize;
	void * pgcopy;
	int ret;
	uint16 hoffset;

	switch(TYPE(pp)) {
		case P_BTREEMETA: return (__bam_mswap(env, (PAGE *)pp));
		case P_HASHMETA: return (__ham_mswap(env, pp));
#ifdef HAVE_HEAP
		case P_HEAPMETA: return (__heap_mswap(env, (PAGE *)pp));
#endif
		case P_QAMMETA: return (__qam_mswap(env, (PAGE *)pp));
		case P_INVALID:
		case P_OVERFLOW:
		case P_QAMDATA:
		    /*
		     * We may have been passed an invalid page, or a queue data
		     * page, or an overflow page where fields like hoffset have a
		     * special meaning.  In that case, no swapping of the page data
		     * is required, just the fields in the page header.
		     */
		    pdata = NULL;
		    break;

		default:
		    break;
	}

	if(pgin) {
		P_32_COPYSWAP(&PGNO(pp), &pg);
		P_16_COPYSWAP(&HOFFSET(pp), &hoffset);
	}
	else {
		pg = PGNO(pp);
		hoffset = HOFFSET(pp);
	}

	if(pdata == NULL)
		ret = __db_byteswap(dbp, pg, (PAGE*)pp, len, pgin);
	else {
		pgsize = hoffset + pdata->size;
		if((ret = __os_malloc(env, pgsize, &pgcopy)) != 0)
			return ret;
		memzero(pgcopy, pgsize);
		memcpy(pgcopy, pp, len);
		memcpy((uint8*)pgcopy + hoffset, pdata->data, pdata->size);
		ret = __db_byteswap(dbp, pg, (PAGE*)pgcopy, pgsize, pgin);
		memcpy(pp, pgcopy, len);
		/*
		 * If we are swapping data to be written to the log, we can't
		 * overwrite the buffer that was passed in: it may be a pointer
		 * into a page in cache.  We set DB_DBT_APPMALLOC here so that
		 * the calling code can free the memory we allocate here.
		 */
		if(!pgin) {
			if((ret = __os_malloc(env, pdata->size, &pdata->data)) != 0) {
				__os_free(env, pgcopy);
				return ret;
			}
			F_SET(pdata, DB_DBT_APPMALLOC);
		}
		memcpy(pdata->data, (uint8*)pgcopy + hoffset, pdata->size);
		__os_free(env, pgcopy);
	}

	return ret;
}

/*
 * __db_recordswap --
 *	Byteswap any database record.
 *
 * PUBLIC: void __db_recordswap __P((uint32,
 * PUBLIC:     uint32, void *, void *, uint32));
 */
void __db_recordswap(uint32 op, uint32 size, void * hdr, void * data, uint32 pgin)
{
	BBLOB * bl;
	BKEYDATA * bk;
	BOVERFLOW * bo;
	BINTERNAL * bi;
	DBT * dbt;
	HEAPHDR * hh;
	HEAPBLOBHDR bhdr;
	HEAPSPLITHDR * hsh;
	RINTERNAL * ri;
	db_indx_t tmp;
	uint8 buf[HEAPBLOBREC_SIZE], * end, * p;
	if(size == 0)
		return;
	switch(OP_PAGE_GET(op)) {
		case P_LDUP:
		case P_LBTREE:
		case P_LRECNO:
		    bk = (BKEYDATA*)hdr;
		    switch(B_TYPE(bk->type)) {
			    case B_KEYDATA: 
					M_16_SWAP(bk->len); 
					break;
			    case B_BLOB:
				bl = (BBLOB*)bk;
				M_16_SWAP(bl->len);
				M_64_SWAP(bl->id);      /* id */
				M_64_SWAP(bl->size);    /* size */
				M_64_SWAP(bl->file_id); /* file id */
				M_64_SWAP(bl->sdb_id);  /* sdb id */
				break;
			    case B_DUPLICATE:
			    case B_OVERFLOW:
				bo = (BOVERFLOW*)hdr;
				M_32_SWAP(bo->pgno);
				M_32_SWAP(bo->tlen);
				break;
			    default:
				DB_ASSERT(NULL, bk->type != bk->type);
		    }
		    break;
		case P_IBTREE:
		    bi = (BINTERNAL*)hdr;
		    M_16_SWAP(bi->len);
		    M_32_SWAP(bi->pgno);
		    M_32_SWAP(bi->nrecs);
		    if(B_TYPE(bi->type) == B_OVERFLOW) {
			    if(data == NULL) {
				    DB_ASSERT(NULL,
					size == BINTERNAL_SIZE(BOVERFLOW_SIZE));
				    bo = (BOVERFLOW*)bi->data;
			    }
			    else
				    bo = (BOVERFLOW*)data;
			    M_32_SWAP(bo->pgno);
			    M_32_SWAP(bo->tlen);
		    }
		    break;
		case P_IRECNO:
		    ri = (RINTERNAL*)hdr;
		    M_32_SWAP(ri->pgno);
		    M_32_SWAP(ri->nrecs);
		    break;
		case P_OVERFLOW:
		    break;
		case P_HASH:
		case P_HASH_UNSORTED:
		    switch(OP_MODE_GET(op)) {
			    /* KEYDATA and DUPLICATE records do not include the header. */
			    case H_KEYDATA:
				break;
			    case H_DUPLICATE:
				p = (uint8*)hdr;
				for(end = p + size; p < end;) {
					if(pgin) {
						P_16_SWAP(p);
						memcpy(&tmp, p, sizeof(db_indx_t));
						p += sizeof(db_indx_t);
					}
					else {
						memcpy(&tmp, p, sizeof(db_indx_t));
						SWAP16(p);
					}
					p += tmp;
					SWAP16(p);
				}
				break;
			    /* These three record types include the full header. */
			    case H_OFFDUP:
				p = (uint8*)hdr;
				p += SSZ(HOFFDUP, pgno);
				SWAP32(p);              /* pgno */
				break;
			    case H_OFFPAGE:
				p = (uint8*)hdr;
				p += SSZ(HOFFPAGE, pgno);
				SWAP32(p);              /* pgno */
				SWAP32(p);              /* tlen */
				break;
			    case H_BLOB:
				p = HBLOB_ID(hdr);
				SWAP64(p);              /* id */
				SWAP64(p);              /* size */
				p = HBLOB_FILE_ID(hdr);
				SWAP64(p);              /* file id */
				SWAP64(p);              /* sdb id */
				break;
			    default:
				DB_ASSERT(NULL, op != op);
		    }
		    break;
		case P_HEAP:
		    hh = (HEAPHDR*)hdr;
		    M_16_SWAP(hh->size);
		    if(F_ISSET(hh, HEAP_RECSPLIT)) {
			    hsh = (HEAPSPLITHDR*)hdr;
			    M_32_SWAP(hsh->tsize);
			    M_32_SWAP(hsh->nextpg);
			    M_16_SWAP(hsh->nextindx);
		    }
		    else if(F_ISSET(hh, HEAP_RECBLOB)) {
			    /*
			     * Heap blob records are broken into two parts when
			     * logged, the shared header and the part that is
			     * unique to blob records, which is stored in the
			     * log data field.
			     */
			    if(data != NULL) {
				    dbt = NULL;
				    if(pgin) {
					    dbt = (DBT *)data;
					    memcpy(buf + sizeof(HEAPHDR),
						dbt->data, HEAPBLOBREC_DSIZE);
				    }
				    else {
					    memcpy(buf + sizeof(HEAPHDR), data, HEAPBLOBREC_DSIZE);
				    }
				    memcpy(&bhdr, buf, HEAPBLOBREC_SIZE);
				    M_64_SWAP(bhdr.id);         /* id */
				    M_64_SWAP(bhdr.size);       /* size */
				    M_64_SWAP(bhdr.file_id);    /* file id */
				    memcpy(buf, &bhdr, HEAPBLOBREC_SIZE);
				    if(pgin) {
					    memcpy(dbt->data,
						HEAPBLOBREC_DATA(buf),
						HEAPBLOBREC_DSIZE);
				    }
				    else {
					    memcpy(data,
						HEAPBLOBREC_DATA(buf),
						HEAPBLOBREC_DSIZE);
				    }
			    }
			    break;
		    }
		    break;
		default:
		    DB_ASSERT(NULL, op != op);
	}
}

/*
 * __db_swap --
 *	Swap the byte order for a page. Used by __db_page_pass.
 */
static int __db_swap(DB *dbp, char * real_name, uint32 flags, DB_FH * fhp, PAGE * h, int * dirtyp)
{
	COMPQUIET(real_name, NULL);
	COMPQUIET(flags, 0);
	COMPQUIET(fhp, NULL);
	*dirtyp = 1;
	return __db_pageswap(dbp->env, dbp, h, dbp->pgsize, NULL, !F_ISSET(dbp, DB_AM_SWAP));
}

static int (*const func_swap[P_PAGETYPE_MAX])(DB *, char *, uint32, DB_FH *, PAGE *, int *) = {
	NULL,                   /* P_INVALID */
	__db_swap,              /* __P_DUPLICATE */
	__db_swap,              /* P_HASH_UNSORTED */
	__db_swap,              /* P_IBTREE */
	__db_swap,              /* P_IRECNO */
	__db_swap,              /* P_LBTREE */
	__db_swap,              /* P_LRECNO */
	__db_swap,              /* P_OVERFLOW */
	__db_swap,              /* P_HASHMETA */
	__db_swap,              /* P_BTREEMETA */
	__db_swap,              /* P_QAMMETA */
	__db_swap,              /* P_QAMDATA */
	__db_swap,              /* P_LDUP */
	__db_swap,              /* P_HASH */
	__db_swap,              /* P_HEAPMETA */
	__db_swap,              /* P_HEAP */
	__db_swap,              /* P_IHEAP */
};

/*
 * __db_convert_pp --
 *	DB->convert pre/post processing.
 *
 * PUBLIC: int __db_convert_pp __P((DB *, const char *, uint32));
 */
int __db_convert_pp(DB *dbp, const char * fname, uint32 lorder)
{
	DB_THREAD_INFO * ip;
	int ret;
	ENV * env = dbp->env;
	ENV_ENTER(env, ip);
	ret = __db_convert(dbp, fname, lorder);
#ifdef HAVE_SLICES
	if(ret == 0)
		ret = __db_slice_process(dbp, fname, lorder, __db_convert_pp, "db_convert");
#endif
	ENV_LEAVE(env, ip);
	return ret;
}

/*
 * __db_convert_extent --
 *	Convert the byte order of each database extent (a queue or partition
 *      extent).
 */
static int __db_convert_extent(ENV *env, const char * fname, uint32 pagesize, uint32 flags)
{
	DB * dbp = 0;
	DB_FH * fhp = 0;
	char * real_name;
	int ret = 0;
	int t_ret = 0;
	/* Get the real backing file name. */
	if((ret = __db_appname(env, DB_APP_DATA, fname, NULL, &real_name)) != 0)
		return ret;
	/* Open the file. */
	if((ret = __os_open(env, real_name, 0, 0, 0, &fhp)) != 0) {
		__db_err(env, ret, "%s", real_name);
		goto err;
	}

	if((ret = __db_create_internal(&dbp, env, 0)) != 0)
		goto err;

	dbp->pgsize = pagesize;
	dbp->flags = flags;
	if((ret = __db_page_pass(dbp, real_name, 0, func_swap, fhp, DB_CONVERT)) != 0)
		goto err;
	ret = __os_fsync(env, fhp);
err:
	if(fhp != NULL && (t_ret = __os_closehandle(env, fhp)) != 0 && ret == 0)
		ret = t_ret;
	if(dbp != NULL && (t_ret = __db_close(dbp, NULL, 0) != 0) && ret == 0)
		ret = t_ret;
	__os_free(env, real_name);
	return ret;
}

static int __db_convert_extent_names(DB *dbp, DBMETA * mbuf, char * fname, char *** namelistp)
{
	ENV * env = dbp->env;
	*namelistp = NULL;
	switch(mbuf->magic) {
		case DB_BTREEMAGIC:
		case DB_HASHMAGIC:
#ifdef HAVE_PARTITION
		    if(dbp->p_internal != NULL) {
			    return __partition_extent_names(dbp, fname, namelistp);
		    }
#endif
		    break;
		case DB_QAMMAGIC:
		    if(F_ISSET(dbp, DB_AM_CHKSUM) && ((QMETA*)mbuf)->page_ext != 0) {
			    return __qam_extent_names(env, fname, namelistp);
		    }
		    break;
		case DB_HEAPMAGIC:
		default:
		    break;
	}

	return 0;
}

/*
 * __db_convert --
 *      Convert the byte order of a database.
 *
 * PUBLIC: int __db_convert __P((DB *, const char *, uint32));
 */
int __db_convert(DB *dbp, const char * fname, uint32 lorder)
{
	uint8 mbuf[DBMETASIZE];
	char * real_name = 0;
	char ** extent_names = 0;
	char ** ename;
	size_t len = 0;
	uint32 db_order;
	int t_ret = 0;
	int ret = 0;
	ENV * env = dbp->env;
	DB_FH * fhp = NULL;
	/* Get the real backing file name. */
	if((ret = __db_appname(env, DB_APP_DATA, fname, NULL, &real_name)) != 0)
		return ret;
	/* Open the file. */
	if((ret = __os_open(env, real_name, 0, 0, 0, &fhp)) != 0) {
		__db_err(env, ret, "%s", real_name);
		goto err;
	}
	/* Read the metadata page. */
	if((ret = __fop_read_meta(env, real_name, mbuf, sizeof(mbuf), fhp, 0, &len)) != 0)
		goto err;
	switch(__db_needswap(((DBMETA*)mbuf)->magic)) {
		case 0:
		    db_order = __db_isbigendian() ? 4321 : 1234;
		    F_SET(dbp, DB_AM_SWAP);
		    break;
		case DB_SWAPBYTES:
		    db_order = __db_isbigendian() ? 1234 : 4321;
		    M_32_SWAP(((DBMETA*)mbuf)->magic);
		    M_32_SWAP(((DBMETA*)mbuf)->pagesize);
		    F_CLR(dbp, DB_AM_SWAP);
		    break;
		default:
		    ret = USR_ERR(env, EINVAL);
		    goto err;
	}
	if(db_order != lorder) {
		dbp->pgsize = ((DBMETA*)mbuf)->pagesize;
		if(FLD_ISSET(((DBMETA*)mbuf)->metaflags, DBMETA_CHKSUM))
			F_SET(dbp, DB_AM_CHKSUM);
		if(((DBMETA*)mbuf)->encrypt_alg != 0) {
			if(!CRYPTO_ON(dbp->env)) {
				ret = USR_ERR(env, EINVAL);
				__db_errx(env, DB_STR("0667", "Attempt to convert an encrypted database without providing a password."));
				goto err;
			}
			F_SET(dbp, DB_AM_ENCRYPT);
		}
		if((ret = __db_page_pass(dbp, real_name, 0, func_swap, fhp, DB_CONVERT)) != 0)
			goto err;
		ret = __os_fsync(env, fhp);
		if((ret = __db_convert_extent_names(dbp, (DBMETA*)mbuf, (char*)fname, &extent_names)) != 0)
			goto err;
		if(extent_names != NULL) {
			for(ename = extent_names; *ename != NULL; ename++) {
				if((t_ret = __db_convert_extent(env, *ename, dbp->pgsize, dbp->flags)) != 0 && ret == 0)
					ret = t_ret;
			}
		}
	}
err:    
	if(fhp != NULL && (t_ret = __os_closehandle(env, fhp)) != 0 && ret == 0)
		ret = t_ret;
	if(real_name != NULL)
		__os_free(env, real_name);
	if(extent_names != NULL)
		__os_free(env, extent_names);
	return ret;
}
