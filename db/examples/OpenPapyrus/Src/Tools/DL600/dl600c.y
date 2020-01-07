// DL600C.Y
// Copyright (c) A.Sobolev 2006-2007, 2008, 2009, 2010, 2011, 2016
//

// debug cmdline: /dict:$(SolutionDir)..\..\BASE\INIT_DL6 /data:$(SolutionDir)..\..\BASE\INIT_DL6 /oracle $(SolutionDir)..\rsrc\dl600\ppdbs.dl6
// debug cmdline: $(SolutionDir)..\rsrc\dl600\ppui-test.dl6

/*
Reserved words:
	IMPORT
	import *
	data
	interface
	struct
	enum
	iclass
	declare
	iteration
	link
	if
	else
	format
	in
	out
	inout
	default
	version
	library
	abstract
	table
	index
	file
*/

//abstract [marshal] ViewItem;

%{

#include <pp.h>
#include <dl600.h>

uint32 SetDecimalDim(uint dec, uint precision); // @prototype @def(dl600c.cpp)

extern long yyin_cnt;
extern YYIN_STR * yyin_struct;

#define YYERROR_VERBOSE

void yyerror(const char * pStr)
{
	printf("%s(%d): error: %s\n", (const char *)DCtx.GetInputFileName(), yyline, pStr);
	exit(1);
}

void ZapToken(CtmToken & rTok)
{
	rTok.Destroy();
}

void ZapToken2(CtmToken & rTok1, CtmToken & rTok2)
{
	rTok1.Destroy();
	rTok2.Destroy();
}

void ZapToken3(CtmToken & rTok1, CtmToken & rTok2, CtmToken & rTok3)
{
	rTok1.Destroy();
	rTok2.Destroy();
	rTok3.Destroy();
}

void ZapToken4(CtmToken & rTok1, CtmToken & rTok2, CtmToken & rTok3, CtmToken & rTok4)
{
	rTok1.Destroy();
	rTok2.Destroy();
	rTok3.Destroy();
	rTok4.Destroy();
}

void ZapToken5(CtmToken & rTok1, CtmToken & rTok2, CtmToken & rTok3, CtmToken & rTok4, CtmToken & rTok5)
{
	rTok1.Destroy();
	rTok2.Destroy();
	rTok3.Destroy();
	rTok4.Destroy();
	rTok5.Destroy();
}
//
// Stub (instead PPLIB\PPDBUTIL.CPP
//
int CallbackCompress(long, long, const char *, int)
{
	return 1;
}

%}

%union {
	long   lval;
	DLSYMBID sival;
	CtmExprConst constval;
	CtmToken token;
	CtmDclr dclr;
	CtmDclrList dclrlist;
	CtmExpr expr;
	CtmVar  var;
	SV_Uint32 ui32list;
	DlScope::IfaceBase ifcbase;
	DlScope::Attr scattr;
	char * sval;
	const  char * scval;
	S_GUID_Base uuid;
	UiCoord uipos;
	UiRelRect uirect;
}

%token T_AND
%token T_OR
%token T_EQ
%token T_NEQ
%token T_GE
%token T_LE
%token T_LBR        // '{'
%token T_RBR        // '}'
%token <token>    T_IN          // "in"
%token <token>    T_OUT         // "out"
%token <token>    T_INOUT       // "inout"
%token <token>    T_EXPORTSTR   // "data"
%token <token>    T_INTERFACE   // "interface"
%token <token>    T_ISTRUCT     // "struct"
%token <token>    T_ENUM        // "enum"
%token <token>    T_ICLASS      // "iclass"
%token <token>    T_DEFAULT     // "default"
%token <token>    T_DECLARE     // "declare"
%token <token>    T_ITERATION   // "iteration"
%token <token>    T_LINK        // "link"
%token <token>    T_IF          // "if"
%token <token>    T_ELSE        // "else"
%token <token>    T_FORMAT_FUNC // "format"
%token <token>    T_VERSION     // "version"
%token <token>    T_LIBRARY     // "library"
%token <token>    T_UUID        // "uuid"
%token <token>    T_ABSTRACT    // "abstract"
%token <token>    T_PROPERTY    // "property"
%token <token>    T_TYPE
%token <token>    T_CONST_INT
%token <token>    T_CONST_REAL
%token <token>    T_CONST_DATE
%token <token>    T_CONST_TIME
%token <token>    T_CONST_STR
%token <token>    T_CONST_UUID
%token <token>    T_IDENT
%token <token>    T_AT_IDENT    // '@' ident (���������� ������, � ������� ������ @ ��� �����)
%token <token>    T_FMT
%token <var>      T_VAR
%token <token>    T_TABLE       // "table"
%token <token>    T_INDEX       // "index"
%token <token>    T_FILE        // "file"
%token <token>    T_DIALOG
%token <token>    T_INPUT
%token <token>    T_STATICTEXT
%token <token>    T_FRAME
%token <token>    T_COMBOBOX
%token <token>    T_BUTTON
%token <token>    T_CHECKBOX
%token <token>    T_CHECKBOXCLUSTER
%token <token>    T_RADIOCLUSTER
%token <token>    T_LISTBOX
%token <token>    T_TREELISTBOX
%token            T_COLUMNS // "columns"
%token <token>    T_DESCRIPT
%token <token>    T_HANDLER

%type <sival>    parent_struc
%type <sival>    expstruc_head
%type <token>    format
%type <token>    alias
%type <constval> constant
%type <sival>    type
%type <sival>    uictrl_type
%type <sival>    uictrl_type_opt
%type <dclr>     declarator
%type <dclr>     func_declarator
%type <dclr>     actual_declarator
%type <dclrlist> declarator_list
%type <expr>     expr
%type <expr>     expr_list
%type <expr>     expr_list_nz
%type <expr>     else_expr
%type <lval>     ifunc_farg_mod_dir
%type <dclr>     ifunc_farg_declarator
%type <dclrlist> ifunc_farg_list
%type <dclrlist> ifunc_farg_list_not_empty
%type <ifcbase>  iclass_interface
%type <ui32list> iclass_interface_list
%type <lval>     iclass_interface_attr_list
%type <lval>     iclass_interface_attr
%type <lval>     version                     // LowWord - major, HiWord - minor
%type <uuid>     uuid
%type <scattr>   library_attr
%type <ui32list> library_attr_list

