/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * __log_env_create --
 *	Log specific initialization of the DB_ENV structure.
 */
int __log_env_create(DB_ENV * dbenv)
{
	/*
	 * !!!
	 * Our caller has not yet had the opportunity to reset the panic
	 * state or turn off mutex locking, and so we can neither check
	 * the panic state or acquire a mutex in the DB_ENV create path.
	 */
	dbenv->lg_bsize = 0;
	dbenv->lg_regionmax = 0;
	return 0;
}
/*
 * __log_env_destroy --
 *	Log specific destruction of the DB_ENV structure.
 */
void __log_env_destroy(DB_ENV * dbenv)
{
	COMPQUIET(dbenv, 0);
}

int __log_get_lg_bsize(DB_ENV * dbenv, uint32 * lg_bsizep)
{
	ENV * env = dbenv->env;
	ENV_NOT_CONFIGURED(env, env->lg_handle, "DB_ENV->get_lg_bsize", DB_INIT_LOG);
	if(LOGGING_ON(env)) {
		/* Cannot be set after open, no lock required to read. */
		*lg_bsizep = ((LOG *)env->lg_handle->reginfo.primary)->buffer_size;
	}
	else
		*lg_bsizep = dbenv->lg_bsize;
	return 0;
}
/*
 * __log_set_lg_bsize --
 *	DB_ENV->set_lg_bsize.
 */
int __log_set_lg_bsize(DB_ENV * dbenv, uint32 lg_bsize)
{
	ENV * env = dbenv->env;
	ENV_ILLEGAL_AFTER_OPEN(env, "DB_ENV->set_lg_bsize");
	dbenv->lg_bsize = lg_bsize;
	return 0;
}

int __log_get_lg_filemode(DB_ENV * dbenv, int * lg_modep)
{
	DB_LOG * dblp;
	DB_THREAD_INFO * ip;
	ENV * env = dbenv->env;
	ENV_NOT_CONFIGURED(env, env->lg_handle, "DB_ENV->get_lg_filemode", DB_INIT_LOG);
	if(LOGGING_ON(env)) {
		dblp = env->lg_handle;
		ENV_ENTER(env, ip);
		LOG_SYSTEM_LOCK(env);
		*lg_modep = ((LOG *)dblp->reginfo.primary)->filemode;
		LOG_SYSTEM_UNLOCK(env);
		ENV_LEAVE(env, ip);
	}
	else
		*lg_modep = dbenv->lg_filemode;
	return 0;
}
/*
 * __log_set_lg_filemode --
 *	DB_ENV->set_lg_filemode.
 */
int __log_set_lg_filemode(DB_ENV * dbenv, int lg_mode)
{
	DB_LOG * dblp;
	DB_THREAD_INFO * ip;
	LOG * lp;
	ENV * env = dbenv->env;
	ENV_NOT_CONFIGURED(env, env->lg_handle, "DB_ENV->set_lg_filemode", DB_INIT_LOG);
	if(LOGGING_ON(env)) {
		dblp = env->lg_handle;
		lp = (LOG *)dblp->reginfo.primary;
		ENV_ENTER(env, ip);
		LOG_SYSTEM_LOCK(env);
		lp->filemode = lg_mode;
		LOG_SYSTEM_UNLOCK(env);
		ENV_LEAVE(env, ip);
	}
	else
		dbenv->lg_filemode = lg_mode;
	return 0;
}

int __log_get_lg_max(DB_ENV * dbenv, uint32 * lg_maxp)
{
	DB_LOG * dblp;
	DB_THREAD_INFO * ip;
	ENV * env = dbenv->env;
	ENV_NOT_CONFIGURED(env, env->lg_handle, "DB_ENV->get_lg_max", DB_INIT_LOG);
	if(LOGGING_ON(env)) {
		dblp = env->lg_handle;
		ENV_ENTER(env, ip);
		LOG_SYSTEM_LOCK(env);
		*lg_maxp = ((LOG *)dblp->reginfo.primary)->log_nsize;
		LOG_SYSTEM_UNLOCK(env);
		ENV_LEAVE(env, ip);
	}
	else
		*lg_maxp = dbenv->lg_size;
	return 0;
}
/*
 * __log_set_lg_max --
 *	DB_ENV->set_lg_max.
 */
