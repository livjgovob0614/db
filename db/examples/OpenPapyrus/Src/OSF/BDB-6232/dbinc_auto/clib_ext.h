/* DO NOT EDIT: automatically built by dist/s_include. */
#ifndef	_clib_ext_h_
#define	_clib_ext_h_

#if defined(__cplusplus)
// @sobolev extern "C" {
#endif

#ifndef HAVE_ATOI
	int atoi(const char *);
#endif
#ifndef HAVE_ATOL
	long atol(const char *);
#endif
#ifndef HAVE_BSEARCH
	void * bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
#endif
#ifndef HAVE_GETCWD
	char * getcwd(char *, size_t);
#endif
#ifndef HAVE_GETOPT
	int getopt(int, char * const *, const char *);
#endif
#ifndef HAVE_ISALPHA
	int isalpha(int);
#endif
#ifndef HAVE_ISDIGIT
	int isdigit(int);
#endif
#ifndef HAVE_ISPRINT
int isprint(int);
#endif
#ifndef HAVE_ISSPACE
int isspace(int);
#endif
#ifndef HAVE_MEMCMP
int memcmp __P((const void *, const void *, size_t));
#endif
#ifndef HAVE_MEMCPY
void *memcpy __P((void *, const void *, size_t));
#endif
#ifndef HAVE_MEMMOVE
void *memmove __P((void *, const void *, size_t));
#endif
#ifndef HAVE_PRINTF
int printf __P((const char *, ...));
#endif
#ifndef HAVE_PRINTF
int fprintf __P((FILE *, const char *, ...));
#endif
#ifndef HAVE_PRINTF
int vfprintf __P((FILE *, const char *, va_list));
#endif
#ifndef HAVE_QSORT
void qsort __P((void *, size_t, size_t, int(*)(const void *, const void *)));
#endif
#ifndef HAVE_RAISE
int raise(int);
#endif
#ifndef HAVE_RAND
int rand(void);
void srand __P((unsigned int));
#endif
#ifndef HAVE_SNPRINTF
int snprintf __P((char *, size_t, const char *, ...));
#endif
#ifndef HAVE_VSNPRINTF
int vsnprintf __P((char *, size_t, const char *, va_list));
#endif
#ifndef HAVE_STRCASECMP
int strcasecmp __P((const char *, const char *));
#endif
#ifndef HAVE_STRCASECMP
int strncasecmp __P((const char *, const char *, size_t));
#endif
#ifndef HAVE_STRCAT
char *strcat __P((char *, const char *));
#endif
#ifndef HAVE_STRCHR
char *strchr __P((const char *,  int));
#endif
#ifndef HAVE_STRDUP
char *strdup(const char *);
#endif
#ifndef HAVE_STRERROR
char *strerror(int);
#endif
#ifndef HAVE_STRNCAT
char *strncat __P((char *, const char *, size_t));
#endif
#ifndef HAVE_STRNCMP
int strncmp __P((const char *, const char *, size_t));
#endif
#ifndef HAVE_STRRCHR
char *strrchr __P((const char *, int));
#endif
#ifndef HAVE_STRSEP
char *strsep __P((char **, const char *));
#endif
#ifndef HAVE_STRTOL
long strtol __P((const char *, char **, int));
#endif
#ifndef HAVE_STRTOUL
unsigned long strtoul __P((const char *, char **, int));
#endif
#ifndef HAVE_TIME
time_t time __P((time_t *));
#endif

#if defined(__cplusplus)
// @sobolev }
#endif
#endif /* !_clib_ext_h_ */