%type <token>    dbindex_name
%type <lval>     dbseg_flag_list

%type <uirect>   uirect
%type <uirect>   uirectopt
%type <uipos>    uiposition
%type <token>    uidescr
%type <token>    listbox_column_list
%type <token>    listbox_column_list_inner
%type <token>    listbox_column
%type <token>    h_alignment

%nonassoc IFXS
%nonassoc IFX
%nonassoc ':'
%nonassoc T_ELSE
%nonassoc T_IF
%left  '?' ','
%left  '(' ')'
%left  T_OR T_AND
%nonassoc '<' '>' T_EQ T_LE T_GE T_NEQ
%left  '|' '&'
%left '!'
%left  '+' '-'
%left  '*' '/' '%'
%left  '.'
%right '='
%nonassoc UPLUS UMINUS

%%

target_list : target_list target
| target

target      : decl_expstruc
| expstruc_head ';' {}
| decl_interface_prototype
| decl_interface
| decl_enum
| decl_struct
| decl_iclass
| decl_library
| decl_abstract
| decl_dbtable
| decl_dialog
//
//
//
version : T_VERSION '(' T_CONST_REAL ')'
{
	double i, fract = modf($3.U.FD, &i);
	$$ = MakeLong((uint)i, (uint)fract);
	ZapToken2($1, $3);
}

uuid : T_UUID '(' T_CONST_UUID ')' { $$ = $3.U.Uuid; ZapToken2($1, $3); }
//
// Abstract syntax
//
decl_abstract : T_ABSTRACT T_IDENT ';'
{
	DLSYMBID type_id = 0;
	if(DCtx.SearchSymb("void", '@', &type_id))
		type_id = DCtx.SetDeclTypeMod(type_id, STypEx::modPtr, 0);
	else {
		type_id = 0;
		DCtx.Error();
	}
	if(!DCtx.AddTypedef($2, type_id, 0))
		DCtx.Error();
	ZapToken2($1, $2);
}
//
// Library syntax
//
decl_library : T_LIBRARY T_IDENT library_attr_list T_LBR
	{
		DLSYMBID symb_id = 0;
		const char * p_name = $2.U.S;
		if(!DCtx.SearchSymb(p_name, '%', &symb_id)) {
			symb_id = DCtx.CreateSymb(p_name, '%', DlContext::crsymfCatCurScope);
			if(symb_id == 0)
				DCtx.Error();
		}
		SString name;
		DCtx.GetSymb(symb_id, name, '%');
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kLibrary, p_name, symb_id, &$3); // library {
		ZapToken2($1, $2);
		$3.Destroy();
	} library_content T_RBR
{
	DCtx.LeaveScope(); // } library
}

library_content : | library_item | library_content library_item
library_item : decl_interface | decl_enum | decl_struct | decl_iclass

library_attr_list :
{
	$$.Init();
}/* @v5.7.1 | library_attr
{
	$$.Init();
	$$.Add($1.A);
	$$.Add($1.Ver);
}*/ | library_attr_list library_attr
{
	$$.Init();
	$$.Copy($1);
	$$.Add($2.A);
	$$.Add($2.Ver);
}

library_attr : version
{
	$$.A = DlScope::sfVersion;
	$$.Ver = (uint32)$1;
} | T_IDENT
{
	if(strcmp($1.U.S, "hidden") == 0) {
		$$.A = DlScope::sfHidden;
		$$.Ver = 0;
	}
	else if(strcmp($1.U.S, "restricted") == 0) {
		$$.A = DlScope::sfRestricted;
		$$.Ver = 0;
	}
	else if(strcmp($1.U.S, "noidl") == 0) {
		$$.A = DlScope::sfNoIDL;
		$$.Ver = 0;
	}
	else {
		$$.A   = 0;
		$$.Ver = 0;
		DCtx.Error(PPERR_DL6_INVATTRLIB, $1.U.S, DlContext::erfLog);
	}
	ZapToken($1);
} | uuid
{
	$$.A = DlScope::sfUUID;
	DCtx.TempUuidList.Add(0, $1, &$$.UuidIdx);
	$$.UuidIdx++;
}
//
// Interface syntax {
//
decl_interface_prototype : T_INTERFACE T_IDENT ';'
{
	DLSYMBID symb_id = 0;
	if(!DCtx.SearchSymb($2.U.S, '@', &symb_id)) {
		symb_id = DCtx.CreateSymb($2.U.S, '@', 0);
		if(symb_id == 0)
			DCtx.Error();
		else
			DCtx.AddStructType(symb_id);
	}
	ZapToken2($1, $2);
}

decl_interface : T_INTERFACE T_IDENT library_attr_list T_LBR
	{
		DLSYMBID symb_id = 0;
		const char * p_name = $2.U.S;
		if(!DCtx.SearchSymb(p_name, '@', &symb_id)) {
			symb_id = DCtx.CreateSymb(p_name, '@', 0);
			if(symb_id == 0)
				DCtx.Error();
			else
				DCtx.AddStructType(symb_id);
		}
		SString name;
		DCtx.GetSymb(symb_id, name, '@');
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kInterface, p_name, symb_id, &$3); // interface {
		ZapToken2($1, $2);
		$3.Destroy();
	} interface_body T_RBR
{
	DCtx.LeaveScope(); // } interface
}

interface_body  : ifunc_dclr_list
ifunc_dclr_list : ifunc_dclr | ifunc_dclr_list ifunc_dclr

