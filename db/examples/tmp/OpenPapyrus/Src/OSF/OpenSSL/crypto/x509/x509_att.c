/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "internal/cryptlib.h"
#pragma hdrstop
//#include "x509_lcl.h"

int X509at_get_attr_count(const STACK_OF(X509_ATTRIBUTE) * x)
{
	return sk_X509_ATTRIBUTE_num(x);
}

int X509at_get_attr_by_NID(const STACK_OF(X509_ATTRIBUTE) * x, int nid, int lastpos)
{
	const ASN1_OBJECT * obj = OBJ_nid2obj(nid);
	return obj ? X509at_get_attr_by_OBJ(x, obj, lastpos) : -2;
}

int X509at_get_attr_by_OBJ(const STACK_OF(X509_ATTRIBUTE) * sk, const ASN1_OBJECT * obj, int lastpos)
{
	int n;
	X509_ATTRIBUTE * ex;
	if(sk == NULL)
		return -1;
	lastpos++;
	if(lastpos < 0)
		lastpos = 0;
	n = sk_X509_ATTRIBUTE_num(sk);
	for(; lastpos < n; lastpos++) {
		ex = sk_X509_ATTRIBUTE_value(sk, lastpos);
		if(OBJ_cmp(ex->object, obj) == 0)
			return (lastpos);
	}
	return -1;
}

X509_ATTRIBUTE * X509at_get_attr(const STACK_OF(X509_ATTRIBUTE) * x, int loc)
{
	if(x == NULL || sk_X509_ATTRIBUTE_num(x) <= loc || loc < 0)
		return NULL;
	else
		return sk_X509_ATTRIBUTE_value(x, loc);
}

X509_ATTRIBUTE * X509at_delete_attr(STACK_OF(X509_ATTRIBUTE) * x, int loc)
{
	X509_ATTRIBUTE * ret;
	if(x == NULL || sk_X509_ATTRIBUTE_num(x) <= loc || loc < 0)
		return NULL;
	ret = sk_X509_ATTRIBUTE_delete(x, loc);
	return ret;
}

STACK_OF(X509_ATTRIBUTE) *X509at_add1_attr(STACK_OF(X509_ATTRIBUTE) **x, X509_ATTRIBUTE *attr)
{
	X509_ATTRIBUTE * new_attr = NULL;
	STACK_OF(X509_ATTRIBUTE) *sk = NULL;
	if(!x) {
		X509err(X509_F_X509AT_ADD1_ATTR, ERR_R_PASSED_NULL_PARAMETER);
		goto err2;
	}
	if(*x == NULL) {
		if((sk = sk_X509_ATTRIBUTE_new_null()) == NULL)
			goto err;
	}
	else
		sk = *x;
	if((new_attr = X509_ATTRIBUTE_dup(attr)) == NULL)
		goto err2;
	if(!sk_X509_ATTRIBUTE_push(sk, new_attr))
		goto err;
	if(*x == NULL)
		*x = sk;
	return (sk);
err:
	X509err(X509_F_X509AT_ADD1_ATTR, ERR_R_MALLOC_FAILURE);
err2:
	X509_ATTRIBUTE_free(new_attr);
	sk_X509_ATTRIBUTE_free(sk);
	return NULL;
}

STACK_OF(X509_ATTRIBUTE) *X509at_add1_attr_by_OBJ(STACK_OF(X509_ATTRIBUTE) **x, const ASN1_OBJECT *obj, int type, const uchar *bytes, int len)
{
	STACK_OF(X509_ATTRIBUTE) *ret;
	X509_ATTRIBUTE * attr = X509_ATTRIBUTE_create_by_OBJ(NULL, obj, type, bytes, len);
	if(!attr)
		return 0;
	ret = X509at_add1_attr(x, attr);
	X509_ATTRIBUTE_free(attr);
	return ret;
}

STACK_OF(X509_ATTRIBUTE) *X509at_add1_attr_by_NID(STACK_OF(X509_ATTRIBUTE) **x, int nid, int type, const uchar *bytes, int len)
{
	STACK_OF(X509_ATTRIBUTE) *ret;
	X509_ATTRIBUTE * attr = X509_ATTRIBUTE_create_by_NID(NULL, nid, type, bytes, len);
	if(!attr)
		return 0;
	ret = X509at_add1_attr(x, attr);
	X509_ATTRIBUTE_free(attr);
	return ret;
}

STACK_OF(X509_ATTRIBUTE) *X509at_add1_attr_by_txt(STACK_OF(X509_ATTRIBUTE) **x, const char * attrname, int type, const uchar *bytes, int len)
{
	STACK_OF(X509_ATTRIBUTE) *ret;
	X509_ATTRIBUTE * attr = X509_ATTRIBUTE_create_by_txt(NULL, attrname, type, bytes, len);
	if(!attr)
		return 0;
	ret = X509at_add1_attr(x, attr);
	X509_ATTRIBUTE_free(attr);
	return ret;
}

void * X509at_get0_data_by_OBJ(STACK_OF(X509_ATTRIBUTE) * x, const ASN1_OBJECT * obj, int lastpos, int type)
{
	X509_ATTRIBUTE * at;
	int i = X509at_get_attr_by_OBJ(x, obj, lastpos);
	if(i == -1)
		return NULL;
	if((lastpos <= -2) && (X509at_get_attr_by_OBJ(x, obj, i) != -1))
		return NULL;
	at = X509at_get_attr(x, i);
	if(lastpos <= -3 && (X509_ATTRIBUTE_count(at) != 1))
		return NULL;
	return X509_ATTRIBUTE_get0_data(at, 0, type, 0);
}

