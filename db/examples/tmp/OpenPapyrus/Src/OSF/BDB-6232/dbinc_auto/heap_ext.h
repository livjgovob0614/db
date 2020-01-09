/* DO NOT EDIT: automatically built by dist/s_include. */
#ifndef	_heap_ext_h_
#define	_heap_ext_h_

#if defined(__cplusplus)
extern "C" {
#endif

int __heapc_init(DBC *);
int __heap_ditem __P((DBC *, PAGE *, uint32, uint32));
int __heap_append __P((DBC *, DBT *, DBT *));
int __heap_pitem __P((DBC *, PAGE *, uint32, uint32, DBT *, DBT *));
int __heapc_dup __P((DBC *, DBC *));
int __heapc_gsplit __P((DBC *, DBT *, void **, uint32 *));
int __heapc_refresh(DBC *);
int __heap_init_recover(ENV *, DB_DISTAB *);
int __heap_addrem_print(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_addrem_60_print(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_addrem_50_print(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_pg_alloc_print(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_trunc_meta_print(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_trunc_page_print(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_init_print(ENV *, DB_DISTAB *);
int __heap_backup __P((DB_ENV *, DB *, DB_THREAD_INFO *, DB_FH *, void *, uint32));
int __heap_pgin __P((DB *, db_pgno_t, void *, DBT *));
int __heap_pgout __P((DB *, db_pgno_t, void *, DBT *));
int __heap_mswap __P((ENV *, PAGE *));
int __heap_db_create(DB *);
int __heap_db_close(DB *);
int __heap_get_heapsize __P((DB *, uint32 *, uint32 *));
int __heap_get_heap_regionsize(DB *, uint32 *);
int __heap_set_heapsize __P((DB *, uint32, uint32, uint32));
int __heap_set_heap_regionsize(DB *, uint32);
int __heap_exist(void);
int __heap_open __P((DB *, DB_THREAD_INFO *, DB_TXN *, const char *, db_pgno_t, uint32));
int __heap_metachk __P((DB *, const char *, HEAPMETA *));
int __heap_read_meta __P((DB *, DB_THREAD_INFO *, DB_TXN *, db_pgno_t, uint32));
int __heap_new_file __P((DB *, DB_THREAD_INFO *, DB_TXN *, DB_FH *, const char *));
int __heap_create_region __P((DBC *, db_pgno_t));
int __heap_addrem_recover(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_pg_alloc_recover(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_trunc_meta_recover(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_trunc_page_recover(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_addrem_60_recover(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_addrem_50_recover(ENV *, DBT *, DB_LSN *, db_recops, void *);
int __heap_truncate __P((DBC *, uint32 *));
int __heap_stat __P((DBC *, void *, uint32));
int __heap_stat_print __P((DBC *, uint32));
void __heap_print_cursor(DBC *);
int __heap_stat_callback __P((DBC *, PAGE *, void *, int *));
int __heap_traverse __P((DBC *, int (*)(DBC *, PAGE *, void *, int *), void *));
int __db_no_heap_am(ENV *);
int __heap_60_heapmeta __P((DB *, char *, uint32, DB_FH *, PAGE *, int *));
int __heap_60_heap __P((DB *, char *, uint32, DB_FH *, PAGE *, int *));
int __heap_vrfy_meta __P((DB *, VRFY_DBINFO *, HEAPMETA *, db_pgno_t, uint32));
int __heap_vrfy __P((DB *, VRFY_DBINFO *, PAGE *, db_pgno_t, uint32));
int __heap_vrfy_structure __P((DB *, VRFY_DBINFO *, uint32));
int __heap_salvage __P((DB *, VRFY_DBINFO *, db_pgno_t, PAGE *, void *, int (*)(void *, const void *), uint32));
int __heap_meta2pgset __P((DB *, VRFY_DBINFO *, HEAPMETA *, DB *));

#if defined(__cplusplus)
}
#endif
#endif /* !_heap_ext_h_ */