ifunc_dclr : type func_declarator '(' ifunc_farg_list ')' ';'
{
	$2.TypeID = $1;
	if(!DCtx.AddFuncDeclare($2, $4))
		DCtx.Error();
	//
	// ������������ ��� '���������' �� ret_type
	//
	DlContext::TypeEntry ret_te_mod;
	DLSYMBID ptr_typ = DCtx.SetDeclTypeMod($1, STypEx::modPtr);
	if(!ptr_typ || !DCtx.SearchTypeID(ptr_typ, 0, &ret_te_mod))
		DCtx.Error();
	//
	$2.Destroy();
	$4.Destroy();
} | iproperty
{
}

ifunc_farg_list : { $$.Init(); } | ifunc_farg_list_not_empty { $$ = $1; }

ifunc_farg_list_not_empty : ifunc_farg_declarator { $$.Init(); $$.Add($1); }
| ifunc_farg_list_not_empty ',' ifunc_farg_declarator { $$ = $1; $$.Add($3); }

// <dclr>
ifunc_farg_declarator : ifunc_farg_mod_dir type declarator
{
	$$ = $3;
	$$.IfaceArgDirMod = (uint16)$1;
	$$.TypeID = $2;
}

ifunc_farg_mod_dir :
{ $$ = 0; }
| T_IN    { $$ = DlFunc::fArgIn;  ZapToken($1); }
| T_OUT   { $$ = DlFunc::fArgOut; ZapToken($1); }
| T_INOUT { $$ = (DlFunc::fArgIn | DlFunc::fArgOut); ZapToken($1); }

iproperty : T_PROPERTY iproperty_entry ';'
{
	ZapToken($1);
} | T_PROPERTY T_LBR iproperty_list T_RBR
{
	ZapToken($1);
}

iproperty_list : iproperty_entry ';'
{
} | iproperty_list iproperty_entry ';'
{
}

iproperty_entry : ifunc_farg_mod_dir type declarator
{
	$3.TypeID = $2;
	if(!DCtx.AddPropDeclare($3, $1 ? $1 : (DlFunc::fArgIn | DlFunc::fArgOut)))
		DCtx.Error();
	$3.Destroy();
}
//
// } Interface syntax
// Enum syntax {
//
decl_enum : T_ENUM T_IDENT T_LBR
	{
		DLSYMBID symb_id = 0;
		const char * p_name = $2.U.S;
		if(!DCtx.SearchSymb(p_name, '@', &symb_id)) {
			symb_id = DCtx.CreateSymb(p_name, '@', /*DlContext::crsymfCatCurScope*/0);
			if(symb_id == 0)
				DCtx.Error();
			else
				DCtx.AddStructType(symb_id);
		}
		SString name;
		DCtx.GetSymb(symb_id, name, '@');
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kEnum, p_name, symb_id, 0); // enum {
		ZapToken2($1, $2);
	} enum_item_list T_RBR
{
	DCtx.LeaveScope(); // } enum
}

enum_item_list : enum_item | enum_item_list ',' enum_item

enum_item : T_IDENT { DCtx.AddEnumItem($1, 0, 0); ZapToken($1); }
| T_IDENT '='     T_CONST_INT { DCtx.AddEnumItem($1, 1, $3.U.I); ZapToken2($1, $3); }
| T_IDENT '=' '+' T_CONST_INT { DCtx.AddEnumItem($1, 1, $4.U.I); ZapToken2($1, $4); }
| T_IDENT '=' '-' T_CONST_INT { DCtx.AddEnumItem($1, 1, -$4.U.I); ZapToken2($1, $4); }
// } enum syntax
// struct syntax {
//
decl_struct : T_ISTRUCT T_IDENT T_LBR
	{
		DLSYMBID symb_id = 0;
		const char * p_name = $2.U.S;
		if(!DCtx.SearchSymb(p_name, '@', &symb_id)) {
			symb_id = DCtx.CreateSymb(p_name, '@', /*DlContext::crsymfCatCurScope*/0);
			if(symb_id == 0)
				DCtx.Error();
			else
				DCtx.AddStructType(symb_id);
		}
		SString name;
		DCtx.GetSymb(symb_id, name, '@');
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kStruct, p_name, symb_id, 0); // struct {
		ZapToken2($1, $2);
	} struct_field_list T_RBR
{
	DCtx.LeaveScope(); // } struct
}

struct_field_list : declaration_struct_field | struct_field_list declaration_exp_field
declaration_struct_field : type actual_declarator ';' { DCtx.AddDeclaration($1, $2, 0); $2.Destroy(); }
//
// } struct syntax
// iClass syntax {
//
decl_iclass : T_ICLASS T_IDENT library_attr_list ':' iclass_interface_list T_LBR
	{
		uint   i;
		SString name = $2.U.S;
		DLSYMBID symb_id = 0;
		if(!DCtx.SearchSymb(name, '@', &symb_id)) {
			symb_id = DCtx.CreateSymb(name, '@', /*DlContext::crsymfCatCurScope*/0);
			if(symb_id == 0)
				DCtx.Error();
			else
				DCtx.AddStructType(symb_id);
		}
		else
			DCtx.Error(PPERR_DL6_CLASSEXISTS, name, DlContext::erfLog | DlContext::erfExit);
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kIClass, name, symb_id, &$3);    // iclass {
		DlScope * p_scope = DCtx.GetScope(scope_id);
		if(p_scope) {
			uint   c = $5.GetCount() / 2;
			for(i = 0; i < c; i++) {
				DlScope::IfaceBase ifbase;
				ifbase.ID = $5[i*2];
				ifbase.Flags = (uint16)$5[i*2+1];
				p_scope->AddIfaceBase(&ifbase);
			}
		}
		ZapToken2($1, $2);
		$3.Destroy();
		$5.Destroy();
	} iclass_body T_RBR
{
	DCtx.LeaveScope(); // } iclass
}

iclass_interface_list : iclass_interface
{
	$$.Init();
	$$.Add($1.ID);
	$$.Add($1.Flags);
} | iclass_interface_list ',' iclass_interface
{
	$$.Init();
	$$.Copy($1);
	$$.Add($3.ID);
	$$.Add($3.Flags);
	$1.Destroy();
}

