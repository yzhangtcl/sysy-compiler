
# SysY Compiler 

## Lv1-Lv2

### 项目结构

给出了代码的主要框架：

- src/
	- main.cpp: 程序入口, 解析命令行参数, 驱动前端与后端流程。
	- ast.h / ast.cpp: AST 定义与 Koopa IR 文本输出。
	- sysy.l: Flex 词法规则。
	- sysy.y: Bison 语法规则与 AST 构建。
	- visit.h / visit.cpp: Koopa raw program 遍历与 RISC-V 汇编生成。
- build/: 编译输出目录 (由 Makefile 生成)。
- Makefile: 构建规则。

### 项目流程

最终目的是将输入的 SysY 源代码, 编译到 RISC-V 汇编，项目的整体流程大致分成了三步：
1. 前端: 通过词法分析和语法分析, 将源代码解析成抽象语法树 (abstract syntax tree, AST). 通过语义分析, 扫描抽象语法树, 检查其是否存在语义错误.
2. 中端: 将抽象语法树转换为中间表示 (intermediate representation, IR), 并在此基础上完成一些机器无关优化.
3. 后端: 将中间表示转换为目标平台的汇编代码, 并在此基础上完成一些机器相关优化.


## lv3-expr

为支持表达式计算, 本阶段做了如下改动:

- 前端语法扩展: 增加一元、算术、比较与逻辑表达式的产生式, 支持括号表达式。
- 词法扩展: 补充 <= >= == != && || 等多字符运算符的识别。
- AST 扩展: 引入表达式节点, 支持一元/二元表达式并生成 Koopa IR。
- IR 生成: 统一从表达式生成值, 逻辑与或转换为 Koopa 二元指令组合(不做短路)。
- 汇编生成: 二元指令落栈存值, 用固定寄存器完成计算并回写, 避免临时寄存器耗尽。

## lv4-const-n-var

为支持常量、变量与赋值语句, 本阶段做了如下改动:

- 前端语法扩展: 增加 const/变量声明、赋值语句与块内多条语句/声明的产生式。
- 词法扩展: 增加 const 关键字识别。
- AST 扩展: 引入常量/变量定义、赋值语句与左值节点, 并增加常量表达式求值接口。
- 语义分析: 引入作用域符号表, 区分常量与变量, 支持常量折叠和赋值合法性检查。
- IR 生成: 变量使用 alloc/load/store 落栈, 常量在编译期求值后直接替换。
- 汇编生成: 处理 alloc/load/store, 统一为所有有返回值的指令分配栈槽。

## lv4_w_float float-support

为支持浮点变量与表达式, 本阶段做了如下改动:

- 前端语法扩展: 增加 float 基本类型与浮点常量, 常量/变量声明按类型落位。
- AST 扩展: 引入浮点字面量与类型标记, 常量表达式支持浮点求值与类型转换。
- IR 生成: 浮点值以 32-bit bit pattern 形式承载, 运算保留类型信息。
- 汇编生成: 使用 RISC-V F 扩展指令完成浮点运算/比较/类型转换。

同时增加了自定义测试 `./test.sh`

添加环境：

```bash
apt update
apt install -y gcc-riscv64-linux-gnu qemu-user
```

测试程序在 `./test/` 中，文件名为 `test1.c`，则测试命令为：

```bash
./test.sh test1
```

注意，当前测试方法只支持通过 return 的值来判断结果是否正确，具体请参考 `./test/test1` (~~内藏小彩蛋~~)

## lv5-block-n-scope

为支持语句块与作用域, 本阶段做了如下改动:

- 前端语法扩展: 增加语句块作为 Stmt 的产生式, 支持空语句与表达式语句。
- AST 扩展: 新增表达式语句节点, 确保仅有副作用的表达式被正确生成 IR。
- 语义分析: 作用域随 Block 进入/退出, 同名标识符在内层作用域覆盖外层定义。

## lv6-if

为支持 if/else 与短路求值, 本阶段做了如下改动:

- 前端语法扩展: 增加 if/else 语句并通过 matched/open 拆分解决悬挂 else。
- AST 与 IR 生成: 新增 if 语句节点, 生成 br/jump 与基本块标签; 逻辑与/或改为短路求值。
- 汇编生成: 新增 branch/jump 指令翻译, 输出基本块标签并使用 bnez/j 完成控制流跳转。

## lv7-while

为支持 while/break/continue, 本阶段做了如下改动:

- 前端语法扩展: 增加 while、break、continue 语句, 同样按 matched/open 处理 while 体的悬挂 else。
- AST 扩展: 新增 while/break/continue 节点, 统一纳入语句生成流程。
- IR 生成: 生成 while 的条件块/循环体/结束块, 维护循环栈以定位 break/continue 的跳转目标。
## lv8-func-n-global

