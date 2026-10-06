/*
 * 测试 C99 7.20.1.1 —— atof 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：double atof(const char *nptr);  需要 #include <stdlib.h>
 *   [2] 语义：把 nptr 所指字符串的初始部分转换为 double 表示；
 *             除错误行为外，等价于 strtod(nptr, (char **)NULL)。
 *   [3] 返回：返回转换后的值。
 *   Forward references: strtod / strtof / strtold (7.20.1.3)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* 辅助：比较两个 double 是否近似相等 */
static int dbl_eq(double a, double b)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= 1e-9;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：包含 <stdlib.h> 后 atof 声明为
     *     double atof(const char *nptr);
     *     这里通过取函数指针类型来静态验证原型签名。 */
    {
        double (*fp)(const char *) = atof;   /* 若原型不符则编译报错 */
        assert(fp != NULL);
    }

    /* [2] 基本转换：整数形式的字符串 */
    {
        double v = atof("123");
        assert(dbl_eq(v, 123.0));
    }

    /* [2] 小数形式 */
    {
        double v = atof("3.14159");
        assert(dbl_eq(v, 3.14159));
    }

    /* [2] 前导空白被跳过（与 strtod 一致） */
    {
        double v = atof("   \t\n 42.5");
        assert(dbl_eq(v, 42.5));
    }

    /* [2] 正负号 */
    {
        assert(dbl_eq(atof("-7.25"), -7.25));
        assert(dbl_eq(atof("+7.25"),  7.25));
    }

    /* [2] 科学计数法 */
    {
        assert(dbl_eq(atof("1e3"),   1000.0));
        assert(dbl_eq(atof("1.5E-2"), 0.015));
        assert(dbl_eq(atof("2e+2"),  200.0));
    }

    /* [2] “初始部分”转换：遇到不能构成数字的字符即停止 */
    {
        double v = atof("12.5abc");
        assert(dbl_eq(v, 12.5));
    }
    {
        double v = atof("3.14xyz99");
        assert(dbl_eq(v, 3.14));
    }

    /* [2] 十六进制浮点（C99 strtod 支持，atof 等价于 strtod） */
    {
        double v = atof("0x1p4");   /* 1 * 2^4 = 16 */
        assert(dbl_eq(v, 16.0));
    }

    /* [2] 与 strtod(nptr, (char **)NULL) 等价性验证 */
    {
        const char *s = "  -2.71828e1 tail";
        double a = atof(s);
        double b = strtod(s, (char **)NULL);
        assert(dbl_eq(a, b));
        assert(dbl_eq(a, -27.1828));
    }

    /* [2] 无法转换时（无数字）返回 0.0（与 strtod 一致，非错误路径） */
    {
        double v = atof("abc");
        assert(dbl_eq(v, 0.0));
    }
    {
        double v = atof("");
        assert(dbl_eq(v, 0.0));
    }

    /* [3] 返回值类型为 double，可直接参与算术 */
    {
        double sum = atof("1.5") + atof("2.5");
        assert(dbl_eq(sum, 4.0));
    }

    /* [3] 返回值可用于初始化/赋值 double 对象 */
    {
        double d = atof("99.9");
        assert(dbl_eq(d, 99.9));
    }

    /* [2] 参数为 const char *：传入字符串字面量（const 限定）应可编译 */
    {
        const char *p = "6.02e23";
        double v = atof(p);
        assert(dbl_eq(v, 6.02e23));
    }

    printf("All positive tests for C99 7.20.1.1 (atof) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「atof 的参数类型为 const char *」：
     * 传入 double 实参，gcc -std=c99 应报 incompatible type 错误。 */
    {
        double x = 1.0;
        double v = atof(x);          /* 期望：编译报错 */
        (void)v;
    }

    /* 违反约束「atof 的参数类型为 const char *」：
     * 传入 int 实参，应报错。 */
    {
        int n = 5;
        double v = atof(n);          /* 期望：编译报错 */
        (void)v;
    }

    /* 违反约束「atof 的参数类型为 const char *」：
     * 传入结构体指针，应报错。 */
    {
        struct S { int x; } s;
        double v = atof(&s);         /* 期望：编译报错 */
        (void)v;
    }

    /* 违反约束「atof 返回 double」：
     * 把返回值赋给结构体类型，应报错。 */
    {
        struct S { int x; } s;
        s = atof("1.0");             /* 期望：编译报错 */
    }

    /* 违反约束「atof 返回 double」：
     * 对函数调用结果取成员（非结构体），应报错。 */
    {
        int y = atof("1.0").x;       /* 期望：编译报错 */
        (void)y;
    }

    /* 违反约束「atof 需要恰好一个参数」：
     * 无参数调用，应报错。 */
    {
        double v = atof();           /* 期望：编译报错 */
        (void)v;
    }

    /* 违反约束「atof 需要恰好一个参数」：
     * 两个参数调用，应报错。 */
    {
        double v = atof("1.0", "2.0"); /* 期望：编译报错 */
        (void)v;
    }

    /* 违反约束「atof 返回 double，不是左值」：
     * 对函数调用结果赋值，应报错。 */
    {
        atof("1.0") = 2.0;           /* 期望：编译报错 */
    }

    /* 违反约束「atof 返回 double，不是左值」：
     * 对函数调用结果取地址，应报错。 */
    {
        double *p = &atof("1.0");    /* 期望：编译报错 */
        (void)p;
    }

#endif /* 负向测试结束 */

    return 0;
}