iclass_interface : T_TYPE '(' iclass_interface_attr_list ')'
{
	DLSYMBID symb_id = $1.U.ID;
	const DlScope * p_scope = DCtx.GetScope(symb_id);
	if(p_scope && p_scope->IsKind(DlScope::kInterface))
		$$.ID = symb_id;
	else {
		SString symb;
		if(p_scope)
			symb = p_scope->Name;
		else
			DCtx.GetSymb(symb_id, symb, '@');
		DCtx.Error(PPERR_DL6_ICLSPARNOTIFACE, symb, DlContext::erfLog | DlContext::erfExit);
	}
	$$.Flags = (uint16)$3;
	ZapToken($1);
} | T_TYPE
{
	DLSYMBID symb_id = $1.U.ID;
	const DlScope * p_scope = DCtx.GetScope(symb_id);
	if(p_scope && p_scope->IsKind(DlScope::kInterface))
		$$.ID = symb_id;
	else {
		SString symb;
		if(p_scope)
			symb = p_scope->Name;
		else
			DCtx.GetSymb(symb_id, symb, '@');
		DCtx.Error(PPERR_DL6_ICLSPARNOTIFACE, symb, DlContext::erfLog | DlContext::erfExit);
	}
	$$.Flags = 0;
	ZapToken($1);
}

iclass_interface_attr_list : iclass_interface_attr { $$ = $1; }
| iclass_interface_attr_list ',' iclass_interface_attr { $$ = $1 | $3; }

iclass_interface_attr : T_DEFAULT { $$ = DlScope::IfaceBase::fDefault; ZapToken($1); }

iclass_body : {}
//
// } iClass syntax
//

handler : T_HANDLER T_IDENT T_LBR expr_list T_RBR
{
}

decl_expstruc : expstruc_head parent_struc T_LBR
	{
		SString name;
		DCtx.GetSymb($1, name, '!');
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kExpData, name, $1, 0);    // data {
		DCtx.SetInheritance(scope_id, $2);
		DCtx.EnterScope(DlScope::kExpDataHdr, "hdr", 0, 0); // hdr {
	} expstruc_declare_list expstruc_body T_RBR
{
	DCtx.LeaveScope(); // } hdr
	DCtx.CompleteExportDataStruc(); // ������ ������� ������� - DlScope::kExpData. ������ ����������� ��������.
	DCtx.LeaveScope(); // } data
}

expstruc_head : T_EXPORTSTR T_IDENT
{
	DLSYMBID symb_id = 0;
	if(!DCtx.SearchSymb($2.U.S, '!', &symb_id)) {
		symb_id = DCtx.CreateSymb($2.U.S, '!', DlContext::crsymfCatCurScope);
		if(symb_id == 0)
			DCtx.Error();
		else
			DCtx.AddStructType(symb_id);
	}
	$$ = symb_id;
	ZapToken2($1, $2);
}

parent_struc :
{
	$$ = 0;
} | ':' T_IDENT
{
	DLSYMBID symb_id = 0;
	if(DCtx.SearchSymb($2.U.S, '!', &symb_id))
		$$ = symb_id;
	else
		DCtx.Error();
	ZapToken($2);
}

expstruc_declare_list : | expstruc_declare_statement | expstruc_declare_list expstruc_declare_statement
{
}

expstruc_declare_statement : T_DECLARE T_CONST_STR
{
	DCtx.AddStrucDeclare($2.U.S);
	ZapToken2($1, $2);
}

expstruc_body : expstruc_field_list expstruc_iter_list
{
}

expstruc_iter_list : | expstruc_iter_list expstruc_iter
{
}

expstruc_iter : T_ITERATION T_LBR // ������������� ��������
	{
		DCtx.PushScope();
		DCtx.EnterScope(DlScope::kExpDataIter, "iter@def", 0, 0);
		ZapToken($1);
	} expstruc_field_list T_RBR
{
	DCtx.LeaveScope();
	DCtx.PopScope();
} | T_ITERATION T_IDENT T_LBR // ����������� ��������
	{
		SString temp_buf;
		(temp_buf = "iter").CatChar('@').Cat($2.U.S);
		DCtx.PushScope();
		DCtx.EnterScope(DlScope::kExpDataIter, temp_buf, 0, 0);
		ZapToken2($1, $2);
	} expstruc_field_list T_RBR
{
	DCtx.LeaveScope();
	DCtx.PopScope();
}
//
// ������ �����.
//
expstruc_field_list : | declaration_exp_field | expstruc_field_list declaration_exp_field

declaration_exp_field : type actual_declarator alias format ';'
{
	$2.Alias  = $3;
	$2.Format = $4;
	DCtx.AddDeclaration($1, $2, 0);
	$2.Destroy();
} | actual_declarator '=' expr alias format ';'
{
	$1.Alias  = $4;
	$1.Format = $5;
	DCtx.AddDeclaration(0, $1, &$3);
	$1.Destroy();
} | type actual_declarator '(' ifunc_farg_list ')' ';' // Function declaration
{
	$2.TypeID = $1;
	if(!DCtx.AddFuncDeclare($2, $4))
		DCtx.Error();
	$2.Destroy();
	$4.Destroy();
} | handler
{
}

alias  : { $$.Create(0); } | T_AT_IDENT { $$ = $1; }
format : { $$.Create(0); } | T_FMT { $$ = $1; }

declaration : type declarator_list ';'
{
	CtmDclrList & r_list = $2;
	if(r_list.P_List)
		for(uint i = 0; i < r_list.P_List->getCount(); i++) {
			CtmDclr * p_dclr = r_list.P_List->at(i);
			if(p_dclr)
				DCtx.AddDeclaration($1, *p_dclr, 0);
		}
	$2.Destroy();
}

declarator_list : declarator     { $$.Init(); $$.Add($1); $1.Destroy(); }
| declarator_list ',' declarator { $$ = $1; $$.Add($3); $3.Destroy(); }

