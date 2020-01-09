/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
#include "dbinc/lock.h"
#include "dbinc/log.h"

static int __lock_freelocker_int(DB_LOCKTAB *, DB_LOCKREGION *, DB_LOCKER *, int);
/*
 * __lock_id_pp -- DB_ENV->lock_id pre/post processing.
 */
int __lock_id_pp(DB_ENV *dbenv, uint32 * idp)
{
	DB_THREAD_INFO * ip;
	int ret;
	ENV * env = dbenv->env;
	ENV_REQUIRES_CONFIG(env, env->lk_handle, "DB_ENV->lock_id", DB_INIT_LOCK);
	ENV_ENTER(env, ip);
	REPLICATION_WRAP(env, (__lock_id(env, idp, NULL)), 0, ret);
	ENV_LEAVE(env, ip);
	return ret;
}
/*
 * __lock_id --
 *	Allocate a new lock id as well as a locker struct to hold it. If we wrap
 *	around then we find the minimum currently in use and make sure we can
 *	stay below that. This is similar to __txn_begin_int's code to recover txn ids.
 */
int FASTCALL __lock_id(ENV *env, uint32 * idp, DB_LOCKER ** lkp)
{
	DB_LOCKER * lk = 0;
	uint32 * ids;
	int nids;
	int ret = 0;
	DB_LOCKTAB * lt = env->lk_handle;
	DB_LOCKREGION * region = (DB_LOCKREGION *)lt->reginfo.primary;
	uint32 id = DB_LOCK_INVALIDID;
	LOCK_LOCKERS(env, region);
	/*
	 * Our current valid range can span the maximum valid value, so check
	 * for it and wrap manually.
	 */
	if(region->lock_id == DB_LOCK_MAXID && region->cur_maxid != DB_LOCK_MAXID)
		region->lock_id = DB_LOCK_INVALIDID;
	if(region->lock_id == region->cur_maxid) {
		if((ret = __os_malloc(env, sizeof(uint32) * region->nlockers, &ids)) != 0)
			goto err;
		nids = 0;
		SH_TAILQ_FOREACH(lk, &region->lockers, ulinks, __db_locker)
		ids[nids++] = lk->id;
		region->lock_id = DB_LOCK_INVALIDID;
		region->cur_maxid = DB_LOCK_MAXID;
		if(nids != 0)
			__db_idspace(ids, nids, &region->lock_id, &region->cur_maxid);
		__os_free(env, ids);
	}
	id = ++region->lock_id;
	/* Allocate a locker for this id. */
	ret = __lock_getlocker_int(lt, id, 1, NULL, &lk);
err:    
	UNLOCK_LOCKERS(env, region);
	if(idp != NULL)
		*idp = id;
	if(lkp != NULL)
		*lkp = lk;
	return ret;
}
/*
 * __lock_set_thread_id -- Set the thread_id in an existing locker.
 */
void __lock_set_thread_id(void * lref_arg, pid_t pid, db_threadid_t tid)
{
	DB_LOCKER * lref = (DB_LOCKER *)lref_arg;
	lref->pid = pid;
	lref->tid = tid;
}
/*
 * __lock_id_free_pp -- ENV->lock_id_free pre/post processing.
 */
