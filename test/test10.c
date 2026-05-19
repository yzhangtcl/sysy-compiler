/* 简单线性回归 + 预测
   输入：点的个数 n，然后 n 对 (x, y)
   输出：拟合直线 y = a*x + b 的系数 a 和 b，
         然后读取一个 x0，输出预测值 y0
   只用到 int, float, 数组, 函数, while, if */

/* 库函数原型 (假设编译器已提供，若未提供可加上声明) */

const int MAX=100;   /* 最多处理的点数 */

/* 计算数组的平均值 */
float mean(float arr[], int n) {
    float sum;
    int i;
    sum = 0.0;
    i = 0;
    while (i < n) {
        sum = sum + arr[i];
        i = i + 1;
    }
    return sum / n;
}

/* 计算数组的方差 (已知均值) */
float variance(float arr[], int n, float m) {
    float sum;
    float diff;
    int i;
    sum = 0.0;
    i = 0;
    while (i < n) {
        diff = arr[i] - m;
        sum = sum + diff * diff;
        i = i + 1;
    }
    return sum / n;
}

/* 计算两个数组的协方差 (已知各自的均值) */
float covariance(float x[], float y[], int n, float mx, float my) {
    float sum;
    int i;
    sum = 0.0;
    i = 0;
    while (i < n) {
        sum = sum + (x[i] - mx) * (y[i] - my);
        i = i + 1;
    }
    return sum / n;
}

int main() {
    int n;
    float x[MAX], y[MAX];
    float mx, my, var, cov;
    float a, b;
    float x0, y0;
    int i;

    /* 1. 读取点的个数 */
    n = getint();

    /* 2. 检查边界，若非法则输出错误码 -1 并结束 */
    if (n <= 0) {
        putint(-1);
        return -1;
    }
    if (n > MAX) {
        putint(-2);
        return -2;
    }

    /* 3. 循环读取 n 对浮点数 */
    i = 0;
    while (i < n) {
        x[i] = getfloat();
        y[i] = getfloat();
        i = i + 1;
    }

    /* 4. 计算均值 */
    mx = mean(x, n);
    my = mean(y, n);

    /* 5. 计算方差和协方差 */
    var = variance(x, n, mx);
    cov = covariance(x, y, n, mx, my);

    /* 6. 如果方差为 0，无法拟合 (所有 x 相同) */
    if (var == 0.0) {
        putint(-3);
        return -3;
    }

    /* 7. 计算回归系数 */
    a = cov / var;
    b = my - a * mx;

    /* 8. 输出系数 */
    putfloat(a);
    putfloat(b);

    /* 9. 读取一个新的 x0 并输出预测值 y0 */
    x0 = getfloat();
    y0 = a * x0 + b;
    putfloat(y0);

    return 0;
}