declarator : '&' declarator { $$ = $2; $$.AddPtrMod(STypEx::modRef, 0); }
| '*' declarator { $$ = $2; $$.AddPtrMod(STypEx::modPtr, 0); }
| actual_declarator { $$ = $1; }

actual_declarator : T_IDENT             { $$.Init(); $$.Tok = $1; }
| '(' declarator ')'                    { $$ = $2; }
| actual_declarator '[' T_CONST_INT ']' { $$ = $1; $$.AddDim($3.U.I); ZapToken($3); }
| actual_declarator '[' T_CONST_INT '.' T_CONST_INT ']' { $$ = $1; $$.AddDecimalDim($3.U.I, $5.U.I); ZapToken2($3, $5); }
| actual_declarator '[' ']'             { $$ = $1; $$.AddDim(0); }

func_declarator : T_IDENT { $$.Init(); $$.Tok = $1; }
| '&' func_declarator { $$ = $2; $$.AddPtrMod(STypEx::modRef, 0); }
| '*' func_declarator { $$ = $2; $$.AddPtrMod(STypEx::modPtr, 0); /* ### */ }

type : T_TYPE
{
	$$ = DCtx.SetDeclType($1.U.ID);
	if($$ == 0)
		DCtx.Error();
	ZapToken($1);
} | T_LINK T_IDENT
{
	DLSYMBID symb_id = 0;
	if(DCtx.SearchSymb($2.U.S, '!', &symb_id))
		$$ = DCtx.SetDeclTypeMod(symb_id, STypEx::modLink, 0);
	else {
		$$ = 0;
		DCtx.Error();
	}
	ZapToken2($1, $2);
}

expr : constant { $$.Init($1); }
| T_IDENT       { $$.InitVar($1.U.S); ZapToken($1); }
//| T_VAR         { $$.InitVar($1); }
//| T_IDENT '(' expr_list ')' { $$.InitFuncCall($1.U.S, $3); ZapToken($1); }
| T_IDENT '(' expr_list ')' { $$.InitFuncCall($1.U.S, $3); ZapToken($1); }
| expr '.' expr { $$.InitBinaryOp(dlopDot, $1, $3); }
| '(' expr ')'          { $$ = $2; }
| '(' T_TYPE ')' expr   { $$.InitTypeConversion($4, $2.U.ID); ZapToken($2); }
| '+' expr %prec UPLUS  { $$.InitUnaryOp(dlopUnaryPlus,  $2); }
| '-' expr %prec UMINUS { $$.InitUnaryOp(dlopUnaryMinus, $2); }
| expr '+' expr   { $$.InitBinaryOp(dlopAdd, $1, $3); }
| expr '-' expr   { $$.InitBinaryOp(dlopSub, $1, $3); }
| expr '*' expr   { $$.InitBinaryOp(dlopMul, $1, $3); }
| expr '/' expr   { $$.InitBinaryOp(dlopDiv, $1, $3); }
| expr '%' expr   { $$.InitBinaryOp(dlopMod, $1, $3); }
| expr T_AND expr { $$.InitBinaryOp(dlopAnd, $1, $3); }
| expr T_OR expr  { $$.InitBinaryOp(dlopOr, $1, $3);  }
| '!' expr        { $$.InitUnaryOp(dlopNot, $2);      }
| expr '&' expr   { $$.InitBinaryOp(dlopBwAnd, $1, $3); }
| expr '|' expr   { $$.InitBinaryOp(dlopBwOr, $1, $3);  }
| expr T_EQ expr  { $$.InitBinaryOp(dlopEq, $1, $3);    }
| expr T_NEQ expr { $$.InitBinaryOp(dlopNeq, $1, $3);   }
| expr '<' expr   { $$.InitBinaryOp(dlopLt, $1, $3);    }
| expr '>' expr   { $$.InitBinaryOp(dlopGt, $1, $3);    }
| expr T_LE expr  { $$.InitBinaryOp(dlopLe, $1, $3);    }
| expr T_GE expr  { $$.InitBinaryOp(dlopGe, $1, $3);    }
| '@' '(' T_IDENT ',' expr ')'
{
	DLSYMBID symb_id = 0;
	DLSYMBID type_id = 0;
	if(DCtx.SearchSymb($3.U.S, '!', &symb_id))
		type_id = DCtx.SetDeclTypeMod(symb_id, STypEx::modLink, 0);
	else
		DCtx.Error();
	$$.InitRefOp(type_id, $5);
	ZapToken($3);
}
| expr '?' expr ':' expr %prec IFXS
{
	$$.Init(CtmExpr::kOp);
	$$.U.Op = dlopQuest;
	$$.AddArg($1);
	$$.AddArg($3);
	$$.AddArg($5);
} | T_IF '(' expr ')' expr else_expr
{
	$$.Init(CtmExpr::kOp);
	$$.U.Op = dlopQuest;
	$$.AddArg($3);
	$$.AddArg($5);
	if($6.GetKind() != CtmExpr::kEmpty)
		$$.AddArg($6);
} | T_FORMAT_FUNC '(' expr ',' T_FMT ')'
{
	CtmExprConst c;
	CtmExpr fmt_str;
	SString val(1); // (1) �������, ��� (const char *)val != 0.
	val = $5.U.S;
	DCtx.AddConst(val, &c);
	fmt_str.Init(c);
	$3.Append(fmt_str);
	$$.InitFuncCall("format", $3);
	ZapToken2($1, $5);
}

else_expr : { $$.Init(CtmExpr::kEmpty); } | T_ELSE expr { $$ = $2; ZapToken($1); }
//
//
//
constant : T_CONST_REAL
{
	double val = $1.U.FD;
	DCtx.AddConst(val, &$$);
	ZapToken($1);
} | T_CONST_INT
{
	int32  val = $1.U.I;
	DCtx.AddConst(val, &$$);
	ZapToken($1);
} | T_CONST_STR
{
	SString val(1); // (1) ��� �����������, ��� (const char *)val != 0.
	val = $1.U.S;
	DCtx.AddConst(val, &$$);
	ZapToken($1);
} | T_CONST_DATE
{
	LDATE  val = $1.U.D;
	DCtx.AddConst(val, &$$);
	ZapToken($1);
} | T_CONST_TIME
{
	LTIME  val = $1.U.T;
	DCtx.AddConst(val, &$$);
	ZapToken($1);
}

