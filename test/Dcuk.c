/* Dcuk.c — SysY编译器全特性综合测试
   覆盖: const / 全局变量 / 所有运算符 / 控制流 / 递归 / 多维数组
         / 类型转换 / 作用域 / 名字遮蔽 / 短路求值 / 数组参数指针退化
         / 浮点运算 / void函数 / 多参数(>8) / 所有10个库函数 */

/* ===== 常量 ===== */
const int N = 5;
const int M = 3;
const float PI = 3.14;
const float E = 2.72;
const int FOLD = N + M * 2;       /* 常量折叠: 5 + 6 = 11 */
const int ARR_DIM = 2 + 2;        /* 常量折叠: 4 */

/* ===== 全局变量 ===== */
int g_cnt;
float g_scale;
int g_flag;                       /* 短路求值辅助 */

/* ===== 辅助函数 ===== */

/* 递归 — 测试函数递归调用 */
int factorial(int n) {
    if (n <= 1)
        return 1;
    return n * factorial(n - 1);
}

/* float返回值函数 */
float circle_area(float r) {
    return PI * r * r;
}

/* void函数 — 测试break/continue */
void test_loop() {
    int i;
    i = 0;
    while (i < 10) {
        i = i + 1;
        if (i == 3)
            continue;
        if (i == 7)
            break;
        putint(i);
        putch(32);
    }
    putch(10);
}

/* 多参数函数 — 测试>8个参数(栈传参) */
int many_args(int a0, int a1, int a2, int a3, int a4,
              int a5, int a6, int a7, int a8, int a9) {
    return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
}

/* 1D数组参数 */
float sum_1d(float arr[], int n) {
    float s;
    int i;
    s = 0.0;
    i = 0;
    while (i < n) {
        s = s + arr[i];
        i = i + 1;
    }
    return s;
}

/* 2D数组参数 — 修改数组元素 */
void double_2d(int mat[][3], int rows) {
    int i, j;
    i = 0;
    while (i < rows) {
        j = 0;
        while (j < 3) {
            mat[i][j] = mat[i][j] * 2;
            j = j + 1;
        }
        i = i + 1;
    }
}

/* 短路求值辅助 — 通过全局变量副作用验证 */
int side_effect_true() {
    g_flag = 1;
    return 1;
}

int side_effect_false() {
    g_flag = 2;
    return 0;
}

/* 作用域 + 名字遮蔽 */
int scope_test() {
    int x;
    x = 10;
    {
        int x;
        x = 20;
        putint(x);
        putch(32);
    }
    putint(x);
    putch(32);
    return x;
}

/* 填充float数组 */
void fill_float_arr(float arr[], int n, float start, float step) {
    int i;
    float val;
    i = 0;
    val = start;
    while (i < n) {
        arr[i] = val;
        val = val + step;
        i = i + 1;
    }
}

/* void无返回值函数 */
void print_ok() {
    putint(1);
}