int __lock_id_free_pp(DB_ENV *dbenv, uint32 id)
{
	DB_LOCKER * sh_locker;
	DB_LOCKREGION * region;
	DB_LOCKTAB * lt;
	DB_THREAD_INFO * ip;
	ENV * env;
	int handle_check, ret, t_ret;
	env = dbenv->env;
	ENV_REQUIRES_CONFIG(env, env->lk_handle, "DB_ENV->lock_id_free", DB_INIT_LOCK);
	ENV_ENTER(env, ip);
	/* Check for replication block. */
	handle_check = IS_ENV_REPLICATED(env);
	if(handle_check && (ret = __env_rep_enter(env, 0)) != 0) {
		handle_check = 0;
		goto err;
	}
	lt = env->lk_handle;
	region = (DB_LOCKREGION *)lt->reginfo.primary;
	LOCK_LOCKERS(env, region);
	if((ret =
	    __lock_getlocker_int(env->lk_handle,
	    id, 0, NULL, &sh_locker)) == 0) {
		if(sh_locker != NULL)
			ret = __lock_freelocker_int(lt, region, sh_locker, 1);
		else {
			ret = USR_ERR(env, EINVAL);
			__db_errx(env, DB_STR_A("2045", "Unknown locker id: %lx", "%lx"), (u_long)id);
		}
	}
	UNLOCK_LOCKERS(env, region);
	if(handle_check && (t_ret = __env_db_rep_exit(env)) != 0 && ret == 0)
		ret = t_ret;
err:    
	ENV_LEAVE(env, ip);
	return ret;
}
// 
// __lock_id_free -- Free a locker id.
// 
int FASTCALL __lock_id_free(ENV *env, DB_LOCKER * sh_locker)
{
	DB_LOCKER locker;
	DB_MSGBUF mb;
	DB_LOCKTAB * lt = env->lk_handle;
	DB_LOCKREGION * region = (DB_LOCKREGION *)lt->reginfo.primary;
	int ret = 0;
	if(sh_locker->nlocks != 0) {
		locker = *sh_locker;
		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR_A("2046", "Locker %d still has %d locks", "%d %d"), locker.id, locker.nlocks);
		DB_MSGBUF_INIT(&mb);
		(void)__lock_dump_locker(env, &mb, lt, sh_locker);
		DB_MSGBUF_FLUSH(env, &mb);
		goto err;
	}
	LOCK_LOCKERS(env, region);
	ret = __lock_freelocker_int(lt, region, sh_locker, 1);
	UNLOCK_LOCKERS(env, region);
err:    
	return ret;
}
// 
// __lock_id_set -- Set the current locker ID and current maximum unused ID (for testing purposes only).
// 
int __lock_id_set(ENV *env, uint32 cur_id, uint32 max_id)
{
	DB_LOCKREGION * region;
	DB_LOCKTAB * lt;
	ENV_REQUIRES_CONFIG(env, env->lk_handle, "lock_id_set", DB_INIT_LOCK);
	lt = env->lk_handle;
	region = (DB_LOCKREGION *)lt->reginfo.primary;
	region->lock_id = cur_id;
	region->cur_maxid = max_id;
	return 0;
}
/*
 * __lock_getlocker,__lock_getlocker_int --
 *	Get a locker in the locker hash table.  The create parameter indicates
 * whether the locker should be created if it doesn't exist in the table. If
 * there's a matching locker cached in the thread info, use that without
 * locking.
 *
 * The internal version does not check the thread info cache; it must be called
 * with the locker mutex locked.
 *
 * PUBLIC: int __lock_getlocker __P((DB_LOCKTAB *,
 * PUBLIC:     uint32, int, DB_LOCKER **));
 * PUBLIC: int __lock_getlocker_int __P((DB_LOCKTAB *,
 * PUBLIC:     uint32, int, DB_THREAD_INFO *, DB_LOCKER **));
 */
int __lock_getlocker(DB_LOCKTAB *lt, uint32 locker, int create, DB_LOCKER ** retp)
{
	DB_LOCKREGION * region;
	DB_THREAD_INFO * ip;
	ENV * env;
	int ret;
	COMPQUIET(region, NULL);
	env = lt->env;
	region = (DB_LOCKREGION *)lt->reginfo.primary;
	ENV_GET_THREAD_INFO(env, ip);
	/* Check to see if the locker is already in the thread info */
	if(ip != NULL && ip->dbth_local_locker != INVALID_ROFF) {
		*retp = (DB_LOCKER*)R_ADDR(&lt->reginfo, ip->dbth_local_locker);
		if((*retp)->id == locker) {
			DB_ASSERT(env, !F_ISSET(*retp, DB_LOCKER_FREE));
#ifdef HAVE_STATISTICS
			region->stat.st_nlockers_hit++;
#endif
			return 0;
		}
	}
	LOCK_LOCKERS(env, region);
	ret = __lock_getlocker_int(lt, locker, create, ip, retp);
	UNLOCK_LOCKERS(env, region);
	return ret;
}

