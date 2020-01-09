/* DO NOT EDIT: automatically built by dist/s_include. */
#ifndef	_env_ext_h_
#define	_env_ext_h_

#if defined(__cplusplus)
extern "C" {
#endif

void __env_alloc_init(REGINFO *, size_t);
size_t __env_alloc_overhead(void);
size_t __env_alloc_size(size_t);
int __env_alloc(REGINFO *, size_t, void *);
void __env_alloc_free(REGINFO *, void *);
int __env_alloc_extend(REGINFO *, void *, size_t *);
int __env_region_extend(ENV *, REGINFO *);
uintmax_t __env_elem_size(ENV *, void *);
void * __env_get_chunk(REGINFO *, void **, uintmax_t *);
void __env_alloc_print(REGINFO *, uint32);
int __env_backup_copy(DB_ENV *, const DB_ENV *);
int __env_get_backup_config(DB_ENV *, DB_BACKUP_CONFIG, uint32*);
int __env_set_backup_config(DB_ENV *, DB_BACKUP_CONFIG, uint32);
int __env_get_backup_callbacks(DB_ENV *, int (**)(DB_ENV *, const char *, const char *, void **), int (**)(DB_ENV *, uint32, uint32, uint32, uint8 *, void *), int (**)(DB_ENV *, const char *, void *));
int __env_set_backup_callbacks(DB_ENV *, int (*)(DB_ENV *, const char *, const char *, void **), int (*)(DB_ENV *, uint32, uint32, uint32, uint8 *, void *), int (*)(DB_ENV *, const char *, void *));
int __env_read_db_config(ENV *);
int __env_failchk_pp(DB_ENV *, uint32);
int __env_failchk_int(DB_ENV *);
size_t __env_thread_size(ENV *, size_t);
size_t __env_thread_max(ENV *);
int __env_thread_init(ENV *, int);
void __env_thread_destroy(ENV *);
int __env_set_state(ENV *, DB_THREAD_INFO **, DB_THREAD_STATE);
char *__env_thread_id_string(DB_ENV *, pid_t, db_threadid_t, char *);
int __db_file_extend(ENV *, DB_FH *, size_t);
int __db_file_multi_write(ENV *, const char *);
int __db_file_write(ENV *, DB_FH *, uint32, uint32, int);
void __db_env_destroy(DB_ENV *);
int  __env_get_alloc(DB_ENV *, void *(**)(size_t), void *(**)(void *, size_t), void (**)(void *));
int  __env_set_alloc(DB_ENV *, void *(*)(size_t), void *(*)(void *, size_t), void (*)(void *));
int  __env_get_memory_init(DB_ENV *, DB_MEM_CONFIG, uint32 *);
int  __env_get_blob_threshold_pp(DB_ENV *, uint32 *);
int  __env_get_blob_threshold_int(ENV *, uint32 *);
int  __env_set_blob_threshold(DB_ENV *, uint32, uint32);
int  __env_set_memory_init(DB_ENV *, DB_MEM_CONFIG, uint32);
int  __env_get_memory_max(DB_ENV *, uint32 *, uint32 *);
int  __env_set_memory_max(DB_ENV *, uint32, uint32);
int  __env_set_blob_dir(DB_ENV *, const char *);
int __env_get_encrypt_flags(DB_ENV *, uint32 *);
int __env_set_encrypt(DB_ENV *, const char *, uint32);
void __env_map_flags(const FLAG_MAP *, u_int, uint32, uint32 *);
void __env_fetch_flags(const FLAG_MAP *, u_int, uint32 *, uint32 *);
int  __env_set_flags(DB_ENV *, uint32, int);
int __env_set_backup(ENV *, int);
int  __env_set_data_dir(DB_ENV *, const char *);
int  __env_add_data_dir(DB_ENV *, const char *);
int  __env_set_create_dir(DB_ENV *, const char *);
int __env_set_home_dir(DB_ENV *, const char *);
int  __env_set_metadata_dir(DB_ENV *, const char *);
int  __env_set_data_len(DB_ENV *, uint32);
int  __env_set_intermediate_dir_mode(DB_ENV *, const char *);
void __env_get_errcall(DB_ENV *, void (**)(const DB_ENV *, const char *, const char *));
void __env_set_errcall(DB_ENV *, void (*)(const DB_ENV *, const char *, const char *));
void __env_get_errfile(DB_ENV *, FILE **);
void __env_set_errfile(DB_ENV *, FILE *);
void __env_get_errpfx(DB_ENV *, const char **);
void __env_set_errpfx(DB_ENV *, const char *);
int  __env_set_thread_count(DB_ENV *, uint32);
void __env_get_msgcall(DB_ENV *, void (**)(const DB_ENV *, const char *, const char *));
void __env_set_msgcall(DB_ENV *, void (*)(const DB_ENV *, const char *, const char *));
void __env_get_msgfile(DB_ENV *, FILE **);
void __env_set_msgfile(DB_ENV *, FILE *);
void __env_get_msgpfx(DB_ENV *, const char **);
void __env_set_msgpfx(DB_ENV *, const char *);
int  __env_set_paniccall(DB_ENV *, void (*)(DB_ENV *, int));
int  __env_set_shm_key(DB_ENV *, long);
int  __env_set_tmp_dir(DB_ENV *, const char *);
int  __env_set_verbose(DB_ENV *, uint32, int);
int __db_mi_env(ENV *, const char *);
int __db_mi_open(ENV *, const char *, int);
int __env_not_config(ENV *, char *, uint32);
int __env_set_timeout(DB_ENV *, db_timeout_t, uint32);
int __env_encrypt_adj_size(DB_ENV *, size_t, size_t *);
int __env_encrypt(DB_ENV *, uint8 *, uint8 *, size_t);
int __env_decrypt(DB_ENV *, uint8 *, uint8 *, size_t);
int __db_appname(ENV *, APPNAME, const char *, const char **, char **);
int __db_tmp_open(ENV *, uint32, DB_FH **);
int __env_open_pp(DB_ENV *, const char *, uint32, int);
int __env_open(DB_ENV *, const char *, uint32, int);
int __env_remove(DB_ENV *, const char *, uint32);
int __env_config(DB_ENV *, const char *, uint32 *, int);
int __env_close_pp(DB_ENV *, uint32);
int __env_close(DB_ENV *, uint32);
int __env_refresh(DB_ENV *, uint32, int);
int __env_get_open_flags(DB_ENV *, uint32 *);
int __env_attach_regions(DB_ENV *,  uint32, uint32, int);
int __db_apprec(ENV *, DB_THREAD_INFO *, DB_LSN *, DB_LSN *, int, uint32);
int __env_openfiles(ENV *, DB_LOGC *, void *, DBT *, DB_LSN *, DB_LSN *, double, int);
int __env_init_rec(ENV *, uint32);
int __env_attach(ENV *, uint32 *, int, int);
int __env_turn_on(ENV *);
int __env_turn_off(ENV *, uint32);
void __env_panic_set(ENV *, int);
int __env_ref_increment(ENV *);
int __env_ref_decrement(ENV *);
int __env_ref_get(DB_ENV *, uint32 *);
int __env_region_cleanup(ENV *);
int __env_detach(ENV *, int);
int __env_remove_env(ENV *);
int __env_region_attach(ENV *, REGINFO *, size_t, size_t);
int __env_region_share(ENV *, REGINFO *);
int __env_region_detach(ENV *, REGINFO *, int);
int __envreg_register(ENV *, int *, uint32);
int __envreg_unregister(ENV *, int);
int __envreg_xunlock(ENV *);
int __envreg_isalive(DB_ENV *, pid_t, db_threadid_t, uint32);
uint32 __env_struct_sig(void);
int __env_slice_db_home(DB_ENV *, const char *);
#ifdef HAVE_SLICES
	int __env_slice_open(DB_ENV *, const char *, uint32, int);
#endif
int __env_slice_close_pp(DB_ENV *, uint32);
int __env_slice_txn_checkpoint_pp(DB_ENV *, uint32, uint32, uint32);
int __env_slice_lsn_reset_pp(DB_ENV *, const char *, uint32);
int __env_slice_fileid_reset_pp(DB_ENV *, const char *, uint32);
int  __env_get_slice_count(DB_ENV *, uint32 *);
int  __env_get_slices(DB_ENV *, DB_ENV ***);
int __env_slice_configure(const DB_ENV *, DB_ENV *);
int  __env_set_slice_count(DB_ENV *, uint32);
DB_ENV *__slice_iterate(DB_ENV *, int *);
int __env_slice_dbremove(ENV *, const char *, const char *, uint32);
#ifndef HAVE_SLICES
	int  __env_no_slices(ENV *);
#endif
int  __env_not_sliced(ENV *);
int __env_stat_print_pp(DB_ENV *, uint32);
int __env_print_thread(ENV *);
void __db_print_fh(ENV *, const char *, DB_FH *, uint32);
void __db_print_fileid(ENV *, uint8 *, const char *);
void __db_dl(ENV *, const char *, u_long);
void __db_dl_pct(ENV *, const char *, u_long, int, const char *);
void __db_dlbytes(ENV *, const char *, u_long, u_long, u_long);
void __db_print_reginfo(ENV *, REGINFO *, const char *, uint32);
int __db_stat_not_built(ENV *);
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_close(ENV *);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_get_ack_policy(DB_ENV *, int *);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_set_ack_policy(DB_ENV *, int);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_get_incoming_queue_max(DB_ENV *, uint32 *, uint32 *);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_set_incoming_queue_max(DB_ENV *, uint32, uint32);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_get_incoming_queue_redzone(DB_ENV *, uint32 *, uint32 *);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_get_incoming_queue_fullevent(DB_ENV *, int *);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_site(DB_ENV *, const char *, u_int, DB_SITE **, uint32);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_site_by_eid(DB_ENV *, int, DB_SITE **);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_local_site(DB_ENV *, DB_SITE **);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_site_list_pp(DB_ENV *, u_int *, DB_REPMGR_SITE **);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_start_pp(DB_ENV *, int, uint32);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_stat_pp(DB_ENV *, DB_REPMGR_STAT **, uint32);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_stat_print_pp(DB_ENV *, uint32);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_handle_event(ENV *, uint32, void *);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_channel(DB_ENV *, int, DB_CHANNEL **, uint32);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_set_msg_dispatch(DB_ENV *, void (*)(DB_ENV *, DB_CHANNEL *, DBT *, uint32, uint32), uint32);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_init_recover(ENV *, DB_DISTAB *);
#endif
#ifndef HAVE_REPLICATION_THREADS
	int __repmgr_set_socket(DB_ENV *, int (*)(DB_ENV *, DB_REPMGR_SOCKET, int *, uint32));
#endif

#if defined(__cplusplus)
}
#endif
#endif /* !_env_ext_h_ */