expr_list    : { $$.Init(); } | expr_list_nz { $$ = $1; }
expr_list_nz : expr { $$ = $1; } | expr ',' expr_list_nz { $$ = $1; $$.Append($3); }
//
// Database declarations
//
decl_dbtable : T_TABLE T_IDENT T_LBR
	{
		DLSYMBID symb_id = 0;
		const char * p_name = $2.U.S;
		if(!DCtx.SearchSymb(p_name, '$', &symb_id)) {
			symb_id = DCtx.CreateSymb(p_name, '$', DlContext::crsymfErrorOnDup);
			if(symb_id == 0)
				DCtx.Error();
		}
		SString name;
		DCtx.GetSymb(symb_id, name, '$');
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kDbTable, name, symb_id, 0); // table {
		ZapToken2($1, $2);
	} dbfield_list dbtable_addon_declaration_list T_RBR
{
	// DCtx.CompleteDbTable();
	DCtx.LeaveScope(); // } table
}

dbfield_list : dbfield | dbfield_list dbfield

dbfield : type declarator ';'
{
	DCtx.AddDeclaration($1, $2, 0);
	$2.Destroy();
}
//
//
//
dbtable_addon_declaration_list : dbtable_addon_declaration | dbtable_addon_declaration_list dbtable_addon_declaration
dbtable_addon_declaration      : | T_INDEX ':' dbindex_list {} | T_FILE ':' dbfile_definition_list {}
dbindex_list                   : dbindex_def | dbindex_list dbindex_def

dbindex_def : /*dbindex_name*/
	{
		SString temp_buf;
		DCtx.InitDbIndexName(/*$1.U.S*/0, temp_buf);
		DCtx.EnterScope(DlScope::kDbIndex, temp_buf, 0, 0);
		//ZapToken($1);
	} dbindex_seg_list '(' dbindex_flag_list ')' ';'
{
	DCtx.LeaveScope();
}

dbindex_seg_list : dbindex_seg | dbindex_seg_list ',' dbindex_seg

dbindex_seg : T_IDENT dbseg_flag_list
{
	DCtx.AddDbIndexSegmentDeclaration($1.U.S, $2);
	ZapToken($1);
}
//
// ������������ ������� (������������)
// <token>
//
dbindex_name : { $$.Create(0); } | T_IDENT '=' { $$ = $1; }
//
// ������ ����� �������� ������� ������� ���� ������
// <lval>
//
dbseg_flag_list :
{
	$$ = 0;
} | dbseg_flag_list T_IDENT
{
	DCtx.ResolveDbIndexSegFlag($1, $2.U.S, &$$);
	ZapToken($2);
}
//
// ������ ����� ������� ������� ���� ������
// <lval>
//
dbindex_flag_list :
{
} | dbindex_flag_list T_IDENT
{
	DCtx.ResolveDbIndexFlag($2.U.S);
	ZapToken($2);
}

dbfile_definition_list : | dbfile_definition | dbfile_definition_list dbfile_definition

dbfile_definition : T_CONST_STR ';'
{
	CtmToken temp_token;
	temp_token.Create(0);
	DCtx.ResolveDbFileDefinition(temp_token, $1.U.S, 0);
	ZapToken2($1, temp_token);
} | T_IDENT '=' T_CONST_STR ';'
{
	DCtx.ResolveDbFileDefinition($1, $3.U.S, 0);
	ZapToken2($1, $3);
} | T_IDENT '=' T_CONST_INT ';'
{
	DCtx.ResolveDbFileDefinition($1, 0, $3.U.I);
	ZapToken2($1, $3);
} | T_IDENT ';'
{
	DCtx.ResolveDbFileDefinition($1, 0, 0);
	ZapToken($1);
}
//
// UI
//
/* @construction */

uiposition : T_CONST_INT { $$.Set($1.U.I, UiCoord::dfAbs); }

uirect : '(' uiposition ',' uiposition ',' uiposition ',' uiposition ')'
{
	$$.L.X = $2;
	$$.L.Y = $4;
	$$.R.X = $6;
	$$.R.Y = $8;
}

uirectopt : { $$.Reset(); } | uirect { $$ = $1; }

uidescr :
{
	$$.Create(0);
} | T_DESCRIPT T_CONST_STR
{
	$$ = $2;
	ZapToken($1);
} | T_DESCRIPT '{' T_CONST_STR '}'
{
	$$ = $3;
	ZapToken($1);
}

decl_dialog : T_DIALOG T_IDENT T_CONST_STR uirectopt uictrl_properties T_LBR
	{
		SString name = $2.U.S;
		SString text = $3.U.S;
		DLSYMBID symb_id = 0;
		if(!DCtx.SearchSymb(name, '@', &symb_id)) {
			symb_id = DCtx.CreateSymb(name, '@', 0);
			if(symb_id == 0)
				DCtx.Error();
			else
				DCtx.AddStructType(symb_id);
		}
		else
			DCtx.Error(PPERR_DL6_CLASSEXISTS, name, DlContext::erfLog | DlContext::erfExit);
		DLSYMBID scope_id = DCtx.EnterScope(DlScope::kUiDialog, name, symb_id, 0);  // dialog {
		{
			UiRelRect & r_rect = $4;
			DlScope * p_scope = DCtx.GetScope(scope_id, 0);
			if(p_scope) {
				p_scope->AcceptTempFldConstList(0);
				if(!r_rect.IsEmpty()) {
					CtmExprConst c;
					DCtx.AddConst(&r_rect, sizeof(r_rect), &c);
					p_scope->AddConst(DlScope::cuifCtrlRect, c, 1);
				}
				//
				if(text.NotEmptyS()) {
					CtmExprConst c;
					DCtx.AddConst(text, &c);
					p_scope->AddConst(DlScope::cuifCtrlText, c, 1);
				}
			}
		}
		ZapToken3($1, $2, $3);
	} dialogbody T_RBR
{
	DCtx.LeaveScope(); // } dialog
}

