/*
 * 测试条款：C99 7.12.6.12  The modf functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 modf / modff / modfl，
 *             验证 [2] 整数部分与小数部分类型、符号与实参一致，
 *             整数部分以浮点格式存入 *iptr，
 *             验证 [3] 返回带符号的小数部分。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），
 *             期望编译器报错。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

/* 辅助：判断两个 double 是否近似相等 */
static int dbl_eq(double a, double b)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d < 1e-9;
}

static int flt_eq(float a, float b)
{
    float d = a - b;
    if (d < 0) d = -d;
    return d < 1e-5f;
}

static int ldbl_eq(long double a, long double b)
{
    long double d = a - b;
    if (d < 0) d = -d;
    return d < 1e-9L;
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* ---------------------------------------------------------------
     * [1] Synopsis：三个函数原型可用，返回类型分别为
     *     double / float / long double，第二参数为对应指针类型。
     *     通过取函数指针并赋值来静态验证原型签名。
     * --------------------------------------------------------------- */
    {
        double (*pf_d)(double, double *)          = modf;
        float  (*pf_f)(float, float *)            = modff;
        long double (*pf_l)(long double, long double *) = modfl;
        assert(pf_d != NULL);
        assert(pf_f != NULL);
        assert(pf_l != NULL);
    }

    /* ---------------------------------------------------------------
     * [2] 正数：整数部分与小数部分类型、符号与实参一致；
     *     整数部分以浮点格式存入 *iptr。
     * --------------------------------------------------------------- */
    {
        double value = 3.75;
        double ipart = 0.0;
        double fpart = modf(value, &ipart);

        /* 整数部分 = 3.0，小数部分 = 0.75 */
        assert(dbl_eq(ipart, 3.0));
        assert(dbl_eq(fpart, 0.75));
        /* 符号一致：均为正 */
        assert(ipart >= 0.0);
        assert(fpart >= 0.0);
        /* 恒等式：value == ipart + fpart */
        assert(dbl_eq(ipart + fpart, value));
    }

    /* ---------------------------------------------------------------
     * [2] 负数：整数部分与小数部分都带负号（与实参符号一致）。
     * --------------------------------------------------------------- */
    {
        double value = -3.75;
        double ipart = 0.0;
        double fpart = modf(value, &ipart);

        /* 整数部分 = -3.0，小数部分 = -0.75 */
        assert(dbl_eq(ipart, -3.0));
        assert(dbl_eq(fpart, -0.75));
        /* 符号一致：均为负 */
        assert(ipart <= 0.0);
        assert(fpart <= 0.0);
        assert(dbl_eq(ipart + fpart, value));
    }

    /* ---------------------------------------------------------------
     * [2] 整数实参：小数部分为 0，整数部分等于实参。
     * --------------------------------------------------------------- */
    {
        double value = 42.0;
        double ipart = 0.0;
        double fpart = modf(value, &ipart);

        assert(dbl_eq(ipart, 42.0));
        assert(dbl_eq(fpart, 0.0));
        assert(dbl_eq(ipart + fpart, value));
    }

    /* ---------------------------------------------------------------
     * [2] 纯小数实参：整数部分为 0，小数部分等于实参。
     * --------------------------------------------------------------- */
    {
        double value = 0.25;
        double ipart = 0.0;
        double fpart = modf(value, &ipart);

        assert(dbl_eq(ipart, 0.0));
        assert(dbl_eq(fpart, 0.25));
        assert(dbl_eq(ipart + fpart, value));
    }

    /* ---------------------------------------------------------------
     * [2] 零：整数部分与小数部分均为 0。
     * --------------------------------------------------------------- */
    {
        double value = 0.0;
        double ipart = 1.0;   /* 故意预置非零，验证被写入 */
        double fpart = modf(value, &ipart);

        assert(dbl_eq(ipart, 0.0));
        assert(dbl_eq(fpart, 0.0));
    }

    /* ---------------------------------------------------------------
     * [2] 负零：符号应保持为负（用 signbit 检查）。
     * --------------------------------------------------------------- */
    {
        double value = -0.0;
        double ipart = 0.0;
        double fpart = modf(value, &ipart);

        assert(dbl_eq(ipart, 0.0));
        assert(dbl_eq(fpart, 0.0));
        /* 负零的符号位应保留 */
        assert(signbit(ipart));
        assert(signbit(fpart));
    }

    /* ---------------------------------------------------------------
     * [2] 大整数：整数部分精确，小数部分为 0。
     * --------------------------------------------------------------- */
    {
        double value = 1e15;
        double ipart = 0.0;
        double fpart = modf(value, &ipart);

        assert(dbl_eq(ipart, 1e15));
        assert(dbl_eq(fpart, 0.0));
    }

    /* ---------------------------------------------------------------
     * [2] 类型一致性：modff 使用 float，modfl 使用 long double。
     * --------------------------------------------------------------- */
    {
        float value = 2.5f;
        float ipart = 0.0f;
        float fpart = modff(value, &ipart);

        assert(flt_eq(ipart, 2.0f));
        assert(flt_eq(fpart, 0.5f));
        assert(flt_eq(ipart + fpart, value));
    }
    {
        float value = -2.5f;
        float ipart = 0.0f;
        float fpart = modff(value, &ipart);

        assert(flt_eq(ipart, -2.0f));
        assert(flt_eq(fpart, -0.5f));
        assert(flt_eq(ipart + fpart, value));
    }
    {
        long double value = 7.125L;
        long double ipart = 0.0L;
        long double fpart = modfl(value, &ipart);

        assert(ldbl_eq(ipart, 7.0L));
        assert(ldbl_eq(fpart, 0.125L));
        assert(ldbl_eq(ipart + fpart, value));
    }
    {
        long double value = -7.125L;
        long double ipart = 0.0L;
        long double fpart = modfl(value, &ipart);

        assert(ldbl_eq(ipart, -7.0L));
        assert(ldbl_eq(fpart, -0.125L));
        assert(ldbl_eq(ipart + fpart, value));
    }

    /* ---------------------------------------------------------------
     * [3] 返回值是带符号的小数部分：直接检查返回值符号。
     * --------------------------------------------------------------- */
    {
        double f1 = modf(5.5, &(double){0});
        double f2 = modf(-5.5, &(double){0});
        assert(f1 > 0.0);
        assert(f2 < 0.0);
        assert(dbl_eq(f1, 0.5));
        assert(dbl_eq(f2, -0.5));
    }

    /* ---------------------------------------------------------------
     * [2][3] 恒等式：对一组值，value == *iptr + 返回值。
     * --------------------------------------------------------------- */
    {
        double vals[] = { 1.5, -1.5, 100.25, -100.25, 0.001, -0.001 };
        size_t i;
        for (i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
            double ip = 0.0;
            double fp = modf(vals[i], &ip);
            assert(dbl_eq(ip + fp, vals[i]));
            /* 小数部分绝对值 < 1 */
            double afp = fp < 0 ? -fp : fp;
            assert(afp < 1.0);
        }
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「modf 第二参数必须为 double *」：
 * 传入 int * 类型不匹配，gcc -std=c99 应报错
 *   warning/error: passing argument 2 of 'modf' from incompatible pointer type
 */
void neg_wrong_ptr_type(void)
{
    int ipart;
    double fpart = modf(3.5, &ipart);   /* 期望编译报错 */
    (void)fpart;
}

/* 违反约束「modf 第二参数必须为 double *」：
 * 传入 float * 类型不匹配，gcc -std=c99 应报错
 */
void neg_float_ptr(void)
{
    float ipart;
    double fpart = modf(3.5, &ipart);   /* 期望编译报错 */
    (void)fpart;
}

/* 违反约束「modff 第二参数必须为 float *」：
 * 传入 double * 类型不匹配，gcc -std=c99 应报错
 */
void neg_modff_wrong_ptr(void)
{
    double ipart;
    float fpart = modff(3.5f, &ipart);  /* 期望编译报错 */
    (void)fpart;
}

/* 违反约束「modfl 第二参数必须为 long double *」：
 * 传入 double * 类型不匹配，gcc -std=c99 应报错
 */
void neg_modfl_wrong_ptr(void)
{
    double ipart;
    long double fpart = modfl(3.5L, &ipart);  /* 期望编译报错 */
    (void)fpart;
}

/* 违反约束「modf 第二参数必须为指针」：
 * 传入非指针（double 值），gcc -std=c99 应报错
 */
void neg_non_pointer(void)
{
    double ipart = 0.0;
    double fpart = modf(3.5, ipart);    /* 期望编译报错 */
    (void)fpart;
}

/* 违反约束「modf 第二参数必须为可修改左值（指向对象的指针）」：
 * 传入 const double * 会丢弃 const 限定，gcc -std=c99 应报错
 */
void neg_const_ptr(void)
{
    const double ipart = 0.0;
    double fpart = modf(3.5, &ipart);   /* 期望编译报错：丢弃 const 限定 */
    (void)fpart;
}

/* 违反约束「modf 第二参数必须为指针」：
 * 传入函数指针类型不匹配，gcc -std=c99 应报错
 */
void neg_func_ptr(void)
{
    double (*fp)(double, double *) = modf;
    double fpart = modf(3.5, fp);       /* 期望编译报错：类型不匹配 */
    (void)fpart;
}

/* 违反约束「modf 第一参数必须为算术类型（double）」：
 * 传入结构体，gcc -std=c99 应报错
 */
struct S { int x; };
void neg_struct_arg(void)
{
    struct S s;
    double ipart;
    double fpart = modf(s, &ipart);     /* 期望编译报错 */
    (void)fpart;
}

/* 违反约束「modf 第一参数必须为算术类型（double）」：
 * 传入指针，gcc -std=c99 应报错
 */
void neg_pointer_arg(void)
{
    int x = 0;
    double ipart;
    double fpart = modf(&x, &ipart);    /* 期望编译报错 */
    (void)fpart;
}

/* 违反约束「modf 返回值类型为 double」：
 * 将返回值赋给结构体，gcc -std=c99 应报错
 */
void neg_return_to_struct(void)
{
    struct S s;
    double ipart;
    s = modf(3.5, &ipart);              /* 期望编译报错：类型不兼容 */
    (void)s;
}

/* 违反约束「modf 第二参数必须为指针」：
 * 传入整数常量，gcc -std=c99 应报错
 */
void neg_int_literal(void)
{
    double fpart = modf(3.5, 0);        /* 期望编译报错：0 不是指针 */
    (void)fpart;
}

#endif /* 负向测试结束 */