
# SysY Compiler (Lv1-Lv2)

## 代码结构

- src/
	- main.cpp: 程序入口, 解析命令行参数, 驱动前端与后端流程。
	- ast.h / ast.cpp: AST 定义与 Koopa IR 文本输出。
	- sysy.l: Flex 词法规则。
	- sysy.y: Bison 语法规则与 AST 构建。
	- visit.h / visit.cpp: Koopa raw program 遍历与 RISC-V 汇编生成。
- build/: 编译输出目录 (由 Makefile 生成)。
- Makefile: 构建规则。

## 功能概述

- 支持解析仅包含 `main` 和 `return` 的 SysY 程序。
- `-koopa` 模式输出 Koopa IR。
- `-riscv` 模式输出对应的 RISC-V 汇编。
