%code requires {
  #include "ast.h"
  #include <memory>
  #include <string>
  #include <vector>
}

%{

#include "ast.h"
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
  BaseAST *ast_val;
  std::vector<BaseAST *> *ast_list;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST
%token <str_val> IDENT
%token <int_val> INT_CONST
%token LE GE EQ NE AND OR

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block BlockItem Decl ConstDecl VarDecl ConstDef VarDef
%type <ast_val> Stmt Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> InitVal LVal
%type <ast_list> BlockItems ConstDefList VarDefList
%type <int_val> Number

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = std::make_unique<CompUnitAST>();
    comp_unit->func_def = std::unique_ptr<BaseAST>($1);
    ast = std::move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = std::unique_ptr<BaseAST>($1);
    ast->ident = *std::unique_ptr<std::string>($2);
    ast->block = std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->name = "int";
    $$ = ast;
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
  : CONST INT ConstDefList ';' {
    auto ast = new ConstDeclAST();
    for (auto *def : *$3) {
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

ConstDef
  : IDENT '=' Exp {
    auto ast = new ConstDefAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    ast->init = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

VarDecl
  : INT VarDefList ';' {
    auto ast = new VarDeclAST();
    for (auto *def : *$2) {
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
  ;

InitVal
  : Exp {
    $$ = $1;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new ReturnStmtAST();
    ast->ret_exp = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new AssignStmtAST();
    ast->lval = std::unique_ptr<BaseAST>($1);
    ast->value = std::unique_ptr<BaseAST>($3);
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
  ;

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *std::unique_ptr<std::string>($1);
    $$ = ast;
  }
  ;

UnaryExp
  // 一元表达式: 基本表达式或前缀一元运算
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