// BASE64.CPP
// @threadsafe
//
#include <slib.h>
#include <tv.h>
#pragma hdrstop

/*static void FASTCALL makebasis64(char * pBuf)
{
// 
// ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
// ????????????????????????????????????????????????????????????????
// ????????????????????????????????????????????????????????????????
// ???????????
// 
	const size_t count = 203;
	size_t p = 0;
	char   i;
	for(i = 0; i < 'Z'-'A'+1; i++)
		pBuf[p++] = 'A' + i;
	for(i = 0; i < 'z'-'a'+1; i++)
		pBuf[p++] = 'a' + i;
	for(i = 0; i < 10; i++)
		pBuf[p++] = '0'+i;
	pBuf[p++] = '+';
	pBuf[p++] = '/';
	while(p < count)
		pBuf[p++] = '?';
	pBuf[p] = 0;
}*/

int FASTCALL encode64(const char * pIn, size_t inLen, char * pOut, size_t outMax, size_t * pOutLen)
{
	int    ok = 1;
	//char   basis_64[256];
	uchar * out = (uchar *)pOut;
	uchar  oval;
	const  uchar * in = (const uchar *)pIn;
	size_t olen = (inLen + 2) / 3 * 4;
	size_t real_len = 0;
	ASSIGN_PTR(pOutLen, olen);
	//makebasis64(basis_64);
	const char * p_basis = STextConst::Get(STextConst::cBasis64, 0);
	THROW(outMax >= olen);
	while(inLen >= 3) {
		THROW(real_len < (outMax-4)); // user provided max buffer size; make sure we don't go over it
		*out++ = p_basis[in[0] >> 2];
		*out++ = p_basis[((in[0] << 4) & 0x30) | (in[1] >> 4)];
		*out++ = p_basis[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
		*out++ = p_basis[in[2] & 0x3f];
		real_len += 4;
		in += 3;
		inLen -= 3;
	}
	if(inLen > 0) {
		THROW(real_len < (outMax-4)); // user provided max buffer size; make sure we don't go over it
		*out++ = p_basis[in[0] >> 2];
		oval = (in[0] << 4) & 0x30;
		if(inLen > 1)
			oval |= in[1] >> 4;
		*out++ = p_basis[oval];
		*out++ = (inLen < 2) ? '=' : p_basis[(in[1] << 2) & 0x3c];
		*out++ = '=';
		real_len += 4;
	}
	THROW(real_len < outMax);
	*out = '\0';
	real_len++;
	ASSIGN_PTR(pOutLen, real_len);
	CATCH
		ok = (SLibError = SLERR_BUFTOOSMALL, 0);
	ENDCATCH
	return ok;
}

static const char index_64[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

#define CHAR64(c) (index_64[(uint8)(c)])

int FASTCALL decode64(const char * pIn, size_t inLen, char * pOut, size_t * pOutLen)
{
	assert(sizeof(index_64) == 256);
	int    ok = 1;//, c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	size_t len = 0, lup = 0;
	char * p_out = pOut;
	const  char * p_in = pIn;
	//char   basis_64[256];
	if(p_in[0] == '+' && p_in[1] == ' ')
		p_in += 2;
	THROW(*p_in != 0);
	// makebasis64(basis_64);
	if(p_out) {
		for(lup = 0; lup < inLen / 4; lup++) {
			const int b1 = CHAR64(p_in[0]);
			const int b2 = CHAR64(p_in[1]);
			const int c3 = p_in[2];
			const int b3 = CHAR64(c3);
			const int c4 = p_in[3];
			const int b4 = CHAR64(c4);
			THROW(b1 != -1 && b2 != -1 && (c3 == '=' || b3 != -1) && (c4 == '=' || b4 != -1));
			p_in += 4;
			*p_out++ = (char)(b1 << 2) | (b2 >> 4);
			len++;
			if(c3 != '=') {
				*p_out++ = (char)((b2 << 4) & 0xf0) | (b3 >> 2);
				len++;
				if(c4 != '=') {
					*p_out++ = (char)((b3 << 6) & 0xc0) | b4;
					len++;
				}
			}
		}
	}
	else {
		for(lup = 0; lup < inLen / 4; lup++) {
			const int b1 = CHAR64(p_in[0]);
			const int b2 = CHAR64(p_in[1]);
			const int c3 = p_in[2];
			const int b3 = CHAR64(c3);
			const int c4 = p_in[3];
			const int b4 = CHAR64(c4);
			THROW(b1 != -1 && b2 != -1 && (c3 == '=' || b3 != -1) && (c4 == '=' || b4 != -1));
			p_in += 4;
			len++;
			if(c3 != '=') {
				len++;
				if(c4 != '=') {
					len++;
				}
			}
		}
	}
	ASSIGN_PTR(pOutLen, len);
	CATCHZOK
	return ok;
}
//
//
//
MIME64::MIME64()
{
	// @v9.9.5 makebasis64(Basis64);
}

int MIME64::Encode(const void * pIn, size_t inLen, char * pOut, size_t outBufLen, size_t * pOutDataLen) const
{
	int    ok = 1;
	uchar * out = (uchar *)pOut;
	const  uchar * in = (const uchar *)pIn;
	size_t olen = (inLen + 2) / 3 * 4;
	ASSIGN_PTR(pOutDataLen, olen);
	if(outBufLen >= olen) {
		const char * p_basis = STextConst::Get(STextConst::cBasis64, 0);
		while(inLen >= 3) {
			// user provided max buffer size; make sure we don't go over it
			*out++ = p_basis[in[0] >> 2];
			*out++ = p_basis[((in[0] << 4) & 0x30) | (in[1] >> 4)];
			*out++ = p_basis[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
			*out++ = p_basis[in[2] & 0x3f];
			in += 3;
			inLen -= 3;
		}
		if(inLen > 0) {
			// user provided max buffer size; make sure we don't go over it
			*out++ = p_basis[in[0] >> 2];
			uchar  oval = (in[0] << 4) & 0x30;
			if(inLen > 1)
				oval |= in[1] >> 4;
			*out++ = p_basis[oval];
			*out++ = (inLen < 2) ? '=' : p_basis[(in[1] << 2) & 0x3c];
			*out++ = '=';
		}
		if(olen < outBufLen)
			*out = '\0';
	}
	else
		ok = (SLibError = SLERR_BUFTOOSMALL, 0);
	return ok;
}

int MIME64::Decode(const char * pIn, size_t inLen, char * pOut, size_t * pOutDataLen) const
{
	int    ok = 1;
	size_t len = 0, lup = 0;
	char * p_out = pOut;
	const  char * p_in = pIn;
	if(p_in[0] == '+' && p_in[1] == ' ')
		p_in += 2;
	THROW(*p_in != 0);
	for(lup = 0; lup < inLen / 4; lup++) {
		char   c64[4];
		char   c[4];
		*(uint32 *)c = *(uint32 *)p_in;
		p_in += 4;
		c64[0] = CHAR64(c[0]);
		c64[1] = CHAR64(c[1]);
		c64[2] = CHAR64(c[2]);
		c64[3] = CHAR64(c[3]);
		THROW(c64[0] != -1 && c64[1] != -1);
		THROW(c[2] == '=' || c64[2] != -1);
		THROW(c[3] == '=' || c64[3] != -1);
		*p_out++ = (char)(c64[0] << 2) | (c64[1] >> 4);
		len++;
		if(c[2] != '=') {
			*p_out++ = (char)((c64[1] << 4) & 0xf0) | (c64[2] >> 2);
			len++;
			if(c[3] != '=') {
				*p_out++ = (char) ((c64[2] << 6) & 0xc0) | c64[3];
				len++;
			}
		}
	}
	*p_out = 0;
	ASSIGN_PTR(pOutDataLen, len);
	CATCHZOK
	return ok;
}
