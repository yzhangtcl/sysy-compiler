%code requires {
  #include "ast.h"
  #include <memory>
  #include <string>
  #include <vector>
}

%{

#include "ast.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  double float_val;
  ValueType type_val;
  BaseAST *ast_val;
  std::vector<BaseAST *> *ast_list;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT FLOAT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST
%token <float_val> FLOAT_CONST
%token LE GE EQ NE AND OR

// 非终结符的类型定义
%type <ast_val> FuncDef Block BlockItem Decl ConstDecl VarDecl ConstDef VarDef
%type <ast_val> Stmt MatchedStmt OpenStmt Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> InitVal ConstInitVal LVal FuncFParam
%type <ast_list> BlockItems ConstDefList VarDefList FuncFParams FuncRParams InitValList ConstInitValList
%type <ast_list> ConstExpList ExpList
%type <int_val> Number
%type <type_val> BType

%%

// CompUnit ::= [CompUnit] (Decl | FuncDef);
// 顶层可包含多个全局声明和函数定义
CompUnit
  : {
    auto comp_unit = std::make_unique<CompUnitAST>();
    ast = std::move(comp_unit);
  }
  | CompUnit Decl {
    auto *comp_unit = dynamic_cast<CompUnitAST *>(ast.get());
    assert(comp_unit != nullptr);
    comp_unit->items.emplace_back($2);
  }
  | CompUnit FuncDef {
    auto *comp_unit = dynamic_cast<CompUnitAST *>(ast.get());
    assert(comp_unit != nullptr);
    comp_unit->items.emplace_back($2);
  }
  ;

// FuncDef ::= FuncType IDENT '(' [FuncFParams] ')' Block;
// FuncType 直接展开为 BType | VOID, 避免与 Decl 产生 reduce/reduce 冲突
FuncDef
  : BType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    auto type_ast = new FuncTypeAST();
    type_ast->value_type = $1;
    type_ast->name = ($1 == ValueType::Int) ? "int" : "float";
    ast->func_type = std::unique_ptr<BaseAST>(type_ast);
    ast->ident = *std::unique_ptr<std::string>($2);
    ast->block = std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BType IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    auto type_ast = new FuncTypeAST();
    type_ast->value_type = $1;
    type_ast->name = ($1 == ValueType::Int) ? "int" : "float";
    ast->func_type = std::unique_ptr<BaseAST>(type_ast);
    ast->ident = *std::unique_ptr<std::string>($2);
    for (auto *param : *$4) {
      ast->params.emplace_back(param);
    }
    delete $4;
    ast->block = std::unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  | VOID IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    auto type_ast = new FuncTypeAST();
    type_ast->value_type = ValueType::Void;
    type_ast->name = "void";
    ast->func_type = std::unique_ptr<BaseAST>(type_ast);
    ast->ident = *std::unique_ptr<std::string>($2);
    ast->block = std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | VOID IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    auto type_ast = new FuncTypeAST();
    type_ast->value_type = ValueType::Void;
    type_ast->name = "void";
    ast->func_type = std::unique_ptr<BaseAST>(type_ast);
    ast->ident = *std::unique_ptr<std::string>($2);
    for (auto *param : *$4) {
      ast->params.emplace_back(param);
    }
    delete $4;
    ast->block = std::unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;

// BType ::= "int" | "float";

// FuncFParams ::= FuncFParam {"," FuncFParam};
FuncFParams
  : FuncFParam {
    auto params = new std::vector<BaseAST *>();
    params->push_back($1);
    $$ = params;
  }
  | FuncFParams ',' FuncFParam {
    $1->push_back($3);
    $$ = $1;
  }
  ;

// FuncFParam ::= BType IDENT ["[" "]" {"[" ConstExp "]"}];
// 支持数组参数: int a, int a[], int a[][3], int a[][3][4]
FuncFParam
  : BType IDENT {
    auto ast = new FuncFParamAST();
    ast->value_type = $1;
    ast->ident = *std::unique_ptr<std::string>($2);
    ast->is_array = false;
    $$ = ast;
  }
  | BType IDENT '[' ']' {
    // 一维数组参数: int arr[]
    auto ast = new FuncFParamAST();
    ast->value_type = $1;
    ast->ident = *std::unique_ptr<std::string>($2);
    ast->is_array = true;
    $$ = ast;
  }
  | BType IDENT '[' ']' ConstExpList {
    // 多维数组参数: int arr[][3][4]
    auto ast = new FuncFParamAST();
    ast->value_type = $1;
    ast->ident = *std::unique_ptr<std::string>($2);
    ast->is_array = true;
    for (auto *dim_expr : *$5) {
      ast->dim_exprs.emplace_back(dim_expr);
    }
    delete $5;
    $$ = ast;
  }
  ;

BType
  : INT {
    $$ = ValueType::Int;
  }
  | FLOAT {
    $$ = ValueType::Float;
  }
  ;

// ConstExpList: 数组维度列表, 如 [2][3] 中的 2, 3
ConstExpList
  : '[' Exp ']' {
    auto dims = new std::vector<BaseAST *>();
    dims->push_back($2);
    $$ = dims;
  }
  | ConstExpList '[' Exp ']' {
    $1->push_back($3);
    $$ = $1;
  }
  ;

// ExpList: 数组下标列表, 如 [i][j] 中的 i, j
ExpList
  : '[' Exp ']' {
    auto indices = new std::vector<BaseAST *>();
    indices->push_back($2);
    $$ = indices;
  }
  | ExpList '[' Exp ']' {
    $1->push_back($3);
    $$ = $1;
  }
  ;

// InitValList: 初始化列表元素
InitValList
  : InitVal {
    auto items = new std::vector<BaseAST *>();
    items->push_back($1);
    $$ = items;
  }
  | InitValList ',' InitVal {
    $1->push_back($3);
    $$ = $1;
  }
  ;

// ConstInitValList: 常量初始化列表元素
ConstInitValList
  : ConstInitVal {
    auto items = new std::vector<BaseAST *>();
    items->push_back($1);
    $$ = items;
  }
  | ConstInitValList ',' ConstInitVal {
    $1->push_back($3);
    $$ = $1;
  }
  ;

Block
  : '{' BlockItems '}' {
    auto ast = new BlockAST();
    for (auto *item : *$2) {
      ast->items.emplace_back(item);
    }
    delete $2;
    $$ = ast;
  }
  ;

BlockItems
  : {
    $$ = new std::vector<BaseAST *>();
  }
  | BlockItems BlockItem {
    $1->push_back($2);
    $$ = $1;
  }
  ;

BlockItem
  : Decl {
    $$ = $1;
  }
  | Stmt {
    $$ = $1;
  }
  ;

Decl
  : ConstDecl {
    $$ = $1;
  }
  | VarDecl {
    $$ = $1;
  }
  ;

ConstDecl
  : CONST BType ConstDefList ';' {
    auto ast = new ConstDeclAST();
    for (auto *def : *$3) {
      auto *typed_def = dynamic_cast<ConstDefAST *>(def);
      assert(typed_def != nullptr);
      typed_def->value_type = $2;
      ast->defs.emplace_back(def);
    }
    delete $3;
    $$ = ast;
  }
  ;

ConstDefList
  : ConstDef {
    auto defs = new std::vector<BaseAST *>();
    defs->push_back($1);
    $$ = defs;
  }
  | ConstDefList ',' ConstDef {
    $1->push_back($3);
    $$ = $1;
  }
  ;

// ConstDef ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;
// 支持数组定义: const int a = 1; const int arr[3] = {1,2,3}; const int arr[2][3] = {{1,2,3},{4,5,6}}
ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    ast->init = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ConstExpList '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    for (auto *dim_expr : *$2) {
      ast->dim_exprs.emplace_back(dim_expr);
    }
    delete $2;
    ast->init = std::unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