为支持函数定义与调用、SysY 库函数和全局变量, 本阶段做了如下改动:

### 前端词法扩展
- 新增 `void` 关键字, 支持无返回值函数类型.

### 前端语法扩展
- `CompUnit` 重构为支持多个顶层的 `Decl` 和 `FuncDef`, 允许全局变量/常量与函数共存.
- `FuncDef` 增加形式参数列表 `FuncFParams`, 支持 `int`/`float`/`void` 返回类型.
- `UnaryExp` 新增函数调用产生式 `IDENT '(' [FuncRParams] ')'`, 支持有参/无参调用.
- `return` 语句支持无表达式形式 (`return ;`), 用于 void 函数.

### AST 扩展
- 新增 `FuncFParamAST` 表示函数形式参数 (类型 + 标识符).
- `FuncDefAST` 扩展为包含参数列表和 void 返回类型.
- `UnaryExpAST` 扩展为同时承载一元运算和函数调用 (通过 `call_ident`/`call_args`).
- `CompUnitAST` 改为持有 `vector<BaseAST>` 以容纳多个顶层声明/定义.
- `ReturnStmtAST` 改为可选返回值 (用于 void 函数).

### 语义分析与 IR 生成
- 建立全局作用域: `CompUnit` 输出时先进入全局作用域, 注册 8 个 SysY 库函数符号.
- 库函数声明: 自动在 IR 头部输出所有库函数的 `decl` 语句 (getint/getch/getarray/putint/putch/putarray/starttime/stoptime).
- 函数定义: 生成 `fun @name(@param: i32, ...): ret_type { ... }`, 入口处为每个参数分配局部内存并 store 参数值.
- 函数调用: 有返回值时生成 `%v = call @name(args)`, void 函数生成 `call @name(args)`.
- 全局变量: 使用 `global @var = alloc i32, zeroinit` (带初值则写入具体值), 读写直接使用 `@var` 符号.
- 全局常量: 同局部常量在编译期求值, 仅存入全局符号表.

### 汇编生成 (RISC-V)
- **函数定义与标签**: 为每个非声明函数生成 `.globl` 标签和 `.text` 段声明 (仅首次).
- **栈帧管理**: 预处理阶段扫描函数内所有指令, 统计局部变量空间、是否有 `call` 指令 (决定 `ra` 保存)、最大调用参数个数 (决定栈传参预留空间). 栈空间对齐到 16 字节.
- **Prologue**: `addi sp, sp, -frame_size`; 非叶子函数保存 `ra` 到栈帧顶部.
- **Epilogue**: 恢复 `ra` (如需要) → 恢复 `sp` → `ret`.
- **函数调用 (call)**: 前 8 个参数依次放入 `a0`-`a7`, 超出部分存入 `sp + (i-8)*4`; 执行 `call` 指令后, 返回值从 `a0` (整型) 写入栈槽.
- **全局变量**: 输出 `.data` 段, 使用 `.globl`/`.zero`/`.word` 定义全局符号; 在函数内通过 `la` + `lw`/`sw` 访问.
- **库函数调用**: 与普通函数调用一致, 链接器负责解析外部符号.

## lv8_w_floatIO floatIO-support

添加了支持 `float` 输入输出的 SysY 库函数 `float getfloat()`  和 `void putfloat(float)`

同时增加了测试 `./test_2.sh` 用来进行测试，使用方法如下：

1. 编译 `sysy-runtime-lib`：

   ```bash
   cd ./sysy-runtime-lib
   make CC="riscv64-linux-gnu-gcc"
   ```

2. 运行 `test_2.sh`

   ```cpp
   ./test_2.sh test2
   ```

## lv9-array 数组支持

为支持数组定义、访问与函数参数传递, 本阶段做了如下改动:

### 前端语法扩展
- `ConstDef` 和 `VarDef` 增加可选的数组维度定义 `{"[" ConstExp "]"}`, 支持多维数组声明.
- `ConstInitVal` 和 `InitVal` 扩展为支持嵌套花括号初始化列表 `{InitVal, InitVal, ...}`.
- `LVal` 增加可选的数组下标 `{"[" Exp "]"]}`, 支持 `arr[i]`、`arr[i][j]` 等多维访问.
- `FuncFParam` 扩展为支持数组参数 `BType IDENT "[" "]" {"[" ConstExp "]"}`, 如 `int arr[]`、`int arr[][3]`.
- 新增 `ConstExpList`、`ExpList`、`InitValList`、`ConstInitValList` 辅助产生式.

### AST 扩展
- 新增 `InitValListAST` 表示初始化列表, 可嵌套.
- `ConstDefAST` 和 `VarDefAST` 增加 `dim_exprs` 字段存储维度表达式, 延迟到 IR 生成时求值 (避免符号表未建立时求值失败).
- `LValAST` 增加 `indices` 字段存储数组下标表达式列表.
- `FuncFParamAST` 增加 `is_array` 标记和 `dim_exprs` 存储数组参数维度.
- `ValueResult` 增加 `is_array_ptr` 标记和 `remaining_dims` 字段, 用于函数调用时的数组指针衰减.

