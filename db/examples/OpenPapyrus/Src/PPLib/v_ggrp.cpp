// V_GGRP.CPP
// Copyright (c) A.Sobolev 2006, 2007
// @construction
//
#include <pp.h>
#include <ppdlgs.h>
#pragma hdrstop

struct GoodsGroupFilt {
	enum {
		fNormal        = 0x0001, // ���������� ������������ ������
		fAltern        = 0x0002, // ���������� �������������� ������
		fExtracFilters = 0x0004  // ��������� ������ � ��������������� �������� �� �������
	};
	enum {
		fxAltExcl  = 0x0001, // ���������� ������ ������������ �������������� ������
		fxAltDyn   = 0x0002, // ���������� ������ ������������ �������������� ������
		fxAltTemp  = 0x0004, // ���������� ������ ��������� �������������� ������
		fxFiltered = 0x0008, // ���������� ������ �������������� ������, ���������� ������
	};
	enum {
		sName = 0x0001,
		sAbbr = 0x0002
	};
	PPID   ParentID;
	long   Flags;
};

class PPViewGoodsGroup : public PPView {
public:
	SLAPI  PPViewGoodsGroup();
	virtual SLAPI ~PPViewGoodsGroup();
	virtual int   SLAPI Browse(int modeless);
	virtual int   SLAPI ProcessCommand(uint ppvCmd, const void *, PPViewBrowser *);
	int    SLAPI Init(const GoodsGroupFilt * pFilt);
private:
	PPObjGoodsGroup GGObj;
	GoodsGroupFilt Filt;
	PPIDArray IdList;
	//StringSet
};

SLAPI PPViewGoodsGroup::PPViewGoodsGroup() : PPView(&GGObj)
{
}

SLAPI PPViewGoodsGroup::~PPViewGoodsGroup()
{
}

int SLAPI PPViewGoodsGroup::Init(const GoodsGroupFilt * pFilt)
{
	int    ok = 1;
	Filt = *pFilt;

	return ok;
}

