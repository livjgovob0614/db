/* fileio.c -- does standard I/O

   (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
   See tidy.h for the copyright notice.

   CVS Info :

    $Author: arnaud02 $
    $Date: 2007/05/30 16:47:31 $
    $Revision: 1.17 $

   Default implementations of Tidy input sources
   and output sinks based on standard C FILE*.

 */
#include "tidy-int.h"
#pragma hdrstop
#include "fileio.h"

typedef struct _fp_input_source {
	FILE * fp;
	TidyBuffer unget;
} FileSource;

static int TIDY_CALL filesrc_getByte(void* sourceData)
{
	FileSource * fin = (FileSource*)sourceData;
	return (fin->unget.size > 0) ? tidyBufPopByte(&fin->unget) : fgetc(fin->fp);
}

static bool TIDY_CALL filesrc_eof(void* sourceData)
{
	FileSource* fin = (FileSource*)sourceData;
	bool isEOF = ( fin->unget.size == 0 );
	if(isEOF)
		isEOF = feof(fin->fp) != 0;
	return isEOF;
}

static void TIDY_CALL filesrc_ungetByte(void* sourceData, byte bv)
{
	FileSource* fin = (FileSource*)sourceData;
	tidyBufPutByte(&fin->unget, bv);
}

#if SUPPORT_POSIX_MAPPED_FILES
#define initFileSource initStdIOFileSource
#define freeFileSource freeStdIOFileSource
#endif
int TY_(initFileSource) (TidyAllocator *allocator, TidyInputSource* inp, FILE* fp)
{
	FileSource * fin = static_cast<FileSource *>(TidyAlloc(allocator, sizeof(FileSource)));
	if(!fin)
		return -1;
	TidyClearMemory(fin, sizeof(FileSource));
	fin->unget.allocator = allocator;
	fin->fp = fp;
	inp->getByte    = filesrc_getByte;
	inp->eof        = filesrc_eof;
	inp->ungetByte  = filesrc_ungetByte;
	inp->sourceData = fin;
	return 0;
}

void TY_(freeFileSource) (TidyInputSource* inp, bool closeIt)
{
	FileSource * fin = static_cast<FileSource *>(inp->sourceData);
	if(closeIt && fin && fin->fp)
		fclose(fin->fp);
	tidyBufFree(&fin->unget);
	TidyFree(fin->unget.allocator, fin);
}

void TIDY_CALL TY_(filesink_putByte) (void* sinkData, byte bv)
{
	FILE * fout = static_cast<FILE *>(sinkData);
	fputc(bv, fout);
}

void TY_(initFileSink) (TidyOutputSink* outp, FILE* fp)
{
	outp->putByte  = TY_(filesink_putByte);
	outp->sinkData = fp;
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