/* ===== 主函数 ===== */
int main() {
    int i, j, k;
    float fx, fy, fz;
    float arr_f[M];
    int result;
    float fresult;
    int arr_int[6];

    /* ---- T1: 常量 ---- */
    putint(1); putch(58); putch(32);
    putint(N);
    putch(32);
    putfloat(PI);
    putch(32);
    putint(FOLD);
    putch(10);

    /* ---- T2: 全局变量 ---- */
    g_cnt = 42;
    g_scale = 1.5;
    putint(2); putch(58); putch(32);
    putint(g_cnt);
    putch(32);
    putfloat(g_scale);
    putch(10);

    /* ---- T3: 整数算术 +-*、% ---- */
    putint(3); putch(58); putch(32);
    putint(2 + 3 * 4);
    putch(32);
    putint(10 - 3);
    putch(32);
    putint(15 / 4);
    putch(32);
    putint(15 % 4);
    putch(10);

    /* ---- T4: 浮点算术 + 混合类型 ---- */
    putint(4); putch(58); putch(32);
    fx = 3.5;
    fy = 2.0;
    putfloat(fx + fy);
    putch(32);
    putfloat(fx - fy);
    putch(32);
    putfloat(fx * fy);
    putch(32);
    putfloat(fx / fy);
    putch(32);
    putfloat(fx + 1);
    putch(10);

    /* ---- T5: 比较运算符 (int + float) ---- */
    putint(5); putch(58); putch(32);
    putint(5 > 3);
    putch(32);
    putint(5 < 3);
    putch(32);
    putint(5 == 5);
    putch(32);
    putint(5 != 3);
    putch(32);
    putint(5 >= 5);
    putch(32);
    putint(5 <= 3);
    putch(32);
    fx = 3.14;
    fy = 2.72;
    putint(fx > fy);
    putch(32);
    putint(fx == fy);
    putch(10);

    /* ---- T6: 逻辑运算符 && || ! ---- */
    putint(6); putch(58); putch(32);
    putint(1 && 1);
    putch(32);
    putint(1 && 0);
    putch(32);
    putint(0 || 1);
    putch(32);
    putint(0 || 0);
    putch(32);
    putint(!1);
    putch(32);
    putint(!0);
    putch(10);

    /* ---- T7: 短路求值 (函数+全局副作用) ---- */
    putint(7); putch(58); putch(32);
    g_flag = 0;
    /* 1 || side_effect_false() — 短路: g_flag不变 */
    if (1 || side_effect_false())
        k = g_flag;
    putint(g_flag);
    putch(32);
    /* 0 && side_effect_true() — 短路: g_flag不变 */
    if (0 && side_effect_true())
        k = g_flag;
    putint(g_flag);
    putch(10);

    /* ---- T8: if/else + 悬空else ---- */
    putint(8); putch(58); putch(32);
    i = 10;
    if (i > 5)
        if (i > 8)
            putint(1);
        else
            putint(0);
    putch(32);
    if (i < 5)
        putint(-1);
    else
        putint(2);
    putch(10);

    /* ---- T9: while + break + continue ---- */
    putint(9); putch(58); putch(32);
    test_loop();

    /* ---- T10: 2D int数组访问 ---- */
    putint(10); putch(58); putch(32);
    {
        int m2d[2][3];
        i = 0;
        while (i < 2) {
            j = 0;
            while (j < 3) {
                m2d[i][j] = i * 3 + j + 1;
                j = j + 1;
            }
            i = i + 1;
        }
        putint(m2d[0][0]);
        putch(32);
        putint(m2d[0][2]);
        putch(32);
        putint(m2d[1][0]);
        putch(32);
        putint(m2d[1][2]);
        putch(10);
    }

    /* ---- T11: 2D float数组访问 ---- */
    putint(11); putch(58); putch(32);
    {
        float fm[2][3];
        i = 0;
        while (i < 2) {
            j = 0;
            while (j < 3) {
                fm[i][j] = (i * 3.0 + j) * 0.5;
                j = j + 1;
            }
            i = i + 1;
        }
        putfloat(fm[0][0]);
        putch(32);
        putfloat(fm[0][2]);
        putch(32);
        putfloat(fm[1][1]);
        putch(32);
        putfloat(fm[1][2]);
        putch(10);
    }

    /* ---- T12: 作用域 + 名字遮蔽 ---- */
    putint(12); putch(58); putch(32);
    k = scope_test();
    putint(k);
    putch(10);

    /* ---- T13: 递归函数 ---- */
    putint(13); putch(58); putch(32);
    putint(factorial(5));
    putch(10);

    /* ---- T14: float返回值函数 ---- */
    putint(14); putch(58); putch(32);
    fx = circle_area(3.0);
    putfloat(fx);
    putch(10);

    /* ---- T15: 多参数函数(>8) ---- */
    putint(15); putch(58); putch(32);
    result = many_args(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    putint(result);
    putch(10);

    /* ---- T16: 1D数组参数 ---- */
    putint(16); putch(58); putch(32);
    fill_float_arr(arr_f, M, 1.0, 0.5);
    fresult = sum_1d(arr_f, M);
    putfloat(fresult);
    putch(10);

    /* ---- T17: 2D数组参数 + 修改 ---- */
    putint(17); putch(58); putch(32);
    {
        int imat[2][3];
        i = 0;
        while (i < 2) {
            j = 0;
            while (j < 3) {
                imat[i][j] = i * 3 + j + 1;
                j = j + 1;
            }
            i = i + 1;
        }
        double_2d(imat, 2);
        putint(imat[0][0]);
        putch(32);
        putint(imat[0][1]);
        putch(32);
        putint(imat[1][2]);
        putch(10);
    }

    /* ---- T18: 隐式类型转换 int↔float ---- */
    putint(18); putch(58); putch(32);
    i = 5;
    fx = i;
    putfloat(fx);
    putch(32);
    fy = 3.7;
    j = fy;
    putint(j);
    putch(10);

    /* ---- T19: 一元运算符 + - ! ---- */
    putint(19); putch(58); putch(32);
    putint(-5);
    putch(32);
    putint(!0);
    putch(32);
    putint(!5);
    putch(32);
    fx = -3.14;
    putfloat(fx);
    putch(10);

    /* ---- T20: void函数 ---- */
    putint(20); putch(58); putch(32);
    print_ok();
    putch(10);

    /* ---- T21: I/O: getint/getfloat/getch ---- */
    putint(21); putch(58); putch(32);
    i = getint();
    putint(i);
    putch(32);
    fx = getfloat();
    putfloat(fx);
    putch(32);
    j = getch();
    putch(j);
    putch(10);

    /* ---- T22: getarray/putarray ---- */
    putint(22); putch(58); putch(32);
    k = getarray(arr_int);
    putarray(k, arr_int);

    /* ---- T23: 取模边界情况 ---- */
    putint(23); putch(58); putch(32);
    putint(10 % 3);
    putch(32);
    putint(-10 % 3);
    putch(32);
    putint(-10 % -3);
    putch(10);

    /* ---- T24: 数组参数指针退化 (2D行→1D参数) ---- */
    putint(24); putch(58); putch(32);
    {
        float mat2d[2][3];
        i = 0;
        while (i < 2) {
            j = 0;
            while (j < 3) {
                mat2d[i][j] = (i + 1.0) * (j + 1.0);
                j = j + 1;
            }
            i = i + 1;
        }
        fresult = sum_1d(mat2d[0], 3);
        putfloat(fresult);
        putch(32);
        fresult = sum_1d(mat2d[1], 3);
        putfloat(fresult);
        putch(10);
    }

    /* ---- T25: 深度嵌套控制流 ---- */
    putint(25); putch(58); putch(32);
    i = 0;
    j = 0;
    while (i < 5) {
        if (i % 2 == 0) {
            k = 0;
            while (k < i) {
                j = j + 1;
                k = k + 1;
            }
        }
        i = i + 1;
    }
    putint(j);
    putch(10);

    return 0;
}