X509_ATTRIBUTE * X509_ATTRIBUTE_create_by_NID(X509_ATTRIBUTE ** attr, int nid, int atrtype, const void * data, int len)
{
	X509_ATTRIBUTE * ret;
	ASN1_OBJECT * obj = OBJ_nid2obj(nid);
	if(obj == NULL) {
		X509err(X509_F_X509_ATTRIBUTE_CREATE_BY_NID, X509_R_UNKNOWN_NID);
		return NULL;
	}
	ret = X509_ATTRIBUTE_create_by_OBJ(attr, obj, atrtype, data, len);
	if(!ret)
		ASN1_OBJECT_free(obj);
	return ret;
}

X509_ATTRIBUTE * X509_ATTRIBUTE_create_by_OBJ(X509_ATTRIBUTE ** attr, const ASN1_OBJECT * obj, int atrtype, const void * data, int len)
{
	X509_ATTRIBUTE * ret;
	if(!attr || (*attr == NULL)) {
		if((ret = X509_ATTRIBUTE_new()) == NULL) {
			X509err(X509_F_X509_ATTRIBUTE_CREATE_BY_OBJ, ERR_R_MALLOC_FAILURE);
			return NULL;
		}
	}
	else
		ret = *attr;
	if(!X509_ATTRIBUTE_set1_object(ret, obj))
		goto err;
	if(!X509_ATTRIBUTE_set1_data(ret, atrtype, data, len))
		goto err;
	if(attr && *attr == NULL)
		*attr = ret;
	return ret;
err:
	if(!attr || (ret != *attr))
		X509_ATTRIBUTE_free(ret);
	return NULL;
}

X509_ATTRIBUTE * X509_ATTRIBUTE_create_by_txt(X509_ATTRIBUTE ** attr, const char * atrname, int type, const uchar * bytes, int len)
{
	X509_ATTRIBUTE * nattr;
	ASN1_OBJECT * obj = OBJ_txt2obj(atrname, 0);
	if(obj == NULL) {
		X509err(X509_F_X509_ATTRIBUTE_CREATE_BY_TXT, X509_R_INVALID_FIELD_NAME);
		ERR_add_error_data(2, "name=", atrname);
		return NULL;
	}
	nattr = X509_ATTRIBUTE_create_by_OBJ(attr, obj, type, bytes, len);
	ASN1_OBJECT_free(obj);
	return nattr;
}

int X509_ATTRIBUTE_set1_object(X509_ATTRIBUTE * attr, const ASN1_OBJECT * obj)
{
	if(!attr || (obj == NULL))
		return 0;
	ASN1_OBJECT_free(attr->object);
	attr->object = OBJ_dup(obj);
	return attr->object != NULL;
}

int X509_ATTRIBUTE_set1_data(X509_ATTRIBUTE * attr, int attrtype, const void * data, int len)
{
	ASN1_TYPE * ttmp = NULL;
	ASN1_STRING * stmp = NULL;
	int atype = 0;
	if(!attr)
		return 0;
	if(attrtype & MBSTRING_FLAG) {
		stmp = ASN1_STRING_set_by_NID(NULL, reinterpret_cast<const uchar *>(data), len, attrtype, OBJ_obj2nid(attr->object));
		if(!stmp) {
			X509err(X509_F_X509_ATTRIBUTE_SET1_DATA, ERR_R_ASN1_LIB);
			return 0;
		}
		atype = stmp->type;
	}
	else if(len != -1) {
		if((stmp = ASN1_STRING_type_new(attrtype)) == NULL)
			goto err;
		if(!ASN1_STRING_set(stmp, data, len))
			goto err;
		atype = attrtype;
	}
	/*
	 * This is a bit naughty because the attribute should really have at
	 * least one value but some types use and zero length SET and require
	 * this.
	 */
	if(attrtype == 0) {
		ASN1_STRING_free(stmp);
		return 1;
	}
	if((ttmp = ASN1_TYPE_new()) == NULL)
		goto err;
	if((len == -1) && !(attrtype & MBSTRING_FLAG)) {
		if(!ASN1_TYPE_set1(ttmp, attrtype, data))
			goto err;
	}
	else {
		ASN1_TYPE_set(ttmp, atype, stmp);
		stmp = NULL;
	}
	if(!sk_ASN1_TYPE_push(attr->set, ttmp))
		goto err;
	return 1;
err:
	X509err(X509_F_X509_ATTRIBUTE_SET1_DATA, ERR_R_MALLOC_FAILURE);
	ASN1_TYPE_free(ttmp);
	ASN1_STRING_free(stmp);
	return 0;
}

int X509_ATTRIBUTE_count(const X509_ATTRIBUTE * attr)
{
	return attr ? sk_ASN1_TYPE_num(attr->set) : 0;
}

ASN1_OBJECT * X509_ATTRIBUTE_get0_object(X509_ATTRIBUTE * attr)
{
	return attr ? attr->object : 0;
}

void * X509_ATTRIBUTE_get0_data(X509_ATTRIBUTE * attr, int idx, int atrtype, void * data)
{
	ASN1_TYPE * ttmp = X509_ATTRIBUTE_get0_type(attr, idx);
	if(!ttmp)
		return NULL;
	if(atrtype != ASN1_TYPE_get(ttmp)) {
		X509err(X509_F_X509_ATTRIBUTE_GET0_DATA, X509_R_WRONG_TYPE);
		return NULL;
	}
	return ttmp->value.ptr;
}

ASN1_TYPE * X509_ATTRIBUTE_get0_type(X509_ATTRIBUTE * attr, int idx)
{
	return attr ? sk_ASN1_TYPE_value(attr->set, idx) : 0;
}