dialogbody        : uictrl {} | dialogbody uictrl {}
uictrl_properties : {} | T_PROPERTY T_LBR uictrl_prop_list T_RBR {}
uictrl_prop_list  : uictrl_prop {} | uictrl_prop_list uictrl_prop {}

uictrl_prop : T_IDENT
{
	DCtx.AddTempFldProp($1, (long)1);
	ZapToken($1);
} | T_IDENT '=' T_CONST_STR
{
	DCtx.AddTempFldProp($1, $3.U.S);
	ZapToken2($1, $3);
} | T_IDENT '=' T_CONST_INT
{
	DCtx.AddTempFldProp($1, (long)$3.U.I);
	ZapToken2($1, $3);
} | T_IDENT '=' T_CONST_REAL
{
	DCtx.AddTempFldProp($1, $3.U.FD);
	ZapToken2($1, $3);
} | T_IDENT '=' uirect
{
	DCtx.AddTempFldProp($1, &$3, sizeof($3));
	ZapToken($1);
}

uictrl_type : T_TYPE
{
	$$ = DCtx.SetDeclType($1.U.ID);
	if($$ == 0)
		DCtx.Error();
	// @todo money (default dim)
	ZapToken($1);
} | T_TYPE '[' T_CONST_INT ']'
{
	$$ = DCtx.SetDeclType($1.U.ID);
	if($$) {
		uint32 dim = $3.U.I;
		$$ = DCtx.SetDeclTypeMod($$, STypEx::modArray, dim);
	}
	if($$ == 0)
		DCtx.Error();
	ZapToken2($1, $3);
} | T_TYPE '[' T_CONST_INT '.' T_CONST_INT ']'
{
	$$ = DCtx.SetDeclType($1.U.ID);
	if($$) {
		uint32 dim = SetDecimalDim($3.U.I, $5.U.I);
		$$ = DCtx.SetDeclTypeMod($$, STypEx::modArray, dim);
	}
	if($$ == 0)
		DCtx.Error();
	ZapToken3($1, $3, $5);
}

uictrl_type_opt : uictrl_type { $$ = $1 } | { $$ = 0; }

uictrl :
{
} | T_INPUT T_IDENT T_CONST_STR uirectopt uictrl_type uictrl_properties ';'
{
	DCtx.AddUiCtrl(DlScope::ckInput, $2, $3, $5, $4);
	ZapToken3($1, $2, $3);
} | T_STATICTEXT T_IDENT T_CONST_STR uirectopt uictrl_properties ';'
{
	DCtx.AddUiCtrl(DlScope::ckStatic, $2, $3, 0, $4);
	ZapToken3($1, $2, $3);
} | T_STATICTEXT T_CONST_STR uirectopt uictrl_properties ';'
{
	CtmToken temp_token;
	temp_token.Create(0, "");
	DCtx.AddUiCtrl(DlScope::ckStatic, temp_token, $2, 0, $3);
	ZapToken2($1, $2);
	temp_token.Destroy();
} | T_COMBOBOX T_IDENT T_CONST_STR uirectopt uictrl_type_opt uictrl_properties ';'
{
	DCtx.AddUiCtrl(DlScope::ckCombobox, $2, $3, $5, $4);
	ZapToken3($1, $2, $3);
} | T_BUTTON T_IDENT T_CONST_STR uirectopt T_IDENT uictrl_properties ';'
{
	//
	// $1 button (keyword)
	// $2 ctl_ident
	// $3 text
	// $4 coordinates
	// $5 cmd_ident
	// $6 properties
	//
	DCtx.AddUiButton($2, $3, $4, $5);
	ZapToken4($1, $2, $3, $5);
} | T_CHECKBOX T_IDENT T_CONST_STR uirectopt uictrl_type_opt uictrl_properties ';'
{
	DLSYMBID type_id = $5;
	if(type_id == 0) {
		DCtx.SearchSymb("uint16", '@', &type_id);
	}
	DCtx.AddUiCtrl(DlScope::ckCheckbox, $2, $3, type_id, $4);
	ZapToken3($1, $2, $3);
} | T_CHECKBOXCLUSTER T_IDENT T_CONST_STR uirectopt uictrl_type_opt uictrl_properties T_LBR {
		if(!DCtx.AddUiCluster(DlScope::ckCheckCluster, $2, $3, $5, $4))
			DCtx.Error();
		ZapToken3($1, $2, $3);
	}  ui_cluster_item_list T_RBR
{
	DCtx.LeaveScope(); // } checkboxcluster
} | T_RADIOCLUSTER T_IDENT T_CONST_STR uirectopt uictrl_type_opt uictrl_properties T_LBR {
		if(!DCtx.AddUiCluster(DlScope::ckRadioCluster, $2, $3, $5, $4))
			DCtx.Error();
		ZapToken3($1, $2, $3);
	}
	ui_cluster_item_list T_RBR
{
	DCtx.LeaveScope(); // } radiobuttoncluster
} | T_LISTBOX T_IDENT T_CONST_STR uirectopt uictrl_properties listbox_column_list
{
	// ListBox definition
	DCtx.AddUiListbox($2, $3, $4, $6);
	ZapToken4($1, $2, $3, $6);
} | T_TREELISTBOX T_IDENT T_CONST_STR uirectopt uictrl_properties ';'
{
	// TreeListBox definition
	DCtx.AddUiCtrl(DlScope::ckTreeListbox, $2, $3, 0, $4);
	ZapToken3($1, $2, $3);
} | T_FRAME T_IDENT T_CONST_STR uirectopt uictrl_properties ';'
{
	DCtx.AddUiCtrl(DlScope::ckFrame, $2, $3, 0, $4);
	ZapToken3($1, $2, $3);
} | T_FRAME T_CONST_STR uirectopt uictrl_properties ';'
{
	CtmToken temp_token;
	temp_token.Create(0, "");
	DCtx.AddUiCtrl(DlScope::ckFrame, temp_token, $2, 0, $3);
	ZapToken2($1, $2);
	temp_token.Destroy();
}

