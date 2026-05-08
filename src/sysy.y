%code requires {
    #include "ast.h"
}

%{
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "ast.h"

using namespace std;

int yylex();
void yyerror(BaseAST*& ast, const char* s);
%}

%parse-param { BaseAST*& ast }

/* 关键：必须是 POD 类型 */
%union {
    BaseAST* ast_val;
    std::vector<BaseAST*>* vec_val;
    char* str_val;
    int int_val;
}

/* tokens */
%token INT FLOAT_T RETURN
%token <str_val> IDENT FLOAT_CONST
%token <int_val> INT_CONST

%type <vec_val> StmtList
%type <ast_val> CompUnit Stmt Expr Number Block FuncDef


%left '+' '-'
%left '*' '/'
%right UMINUS

%%

CompUnit
    : FuncDef {
        auto node = new CompUnitAST();
        node->func = $1;
        ast = node;  
    }
    ;

FuncDef
    : INT IDENT '(' ')' Block {
        auto node = new FuncDefAST();
        node->name = std::string($2);
        free($2);
        node->block = $5;
        $$ = node;
    }
    ;

Block
    : '{' StmtList '}' {
        auto node = new BlockAST();
        node->stmts = *$2;
        delete $2;
        $$ = node;
    }
    ;

StmtList
    : StmtList Stmt {
        $1->push_back($2);
        $$ = $1;
    }
    | Stmt {
        auto v = new std::vector<BaseAST*>();
        v->push_back($1);
        $$ = v;
    }
    ;

Stmt
    : RETURN Expr ';' {
        auto node = new ReturnAST();
        node->expr = $2;
        $$ = node;
    }
    ;

Expr
    : Expr '+' Expr { $$ = new BinaryAST("+", $1, $3); }
    | Expr '-' Expr { $$ = new BinaryAST("-", $1, $3); }
    | Expr '*' Expr { $$ = new BinaryAST("*", $1, $3); }
    | Expr '/' Expr { $$ = new BinaryAST("/", $1, $3); }
    | '(' Expr ')'  { $$ = $2; }
    | '-' Expr %prec UMINUS { $$ = new UnaryAST("-", $2); }
    | Number { $$ = $1; }
    ;

Number
    : INT_CONST { $$ = new NumberAST($1); }
    | FLOAT_CONST { $$ = new NumberAST(atof($1)); free($1); }
    ;

%%

void yyerror(BaseAST*& ast, const char* s) {
    cerr << "error: " << s << endl;
}