int __log_set_lg_max(DB_ENV * dbenv, uint32 lg_max)
{
	DB_LOG * dblp;
	DB_THREAD_INFO * ip;
	LOG * lp;
	int ret = 0;
	ENV * env = dbenv->env;
	ENV_NOT_CONFIGURED(env, env->lg_handle, "DB_ENV->set_lg_max", DB_INIT_LOG);
	if(LOGGING_ON(env)) {
		dblp = env->lg_handle;
		lp = (LOG *)dblp->reginfo.primary;
		ENV_ENTER(env, ip);
		if((ret = __log_check_sizes(env, lg_max, 0)) == 0) {
			LOG_SYSTEM_LOCK(env);
			lp->log_nsize = lg_max;
			LOG_SYSTEM_UNLOCK(env);
		}
		ENV_LEAVE(env, ip);
	}
	else
		dbenv->lg_size = lg_max;
	return ret;
}

int __log_get_lg_regionmax(DB_ENV * dbenv, uint32 * lg_regionmaxp)
{
	ENV * env = dbenv->env;
	ENV_NOT_CONFIGURED(env, env->lg_handle, "DB_ENV->get_lg_regionmax", DB_INIT_LOG);
	if(LOGGING_ON(env)) {
		/* Cannot be set after open, no lock required to read. */
		*lg_regionmaxp = ((LOG *)env->lg_handle->reginfo.primary)->regionmax;
	}
	else
		*lg_regionmaxp = dbenv->lg_regionmax;
	return 0;
}
/*
 * __log_set_lg_regionmax --
 *	DB_ENV->set_lg_regionmax.
 */
int __log_set_lg_regionmax(DB_ENV * dbenv, uint32 lg_regionmax)
{
	ENV * env = dbenv->env;
	ENV_ILLEGAL_AFTER_OPEN(env, "DB_ENV->set_lg_regionmax");
	/* Let's not be silly. */
	if(lg_regionmax != 0 && lg_regionmax < LG_BASE_REGION_SIZE) {
		__db_errx(env, DB_STR_A("2569", "log region size must be >= %d", "%d"), LG_BASE_REGION_SIZE);
		return EINVAL;
	}
	else {
		dbenv->lg_regionmax = lg_regionmax;
		return 0;
	}
}

int __log_get_lg_dir(DB_ENV * dbenv, const char ** dirp)
{
	*dirp = dbenv->db_log_dir;
	return 0;
}

int __log_set_lg_dir(DB_ENV * dbenv, const char * dir)
{
	ENV * env = dbenv->env;
	__os_free(env, dbenv->db_log_dir);
	return __os_strdup(env, dir, &dbenv->db_log_dir);
}

void __log_get_flags(DB_ENV * dbenv, uint32 * flagsp)
{
	ENV * env = dbenv->env;
	DB_LOG * dblp = env->lg_handle;
	if(dblp) {
		LOG * lp = (LOG *)dblp->reginfo.primary;
		uint32 flags = *flagsp;
		SETFLAG(flags, DB_LOG_AUTO_REMOVE, lp->db_log_autoremove);
		SETFLAG(flags, DB_LOG_IN_MEMORY, lp->db_log_inmemory);
		SETFLAG(flags, DB_LOG_NOSYNC, lp->nosync); // @bdb_v6223
		*flagsp = flags;
	}
}

void __log_set_flags(ENV * env, uint32 flags, int on)
{
	DB_LOG * dblp = env->lg_handle;
	if(dblp) {
		LOG * lp = (LOG *)dblp->reginfo.primary;
		if(LF_ISSET(DB_LOG_AUTO_REMOVE))
			lp->db_log_autoremove = BIN(on);
		if(LF_ISSET(DB_LOG_IN_MEMORY))
			lp->db_log_inmemory = BIN(on);
		// @bdb_v6223 {
		if(LF_ISSET(DB_LOG_NOSYNC))
			lp->nosync = BIN(on);
		// } @bdb_v6223 
	}
}

/*
 * List of flags we can handle here.  DB_LOG_INMEMORY must be
 * processed before creating the region, leave it out for now.
 */
