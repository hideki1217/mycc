#pragma once

#define S_ADD "+"
#define S_SUB "-"
#define S_MUL "*"
#define S_DIV "/"
#define S_MOD "%"
// #define S_DEREF "*"
// #define S_REF "&"
#define S_NOT "~"
#define S_AND "&"
#define S_OR "|"
#define S_XOR "^"
#define S_USHFT "<<"
#define S_DSHFT ">>"
#define S_INSERT "="

#define S_ADD_ASGN "+="
#define S_SUB_ASGN "-="
#define S_MUL_ASGN "*="
#define S_DIV_ASGN "/="
#define S_MOD_ASGN "%="
#define S_AND_ASGN "&="
#define S_OR_ASGN "|="
#define S_XOR_ASGN "^="
#define S_USHFT_ASGN "<<="
#define S_DSHFT_ASGN ">>="

#define S_LESS_THN "<"
#define S_GRTR_THN ">"
#define S_OR_LESS "<="
#define S_OR_GRTR ">="
#define S_EQL "=="
#define S_NOT_EQL "!="

#define S_LGNOT "!"
#define S_LGAND "&&"
#define S_LGOR "||"

#define S_L0 "("
#define S_R0 ")"
#define S_L1 "["
#define S_R1 "]"
#define S_L2 "{"
#define S_R2 "}"

#define S_C0 ","
#define S_C1 ":"
#define S_C2 ";"

#define S_LINE_COM "//"
#define S_BLCK_COML "/*"
#define S_BLCK_COMR "*/"

#define S_IF "if"
#define S_ELSE "else"
#define S_DO "do"
#define S_GOTO "goto"
#define S_SWITCH "switch"
#define S_WHILE "while"
#define S_FOR "for"

#define S_CONTNE "continue"
#define S_BREAK "break"
#define S_RETURN "return"

#define S_CASE "case"
#define S_DFLT "default"

#define S_STRCT "struct"
#define S_UNION "union"
#define S_ENUM "enum"

#define S_VOID "void"
#define S_CHAR "char"
#define S_SHRT "short"
#define S_INT "int"
#define S_LONG "long"

#define S_FLOAT "float"
#define S_DOBLE "double"

#define S_TYPEDEF "typedef"
#define S_STATIC "static"
#define S_EXTERN "extern"

#define S_CONST "const"
#define S_VLTLE "volatile"

#define S_PP_SYMBL "#"
// #define S_PP_TOSTR "#"
#define S_PP_CONCAT "##"
#define S_PP_DEF "define"
#define S_PP_IF "if"
#define S_PP_IFDEF "ifdef"
#define S_PP_IFNDEF "ifndef"
#define S_PP_ELIF "elif"
#define S_PP_ELSE "else"
#define S_PP_ENDIF "endif"
#define S_PP_PRAGMA "pragma"
#define S_PP_UNDEF "undef"
#define S_PP_INCLUDE "include"

 
typedef enum ID {
ID_ADD,
ID_SUB,
ID_MUL,
ID_DIV,
ID_MOD,
// ID_DEREF,
// ID_REF,
ID_NOT,
ID_AND,
ID_OR,
ID_XOR,
ID_USHFT,
ID_DSHFT,
ID_INSERT,

ID_ADD_ASGN,
ID_SUB_ASGN,
ID_MUL_ASGN,
ID_DIV_ASGN,
ID_MOD_ASGN,
ID_AND_ASGN,
ID_OR_ASGN,
ID_XOR_ASGN,
ID_USHFT_ASGN,
ID_DSHFT_ASGN,

ID_LESS_THN,
ID_GRTR_THN,
ID_OR_LESS,
ID_OR_GRTR,
ID_EQL,
ID_NOT_EQL,

ID_LGNOT,
ID_LGAND,
ID_LGOR,

ID_L0,
ID_R0,
ID_L1,
ID_R1,
ID_L2,
ID_R2,

ID_C0,
ID_C1,
ID_C2,

ID_IF,
ID_ELSE,
ID_DO,
ID_GOTO,
ID_SWITCH,
ID_WHILE,
ID_FOR,

ID_CONTNE,
ID_BREAK,
ID_RETURN,

ID_CASE,
ID_DFLT,

ID_STRCT,
ID_UNION,
ID_ENUM,

ID_VOID,
ID_CHAR,
ID_SHRT,
ID_INT,
ID_LONG,

ID_FLOAT,
ID_DOBLE,

ID_TYPEDEF,
ID_STATIC,
ID_EXTERN,

ID_CONST,
ID_VLTLE,

ID_IDENT,
ID_CONST_INT,
ID_CONST_FLOAT,
ID_CONST_CHAR,
ID_STR,

ID_PP_SYMBL,
// ID_PP_TOSTR,
ID_PP_CONCAT,
ID_PP_DEF,
ID_PP_IF,
ID_PP_IFDEF,
ID_PP_IFNDEF,
ID_PP_ELIF,
ID_PP_ELSE,
ID_PP_ENDIF,
ID_PP_PRAGMA,
ID_PP_UNDEF,
ID_PP_INCLUDE,
} IDs;