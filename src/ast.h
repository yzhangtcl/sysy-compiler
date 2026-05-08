#pragma once
#include <iostream>
#include <vector>
#include <string>

/* =========================
 * indent helper
 * ========================= */
static void indent(int n) {
    for (int i = 0; i < n; i++) std::cout << "    ";
}

/* =========================
 * base AST
 * ========================= */
struct BaseAST {
    virtual ~BaseAST() = default;
    virtual void Dump(int indent_level = 0) const = 0;
};

/* =========================
 * CompUnit (ROOT)
 * ========================= */
struct CompUnitAST : BaseAST {
    BaseAST* func;

    void Dump(int indent_level = 0) const override {
        indent(indent_level);
        std::cout << "CompUnit {\n";

        indent(indent_level + 1);
        std::cout << "func_def: ";
        func->Dump(indent_level + 1);
        std::cout << ",\n";

        indent(indent_level);
        std::cout << "}";
    }
};

/* =========================
 * FuncDef
 * ========================= */
struct FuncDefAST : BaseAST {
    std::string name;
    BaseAST* block;

    void Dump(int indent_level = 0) const override {
        std::cout << "FuncDef {\n";

        indent(indent_level + 1);
        std::cout << "func_type: Int,\n";

        indent(indent_level + 1);
        std::cout << "ident: \"" << name << "\",\n";

        indent(indent_level + 1);
        std::cout << "block: ";
        block->Dump(indent_level + 1);
        std::cout << ",\n";

        indent(indent_level);
        std::cout << "}";
    }
};

/* =========================
 * Block
 * ========================= */
struct BlockAST : BaseAST {
    std::vector<BaseAST*> stmts;

    void Dump(int indent_level = 0) const override {
        std::cout << "Block {\n";

        for (auto s : stmts) {
            indent(indent_level + 1);
            std::cout << "stmt: ";
            s->Dump(indent_level + 1);
            std::cout << ",\n";
        }

        indent(indent_level);
        std::cout << "}";
    }
};

/* =========================
 * Stmt
 * ========================= */
struct StmtAST : BaseAST {};

/* =========================
 * Return
 * ========================= */
struct ReturnAST : StmtAST {
    BaseAST* expr;

    void Dump(int indent_level = 0) const override {
        std::cout << "Stmt {\n";

        indent(indent_level + 1);
        std::cout << "num: ";
        expr->Dump(indent_level + 1);
        std::cout << ",\n";

        indent(indent_level);
        std::cout << "}";
    }
};

/* =========================
 * Expr
 * ========================= */
struct ExprAST : BaseAST {};

/* number */
struct NumberAST : ExprAST {
    int val;

    NumberAST(int v) : val(v) {}

    void Dump(int indent_level = 0) const override {
        std::cout << val;
    }
};

/* binary */
struct BinaryAST : ExprAST {
    std::string op;
    BaseAST* l;
    BaseAST* r;

    BinaryAST(std::string o, BaseAST* a, BaseAST* b)
        : op(o), l(a), r(b) {}

    void Dump(int indent_level = 0) const override {
        std::cout << "(";
        l->Dump(indent_level);
        std::cout << " " << op << " ";
        r->Dump(indent_level);
        std::cout << ")";
    }
};

/* unary */
struct UnaryAST : ExprAST {
    std::string op;
    BaseAST* expr;

    UnaryAST(std::string o, BaseAST* e)
        : op(o), expr(e) {}

    void Dump(int indent_level = 0) const override {
        std::cout << "(" << op;
        expr->Dump(indent_level);
        std::cout << ")";
    }
};