#undef  OK_FLAGS
#define OK_FLAGS (DB_LOG_AUTO_REMOVE|DB_LOG_DIRECT|DB_LOG_DSYNC|DB_LOG_IN_MEMORY|DB_LOG_ZERO)
static const FLAG_MAP LogMap[] = {
	{ DB_LOG_AUTO_REMOVE,   DBLOG_AUTOREMOVE},
	{ DB_LOG_DIRECT,        DBLOG_DIRECT},
	{ DB_LOG_DSYNC,         DBLOG_DSYNC},
	{ DB_LOG_IN_MEMORY,     DBLOG_INMEMORY},
	{ DB_LOG_ZERO,          DBLOG_ZERO}
};
/*
 * __log_get_config --
 *	Configure the logging subsystem.
 */
int __log_get_config(DB_ENV * dbenv, uint32 which, int * onp)
{
	DB_LOG * dblp;
	uint32 flags;
	ENV * env = dbenv->env;
	if(FLD_ISSET(which, ~OK_FLAGS))
		return __db_ferr(env, "DB_ENV->log_get_config", 0);
	dblp = env->lg_handle;
	ENV_REQUIRES_CONFIG(env, dblp, "DB_ENV->log_get_config", DB_INIT_LOG);
	__env_fetch_flags(LogMap, sizeof(LogMap), &dblp->flags, &flags);
	__log_get_flags(dbenv, &flags);
	*onp = LF_ISSET(which) ? 1 : 0;
	return 0;
}
/*
 * __log_set_config --
 *	Configure the logging subsystem.
 */
int __log_set_config(DB_ENV * dbenv, uint32 flags, int on)
{
	return __log_set_config_int(dbenv, flags, on, 0);
}
/*
 * __log_set_config_int --
 *	Configure the logging subsystem.
 */
int __log_set_config_int(DB_ENV * dbenv, uint32 flags, int on, int in_open)
{
	uint32 mapped_flags;
	ENV * env = dbenv->env;
	DB_LOG * dblp = env->lg_handle;
	if(FLD_ISSET(flags, ~OK_FLAGS))
		return __db_ferr(env, "DB_ENV->log_set_config", 0);
	ENV_NOT_CONFIGURED(env, dblp, "DB_ENV->log_set_config", DB_INIT_LOG);
	if(LF_ISSET(DB_LOG_DIRECT) && __os_support_direct_io() == 0) {
		__db_errx(env, "DB_ENV->log_set_config: direct I/O either not configured or not supported");
		return EINVAL;
	}
	if(LOGGING_ON(env)) {
		if(!in_open && LF_ISSET(DB_LOG_IN_MEMORY) && ((LOG *)dblp->reginfo.primary)->db_log_inmemory == 0)
			ENV_ILLEGAL_AFTER_OPEN(env, "DB_ENV->log_set_config: DB_LOG_IN_MEMORY");
		__log_set_flags(env, flags, on);
		mapped_flags = 0;
		__env_map_flags(LogMap, sizeof(LogMap), &flags, &mapped_flags);
		SETFLAG(dblp->flags, mapped_flags, on);
	}
	else {
		/*
		 * DB_LOG_IN_MEMORY, DB_TXN_NOSYNC and DB_TXN_WRITE_NOSYNC
		 * are mutually incompatible.  If we're setting one of them,
		 * clear all current settings.
		 */
		if(on && LF_ISSET(DB_LOG_IN_MEMORY))
			F_CLR(dbenv, DB_ENV_TXN_NOSYNC|DB_ENV_TXN_WRITE_NOSYNC);
		SETFLAG(dbenv->lg_flags, flags, on);
	}
	return 0;
}
/*
 * __log_check_sizes --
 *	Makes sure that the log file size and log buffer size are compatible.
 */
int __log_check_sizes(ENV * env, uint32 lg_max, uint32 lg_bsize)
{
	LOG * lp;
	int inmem;
	DB_ENV * dbenv = env->dbenv;
	if(LOGGING_ON(env)) {
		lp = (LOG *)env->lg_handle->reginfo.primary;
		inmem = lp->db_log_inmemory;
		lg_bsize = lp->buffer_size;
	}
	else
		inmem = (FLD_ISSET(dbenv->lg_flags, DB_LOG_IN_MEMORY) != 0);
	if(inmem) {
		SETIFZ(lg_bsize, LG_BSIZE_INMEM);
		SETIFZ(lg_max, LG_MAX_INMEM);
		if(lg_bsize <= lg_max) {
			__db_errx(env, "in-memory log buffer must be larger than the log file size");
			return EINVAL;
		}
	}
	return 0;
}