int __lock_getlocker_int(DB_LOCKTAB *lt, uint32 locker, int create, DB_THREAD_INFO * ip, DB_LOCKER ** retp)
{
	DB_LOCKER * sh_locker;
	DB_LOCKREGION * region;
#ifdef DIAGNOSTIC
	DB_THREAD_INFO * diag;
#endif
	ENV * env;
	uint32 i, indx, nlockers;
	int ret;
	env = lt->env;
	region = (DB_LOCKREGION *)lt->reginfo.primary;
	MUTEX_REQUIRED(env, region->mtx_lockers);
	LOCKER_HASH(lt, region, locker, indx);
	/*
	 * If we find the locker, then we can just return it.  If we don't find
	 * the locker, then we need to create it.
	 */
	SH_TAILQ_FOREACH(sh_locker, &lt->locker_tab[indx], links, __db_locker)
	if(sh_locker->id == locker)
		break;

	if(sh_locker == NULL && create) {
		/* Can we reuse a locker struct cached in the thread info? */
		if(ip != NULL && ip->dbth_local_locker != INVALID_ROFF &&
		    (sh_locker = (DB_LOCKER*)R_ADDR(&lt->reginfo,
		    ip->dbth_local_locker))->id == DB_LOCK_INVALIDID) {
			DB_ASSERT(env, !F_ISSET(sh_locker, DB_LOCKER_FREE));
#ifdef HAVE_STATISTICS
			region->stat.st_nlockers_reused++;
#endif
		}
		else {
			/* Create new locker and insert it into hash table. */
			if((sh_locker = SH_TAILQ_FIRST(&region->free_lockers, __db_locker)) == NULL) {
				if(region->stat.st_maxlockers != 0 && region->stat.st_maxlockers <= region->stat.st_lockers)
					return (__lock_nomem(env, "locker entries"));
				nlockers = region->stat.st_lockers >> 2;
				/* Just in case. */
				if(nlockers == 0)
					nlockers = 1;
				if(region->stat.st_maxlockers != 0 && region->stat.st_maxlockers < region->stat.st_lockers + nlockers)
					nlockers = region->stat.st_maxlockers - region->stat.st_lockers;
				/*
				 * Don't hold lockers when getting the region,
				 * we could deadlock.  When creating a locker
				 * there is no race since the id allocation
				 * is synchronized.
				 */
				UNLOCK_LOCKERS(env, region);
				LOCK_REGION_LOCK(env);
				/*
				 * If the max memory is not sized for max
				 * objects, allocate as much as possible.
				 */
				F_SET(&lt->reginfo, REGION_TRACKED);
				while(__env_alloc(&lt->reginfo, nlockers *
				    sizeof(struct __db_locker),
				    &sh_locker) != 0) {
					nlockers >>= 1;
					if(nlockers == 0)
						break;
				}
				F_CLR(&lt->reginfo, REGION_TRACKED);
				LOCK_REGION_UNLOCK(lt->env);
				LOCK_LOCKERS(env, region);
				for(i = 0; i < nlockers; i++) {
					SH_TAILQ_INSERT_HEAD(&region->free_lockers, sh_locker, links, __db_locker);
					sh_locker->mtx_locker = MUTEX_INVALID;
#ifdef DIAGNOSTIC
					sh_locker->prev_locker = INVALID_ROFF;
#endif
					sh_locker++;
				}
				if(nlockers == 0)
					return (__lock_nomem(env, "locker entries"));
				region->stat.st_lockers += nlockers;
				sh_locker = SH_TAILQ_FIRST(&region->free_lockers, __db_locker);
			}
			SH_TAILQ_REMOVE(&region->free_lockers, sh_locker, links, __db_locker);
		}
		F_CLR(sh_locker, DB_LOCKER_FREE);
		if(sh_locker->mtx_locker == MUTEX_INVALID) {
			if((ret = __mutex_alloc(env, MTX_LOGICAL_LOCK, DB_MUTEX_LOGICAL_LOCK | DB_MUTEX_SELF_BLOCK, &sh_locker->mtx_locker)) != 0) {
				SH_TAILQ_INSERT_HEAD(&region->free_lockers, sh_locker, links, __db_locker);
				return ret;
			}
			MUTEX_LOCK_NO_CTR(env, sh_locker->mtx_locker);
		}
		++region->nlockers;
#ifdef HAVE_STATISTICS
		STAT_PERFMON2(env, lock, nlockers, region->nlockers, locker);
		if(region->nlockers > region->stat.st_maxnlockers)
			STAT_SET(env, lock, maxnlockers, region->stat.st_maxnlockers, region->nlockers, locker);
#endif
		sh_locker->id = locker;
		env->dbenv->thread_id(env->dbenv, &sh_locker->pid, &sh_locker->tid);
		sh_locker->dd_id = 0;
		sh_locker->master_locker = INVALID_ROFF;
		sh_locker->parent_locker = INVALID_ROFF;
		SH_LIST_INIT(&sh_locker->child_locker);
		sh_locker->flags = 0;
		SH_LIST_INIT(&sh_locker->heldby);
		sh_locker->nlocks = 0;
		sh_locker->nwrites = 0;
		sh_locker->priority = DB_LOCK_DEFPRIORITY;
		sh_locker->lk_timeout = 0;
		timespecclear(&sh_locker->tx_expire);
		timespecclear(&sh_locker->lk_expire);
		SH_TAILQ_INSERT_HEAD(&lt->locker_tab[indx], sh_locker, links, __db_locker);
		SH_TAILQ_INSERT_HEAD(&region->lockers, sh_locker, ulinks, __db_locker);
		if(ip != NULL && ip->dbth_local_locker == INVALID_ROFF)
			ip->dbth_local_locker = R_OFFSET(&lt->reginfo, sh_locker);
#ifdef DIAGNOSTIC
		/*
		 * __db_has_pagelock checks for proper locking by dbth_locker.
		 */
		if((diag = ip) == NULL)
			ENV_GET_THREAD_INFO(env, diag);
		if(diag != NULL) {
			sh_locker->prev_locker = diag->dbth_locker;
			diag->dbth_locker = R_OFFSET(&lt->reginfo, sh_locker);
		}
#endif
	}
	*retp = sh_locker;
	return 0;
}
/*
 * __lock_addfamilylocker
 *	Put a locker entry in for a child transaction.
 *
 * PUBLIC: int __lock_addfamilylocker(ENV *, uint32, uint32, uint32);
 */