### 语义分析与 IR 生成
- 数组类型: 使用 Koopa IR 数组类型 `[i32, N]`、`[[i32, M], N]` 等.
- 数组分配: 局部数组使用 `alloc [T, N]`, 全局数组使用 `global alloc [T, N], {agg}` 或 `zeroinit`.
- 数组访问: 使用 `getelemptr` 指令进行多级索引, 通过 `EmitArrayElemPtr` 辅助函数生成多级指针计算链.
- 初始化列表展平: `FlattenArrayInit` 递归处理嵌套初始化列表, 按 C 语言规则对齐维度边界并补零.
- 全局数组初始化: 使用 Koopa IR 聚合常量 `{v1, v2, ...}` 表示.
- 局部数组初始化: 逐元素使用 `getelemptr` + `store` 初始化.
- 函数数组参数: 使用指针类型 `*i32`、`*[i32, M]` 等, 第一维省略.
- 数组参数访问: 使用 `getptr` 处理第一维, `getelemptr` 处理后续维度.
- 数组指针衰减: 当子数组指针 (如 `*[i32, M]`) 作为函数实参传递给期望基类型指针 (`*i32`) 的形参时, 自动插入 `getelemptr ptr, 0` 完成类型转换.
- 函数参数名唯一化: 使用 `@p0`、`@p1` 等格式避免与全局变量名冲突.
- 函数参数类型注册: `RegisterFunctionParamTypes` 记录每个函数的参数类型信息, 用于调用时的类型匹配与衰减决策.

### 汇编生成 (RISC-V)
- `getelemptr` 指令: 计算 `base + index * sizeof(element)`, 生成 `addi`/`mul`/`add` 指令序列.
- `getptr` 指令: 与 `getelemptr` 类似, 计算指针偏移.
- 全局聚合初始化: 递归输出 `.word` 和 `.zero` 指令.
- 类型大小计算: `CalcTypeSize` 递归计算数组/指针/整数类型的字节大小.
- 栈帧分配: `alloc` 指令按实际类型大小分配栈空间 (非固定 4 字节).


## lv9_w_float 浮点数组的支持

为修复浮点数组的正确处理, 本阶段做了如下改动:

### 后端汇编生成修复 (visit.cpp)
- **浮点 bit pattern 正确传递**: `LoadFloatValue` 中将 `fcvt.s.w` 改为 `fmv.w.x`, 确保 INTEGER 常量作为 IEEE 754 bit pattern 正确移入浮点寄存器 (而非被当作整数值转换).
- **`VisitStore` 指针/浮点区分**: 增加对目标 alloc 是否持有指针类型的检查 — 数组参数的 alloc 持有 `*i32` 等指针类型, 应使用 `sw` 而非 `fsw`; 同时增加 `FUNC_ARG_REF` 分支, 标量浮点参数用 `fmv.w.x` 而非 `fcvt.s.w`.
- **`VisitCall` 返回值修复**: 所有返回类型为 float 的函数 (不仅 `getfloat`) 都从 `fa0` 读取返回值, 而非仅限库函数.
- **函数类型表恢复**: `VisitFunction` 在处理每个函数前调用 `RestoreValueTypeTable`, 解决多函数编译时前端生成的类型信息被后续函数覆盖的问题.

### 前端 IR 生成修复 (ast.cpp)
- **浮点数组初始化**: `VarDefAST::DumpKoopa` 和 `ConstDefAST::DumpKoopa` 中, 浮点数组逐元素初始化时将展平后的 bit pattern 包装为正确的 float IR 值 (`add 0, <bits>`), 避免后端误将裸整数做 `fcvt.s.w` 转换.
- **函数类型表保存/恢复**: 新增 `SaveValueTypeTable` / `RestoreValueTypeTable` 机制, 每个函数生成完毕后保存其类型表, 后端按函数名恢复, 彻底解决类型信息在多函数场景下丢失的根本问题.

### 测试脚本迁移至 riscv32 (test.sh / test_2.sh)
- 汇编编译使用 `clang -target riscv32-unknown-linux-elf -march=rv32imf -mabi=ilp32f` (hard-float ABI).
- 链接使用 `ld.lld -static`.
- 运行使用 `qemu-riscv32`.

### 运行时库编译 (sysy-runtime-lib)

以 riscv32 + hard-float ABI 编译 `libsysy.a`:

```bash
cd ./sysy-runtime-lib
make clean
make NO_LIBC=1 ADD_CFLAGS="-target riscv32-unknown-linux-elf -march=rv32imf -mabi=ilp32f"
```