ui_cluster_item : T_CONST_STR uirectopt uidescr ';'
{
	if(!DCtx.AddUiClusterItem($1, $2, $3))
		DCtx.Error();
	ZapToken2($1, $3);
}

ui_cluster_item_list : {} | ui_cluster_item {} | ui_cluster_item_list ui_cluster_item {}

listbox_column_list : ';'
{
	// listbox_column_list (empty)
	CtmToken temp_token;
	temp_token.Create(0, "");
	$$ = temp_token;
} | T_COLUMNS T_LBR listbox_column_list_inner T_RBR
{
	// listbox_column_list (full syntax)
	$$ = $3;
} | T_COLUMNS T_CONST_STR ';'
{
	// listbox_column_list (one string description)
	$$ = $2;
}

listbox_column_list_inner : listbox_column
{
	$$ = $1;
} | listbox_column_list_inner listbox_column
{
	SString column_descr;
	column_descr = $1.U.S;
	if(column_descr.NotEmptyS())
		column_descr.Semicol().Cat($2.U.S);
	CtmToken temp_token;
	temp_token.Create(0, column_descr);
	$$ = temp_token;

	ZapToken2($1, $2);
}

listbox_column : T_CONST_STR T_CONST_INT h_alignment ';'
{
	SString column_descr;
	(column_descr = $1.U.S).Comma().Cat($3.U.S).Comma().Cat($2.U.I);

	CtmToken temp_token;
	temp_token.Create(0, column_descr);
	$$ = temp_token;
	ZapToken3($1, $2, $3);
} | T_CONST_STR ';'
{
	SString column_descr;
	(column_descr = $1.U.S).Comma().CatChar('L').Comma().Cat(10);

	CtmToken temp_token;
	temp_token.Create(0, column_descr);
	$$ = temp_token;
	ZapToken($1);
}

h_alignment :
{
	CtmToken temp_token;
	temp_token.Create(0, "L");
	$$ = temp_token;
} | T_IDENT
{
	CtmToken temp_token;
	if(sstreqi_ascii($1.U.S, "left"))
		temp_token.Create(0, "L");
	else if(sstreqi_ascii($1.U.S, "right"))
		temp_token.Create(0, "R");
	else if(sstreqi_ascii($1.U.S, "center"))
		temp_token.Create(0, "C");
	else
		temp_token.Create(0, "L");
	$$ = temp_token;
	ZapToken($1);
}

/* @construction */

%%

//
// /ob
// /d
//

int main(int argc, char * argv[])
{
	if(argc < 2) {
		SString msg_buf;
		msg_buf.Cat("Usage: dl600c [options] input_file_name").CR();
		msg_buf.Cat("Options:").CR();
		msg_buf.CatChar('\t').Cat("/ob").CatDiv('-', 1).Cat("output only binary file (no source modules)").CR();
		msg_buf.CatChar('\t').Cat("/d").CatDiv('-', 1).Cat("debug mode").CR();
		msg_buf.CatChar('\t').Cat("/sql").CatDiv('-', 1).Cat("generate sql-script for creating database structure").CR();
		msg_buf.CatChar('\t').Cat("/oracle").CatDiv('-', 1).Cat("generate oracle specific sql-script for creating database structure").CR();
		msg_buf.CatChar('\t').Cat("/dict:path").CatDiv('-', 1).Cat("path to database dictionary (btrieve)").CR();
		msg_buf.CatChar('\t').Cat("/data:path").CatDiv('-', 1).Cat("path to database directory (btrieve)").CR();
		printf(msg_buf.cptr());
		return -1;
	}
	else {
		SLS.Init("DL600C");
#ifdef _DEBUG
		//yydebug = 0;
#endif
		long   cflags = 0;
		SString dict_path;
		SString data_path;
		SString arg, filename, left, right;
		for(int i = 1; i < argc; i++) {
			(arg = argv[i]).Strip();
			if(arg.CmpNC("/ob") == 0)
				cflags |= DlContext::cfBinOnly;
			else if(arg.CmpNC("/d") == 0)
				cflags |= DlContext::cfDebug;
			else if(arg.CmpNC("/sql") == 0)
				cflags |= DlContext::cfSQL;
			else if(arg.CmpNC("/oracle") == 0 || arg.CmpNC("/ora") == 0)
				cflags |= DlContext::cfOracle;
			else if(arg.CmpPrefix("/dict", 1) == 0) {
				if(arg.Divide(':', left, right) > 0) {
					dict_path = right;
					if(!pathValid(dict_path, 1)) {
						(left = 0).Cat("Error: invalid path").CatChar('\'').Cat(dict_path).CatChar('\'').CR();
						printf(left.cptr());
						return 1;
					}
				}
				else {
					(left = 0).Cat("Error: dictionary path must be specified (/dict:path)").CR();
					return 1;
				}
			}
			else if(arg.CmpPrefix("/data", 1) == 0) {
				if(arg.Divide(':', left, right) > 0) {
					data_path = right;
					if(!pathValid(data_path, 1)) {
						(left = 0).Cat("Error: invalid path").CatChar('\'').Cat(data_path).CatChar('\'').CR();
						printf(left.cptr());
						return 1;
					}
				}
				else {
					(left = 0).Cat("Error: database path must be specified (/data:path)").CR();
					return 1;
				}
			}
			else if(arg.C(0) == '/')
				printf("Warning: unknown option '%s'\n", argv[i]);
			else {
				filename = arg;
				if(!::fileExists(filename)) {
					printf("Error: file '%s' is not exists\n", filename.cptr());
					return 1;
				}
			}
		}
		if(filename.NotEmptyS())
			return DCtx.Compile(filename, dict_path, data_path, cflags) ? 0 : 1;
		else {
			printf("Error: undefined input file name\n");
			return 1;
		}
	}
}