int __lock_addfamilylocker(ENV *env, uint32 pid, uint32 id, uint32 is_family)
{
	DB_LOCKER * lockerp, * mlockerp;
	DB_LOCKREGION * region;
	DB_LOCKTAB * lt;
	int ret;
	COMPQUIET(region, NULL);
	lt = env->lk_handle;
	region = (DB_LOCKREGION *)lt->reginfo.primary;
	LOCK_LOCKERS(env, region);
	/* get/create the  parent locker info */
	if((ret = __lock_getlocker_int(lt, pid, 1, NULL, &mlockerp)) != 0)
		goto err;
	/*
	 * We assume that only one thread can manipulate
	 * a single transaction family.
	 * Therefore the master locker cannot go away while
	 * we manipulate it, nor can another child in the
	 * family be created at the same time.
	 */
	if((ret = __lock_getlocker_int(lt, id, 1, NULL, &lockerp)) != 0)
		goto err;
	/* Point to our parent. */
	lockerp->parent_locker = R_OFFSET(&lt->reginfo, mlockerp);
	/* See if this locker is the family master. */
	if(mlockerp->master_locker == INVALID_ROFF)
		lockerp->master_locker = R_OFFSET(&lt->reginfo, mlockerp);
	else {
		lockerp->master_locker = mlockerp->master_locker;
		mlockerp = (DB_LOCKER *)R_ADDR(&lt->reginfo, mlockerp->master_locker);
	}
	/*
	 * Set the family locker flag, so it is possible to distinguish
	 * between locks held by subtransactions and those with compatible
	 * lockers.
	 */
	if(is_family)
		F_SET(mlockerp, DB_LOCKER_FAMILY_LOCKER);
	/*
	 * Link the child at the head of the master's list.
	 * The guess is when looking for deadlock that
	 * the most recent child is the one that's blocked.
	 */
	SH_LIST_INSERT_HEAD(&mlockerp->child_locker, lockerp, child_link, __db_locker);
err:    
	UNLOCK_LOCKERS(env, region);
	return ret;
}
/*
 * __lock_freelocker_int --
 *      Common code for deleting a locker; must be called with the
 *	lockers mutex locked.
 */