// ConstInitVal ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
ConstInitVal
  : Exp {
    $$ = $1;
  }
  | '{' '}' {
    // 空初始化列表 {}
    auto ast = new InitValListAST();
    $$ = ast;
  }
  | '{' ConstInitValList '}' {
    auto ast = new InitValListAST();
    for (auto *item : *$2) {
      ast->items.emplace_back(item);
    }
    delete $2;
    $$ = ast;
  }
  ;

VarDecl
  : BType VarDefList ';' {
    auto ast = new VarDeclAST();
    for (auto *def : *$2) {
      auto *typed_def = dynamic_cast<VarDefAST *>(def);
      assert(typed_def != nullptr);
      typed_def->value_type = $1;
      ast->defs.emplace_back(def);
    }
    delete $2;
    $$ = ast;
  }
  ;

VarDefList
  : VarDef {
    auto defs = new std::vector<BaseAST *>();
    defs->push_back($1);
    $$ = defs;
  }
  | VarDefList ',' VarDef {
    $1->push_back($3);
    $$ = $1;
  }
  ;

// VarDef ::= IDENT {"[" ConstExp "]"} | IDENT {"[" ConstExp "]"} "=" InitVal;
// 支持数组定义: int a; int a = 1; int arr[3]; int arr[3] = {1,2,3}; int arr[2][3] = {{1,2,3},{4,5,6}}
VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    ast->init = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ConstExpList {
    // 数组定义 (无初始化): int arr[3], int arr[2][3]
    auto ast = new VarDefAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    for (auto *dim_expr : *$2) {
      ast->dim_exprs.emplace_back(dim_expr);
    }
    delete $2;
    $$ = ast;
  }
  | IDENT ConstExpList '=' InitVal {
    // 数组定义 (有初始化): int arr[3] = {1,2,3}
    auto ast = new VarDefAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    for (auto *dim_expr : *$2) {
      ast->dim_exprs.emplace_back(dim_expr);
    }
    delete $2;
    ast->init = std::unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

// InitVal ::= Exp | "{" [InitVal {"," InitVal}] "}";
InitVal
  : Exp {
    $$ = $1;
  }
  | '{' '}' {
    // 空初始化列表 {}
    auto ast = new InitValListAST();
    $$ = ast;
  }
  | '{' InitValList '}' {
    auto ast = new InitValListAST();
    for (auto *item : *$2) {
      ast->items.emplace_back(item);
    }
    delete $2;
    $$ = ast;
  }
  ;

Stmt
  : MatchedStmt {
    $$ = $1;
  }
  | OpenStmt {
    $$ = $1;
  }
  ;

MatchedStmt
  : RETURN Exp ';' {
    auto ast = new ReturnStmtAST();
    ast->ret_exp = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    // 无返回值 return, 用于 void 函数
    auto ast = new ReturnStmtAST();
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new BreakStmtAST();
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new ContinueStmtAST();
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new AssignStmtAST();
    ast->lval = std::unique_ptr<BaseAST>($1);
    ast->value = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Block {
    // 语句块本身也是一条语句
    $$ = $1;
  }
  | Exp ';' {
    auto ast = new ExprStmtAST();
    ast->expr = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new ExprStmtAST();
    // 空语句: expr 为空
    $$ = ast;
  }
  | IF '(' Exp ')' MatchedStmt ELSE MatchedStmt {
    auto ast = new IfStmtAST();
    ast->cond = std::unique_ptr<BaseAST>($3);
    ast->then_stmt = std::unique_ptr<BaseAST>($5);
    ast->else_stmt = std::unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' MatchedStmt {
    auto ast = new WhileStmtAST();
    ast->cond = std::unique_ptr<BaseAST>($3);
    ast->body = std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt {
    auto ast = new IfStmtAST();
    ast->cond = std::unique_ptr<BaseAST>($3);
    ast->then_stmt = std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' MatchedStmt ELSE OpenStmt {
    auto ast = new IfStmtAST();
    ast->cond = std::unique_ptr<BaseAST>($3);
    ast->then_stmt = std::unique_ptr<BaseAST>($5);
    ast->else_stmt = std::unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' OpenStmt {
    auto ast = new WhileStmtAST();
    ast->cond = std::unique_ptr<BaseAST>($3);
    ast->body = std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

Exp
  // 表达式顶层入口: 逻辑或表达式
  : LOrExp {
    $$ = $1;
  }
  ;

PrimaryExp
  // 基本表达式: 括号表达式或数字
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->inner = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->inner = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number {
    auto number = new NumberAST();
    number->value = $1;
    auto ast = new PrimaryExpAST();
    ast->inner = std::unique_ptr<BaseAST>(number);
    $$ = ast;
  }
  | FLOAT_CONST {
    auto number = new FloatNumberAST();
    number->value = static_cast<float>($1);
    auto ast = new PrimaryExpAST();
    ast->inner = std::unique_ptr<BaseAST>(number);
    $$ = ast;
  }
  ;

// LVal ::= IDENT {"[" Exp "]"};
// 支持数组访问: arr, arr[i], arr[i][j]
LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    $$ = ast;
  }
  | IDENT ExpList {
    auto ast = new LValAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    for (auto *index_expr : *$2) {
      ast->indices.emplace_back(index_expr);
    }
    delete $2;
    $$ = ast;
  }
  ;

UnaryExp
  // 一元表达式: 基本表达式 或 前缀一元运算 或 函数调用
  : PrimaryExp {
    $$ = $1;
  }
  | '+' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->op = '+';
    ast->operand = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '-' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->op = '-';
    ast->operand = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '!' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->op = '!';
    ast->operand = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT '(' ')' {
    // 无参函数调用: ident()
    auto ast = new UnaryExpAST();
    ast->call_ident = *std::unique_ptr<std::string>($1);
    $$ = ast;
  }
  | IDENT '(' FuncRParams ')' {
    // 有参函数调用: ident(args)
    auto ast = new UnaryExpAST();
    ast->call_ident = *std::unique_ptr<std::string>($1);
    for (auto *arg : *$3) {
      ast->call_args.emplace_back(arg);
    }
    delete $3;
    $$ = ast;
  }
  ;

// FuncRParams ::= Exp {"," Exp};
FuncRParams
  : Exp {
    auto args = new std::vector<BaseAST *>();
    args->push_back($1);
    $$ = args;
  }
  | FuncRParams ',' Exp {
    $1->push_back($3);
    $$ = $1;
  }
  ;

MulExp
  // 乘除模: 左结合, 优先级高于加减
  : UnaryExp {
    $$ = $1;
  }
  | MulExp '*' UnaryExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Mul;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Div;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Mod;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  // 加减: 左结合
  : MulExp {
    $$ = $1;
  }
  | AddExp '+' MulExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Add;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Sub;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelExp
  // 关系运算: < > <= >=
  : AddExp {
    $$ = $1;
  }
  | RelExp '<' AddExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Lt;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp '>' AddExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Gt;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp LE AddExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Le;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp GE AddExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Ge;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqExp
  // 相等性运算: == !=
  : RelExp {
    $$ = $1;
  }
  | EqExp EQ RelExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Eq;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | EqExp NE RelExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Ne;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LAndExp
  // 逻辑与: 不做短路求值, 仅构建 AST
  : EqExp {
    $$ = $1;
  }
  | LAndExp AND EqExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::And;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LOrExp
  // 逻辑或: 不做短路求值, 仅构建 AST
  : LAndExp {
    $$ = $1;
  }
  | LOrExp OR LAndExp {
    auto ast = new BinaryExpAST();
    ast->op = BinaryOp::Or;
    ast->lhs = std::unique_ptr<BaseAST>($1);
    ast->rhs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

Number
  // 整数字面量
  : INT_CONST {
    $$ = $1;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}