static int __lock_freelocker_int(DB_LOCKTAB *lt, DB_LOCKREGION * region, DB_LOCKER * sh_locker, int reallyfree)
{
	DB_MSGBUF mb;
	DB_THREAD_INFO * ip;
	uint32 indx;
	int ret;
	ENV * env = lt->env;
	if(!SH_LIST_EMPTY(&sh_locker->heldby)) {
		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR_A("2060", "Freeing locker %x with locks", "%x"), sh_locker->id);
		DB_MSGBUF_INIT(&mb);
		(void)__lock_dump_locker(env, &mb, lt, sh_locker);
		DB_MSGBUF_FLUSH(env, &mb);
		return ret;
	}
	/* If this is part of a family, we must fix up its links. */
	if(sh_locker->master_locker != INVALID_ROFF) {
		SH_LIST_REMOVE(sh_locker, child_link, __db_locker);
		sh_locker->master_locker = INVALID_ROFF;
	}
	sh_locker->parent_locker = INVALID_ROFF;
	if(reallyfree) {
		LOCKER_HASH(lt, region, sh_locker->id, indx);
		SH_TAILQ_REMOVE(&lt->locker_tab[indx], sh_locker, links, __db_locker);
		SH_TAILQ_REMOVE(&region->lockers, sh_locker, ulinks, __db_locker);
		region->nlockers--;
		STAT_PERFMON2(env, lock, nlockers, region->nlockers, sh_locker->id);
		/*
		 * If this locker is cached in the thread info, zero the id and
		 * leave it allocated. Otherwise, put it back on the free list.
		 */
		ENV_GET_THREAD_INFO(env, ip);
		if(ip != NULL && ip->dbth_local_locker == R_OFFSET(&lt->reginfo, sh_locker)) {
			DB_ASSERT(env, MUTEX_IS_BUSY(env, sh_locker->mtx_locker));
			sh_locker->id = DB_LOCK_INVALIDID;
		}
		else {
			if(sh_locker->mtx_locker != MUTEX_INVALID && (ret = __mutex_free(env, &sh_locker->mtx_locker)) != 0)
				return ret;
			F_SET(sh_locker, DB_LOCKER_FREE);
			SH_TAILQ_INSERT_HEAD(&region->free_lockers, sh_locker, links, __db_locker);
		}
	}
	return 0;
}
/*
 * __lock_freelocker
 *	Remove a locker its family from the hash table.
 *
 * This must be called without the lockers mutex locked.
 *
 * PUBLIC: int __lock_freelocker(DB_LOCKTAB *, DB_LOCKER *);
 */
int __lock_freelocker(DB_LOCKTAB *lt, DB_LOCKER * sh_locker)
{
	int ret;
	DB_LOCKREGION * region = (DB_LOCKREGION *)lt->reginfo.primary;
	ENV * env = lt->env;
	if(sh_locker == NULL)
		return 0;
	LOCK_LOCKERS(env, region);
	ret = __lock_freelocker_int(lt, region, sh_locker, 1);
	UNLOCK_LOCKERS(env, region);
	return ret;
}
/*
 * __lock_familyremove
 *	Remove a locker from its family.
 *
 * This must be called without the locker bucket locked.
 *
 * PUBLIC: int __lock_familyremove  __P((DB_LOCKTAB *, DB_LOCKER *));
 */
int __lock_familyremove(DB_LOCKTAB *lt, DB_LOCKER * sh_locker)
{
	DB_LOCKREGION * region;
	ENV * env;
	int ret;
	region = (DB_LOCKREGION *)lt->reginfo.primary;
	env = lt->env;
	LOCK_LOCKERS(env, region);
	ret = __lock_freelocker_int(lt, region, sh_locker, 0);
	UNLOCK_LOCKERS(env, region);
	return ret;
}
/*
 * __lock_local_locker_invalidate --
 *	Search the thread info table's cached lockers and discard any reference
 *	to this mutex.
 *
 * PUBLIC: int __lock_local_locker_invalidate  __P((ENV *, db_mutex_t));
 */
int __lock_local_locker_invalidate(ENV *env, db_mutex_t mutex)
{
	DB_HASHTAB * htab;
	DB_LOCKER * locker;
	DB_THREAD_INFO * ip;
	uint32 i;
	char buf[DB_THREADID_STRLEN];

	htab = env->thr_hashtab;
	for(i = 0; i < env->thr_nbucket; i++) {
		SH_TAILQ_FOREACH(ip, &htab[i], dbth_links, __db_thread_info) {
			if(ip->dbth_local_locker == INVALID_ROFF)
				continue;
			locker = (DB_LOCKER*)R_ADDR(&env->lk_handle->reginfo,
				ip->dbth_local_locker);
			if(locker->mtx_locker == mutex) {
				__db_msg(env,
				    DB_STR_A("2061", "Removing cached locker mutex %lu reference by %s", "%lu %s"),
				    (u_long)mutex,
				    env->dbenv->thread_id_string(env->dbenv,
				    locker->pid, locker->tid, buf));
				locker->mtx_locker = MUTEX_INVALID;
				return 0;
			}
		}
	}
	return 0;
}
