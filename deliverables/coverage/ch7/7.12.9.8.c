/*
 * 测试 C99 7.12.9.8 —— trunc / truncf / truncl 函数
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 trunc/truncf/truncl，验证它们把参数
 *             向零方向截断为最接近且绝对值不大于参数的整数值（浮点格式），
 *             并返回该截断后的整数值。程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数个数错误、对非算术类型调用等）
 *             应导致编译报错；这些片段放在 #if 0 中，保证本文件仍可编译。
 *
 * 覆盖段落：
 *   [1] 函数声明（Synopsis）：double trunc(double);
 *                            float  truncf(float);
 *                            long double truncl(long double);
 *   [2] 描述（Description）：向零截断，结果绝对值不大于参数。
 *   [3] 返回值（Returns）：返回截断后的整数值。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证三个函数在 <math.h> 中声明，且可被调用。
 *     通过取函数指针来静态确认原型类型。 */
static double (*p_trunc)(double)   = trunc;
static float  (*p_truncf)(float)   = truncf;
static long double (*p_truncl)(long double) = truncl;

/* 辅助：判断浮点值是否为整数值（无小数部分） */
static int is_integral_value(double v)
{
    return v == floor(v);
}

int main(void)
{
    /* ---------- [1] 原型/类型检查 ---------- */
    assert(p_trunc  != NULL);
    assert(p_truncf != NULL);
    assert(p_truncl != NULL);

    /* ---------- [2][3] 正数：向零截断 ---------- */
    assert(trunc(3.7)  == 3.0);
    assert(trunc(3.2)  == 3.0);
    assert(trunc(0.9)  == 0.0);
    assert(trunc(1.0)  == 1.0);   /* 已是整数，不变 */

    /* ---------- [2][3] 负数：向零截断（绝对值不大于参数） ---------- */
    assert(trunc(-3.7) == -3.0);
    assert(trunc(-3.2) == -3.0);
    assert(trunc(-0.9) == -0.0);  /* 结果为 -0.0，与 0.0 相等 */
    assert(trunc(-1.0) == -1.0);

    /* ---------- [2] 结果绝对值不大于参数绝对值 ---------- */
    {
        double xs[] = { 3.7, -3.7, 0.5, -0.5, 123.456, -123.456, 1e10 + 0.5 };
        size_t i;
        for (i = 0; i < sizeof(xs) / sizeof(xs[0]); ++i) {
            double r = trunc(xs[i]);
            double ax = fabs(xs[i]);
            double ar = fabs(r);
            assert(ar <= ax);                 /* 绝对值不大于参数 */
            assert(is_integral_value(r));     /* 结果是整数值 */
            /* 与参数同号（或为零） */
            if (xs[i] > 0.0) assert(r >= 0.0);
            if (xs[i] < 0.0) assert(r <= 0.0);
        }
    }

    /* ---------- [2][3] 特殊值：0、无穷、NaN ---------- */
    assert(trunc(0.0)  == 0.0);
    assert(trunc(-0.0) == -0.0);
    assert(isinf(trunc(INFINITY))  && trunc(INFINITY)  > 0);
    assert(isinf(trunc(-INFINITY)) && trunc(-INFINITY) < 0);
    assert(isnan(trunc(NAN)));

    /* ---------- [1][2][3] truncf：float 版本 ---------- */
    {
        float rf = truncf(2.9f);
        assert(rf == 2.0f);
        assert(truncf(-2.9f) == -2.0f);
        assert(truncf(0.0f)  == 0.0f);
        assert(isinf(truncf(INFINITY)));
        assert(isnan(truncf(NAN)));
    }

    /* ---------- [1][2][3] truncl：long double 版本 ---------- */
    {
        long double rl = truncl(5.75L);
        assert(rl == 5.0L);
        assert(truncl(-5.75L) == -5.0L);
        assert(truncl(0.0L)   == 0.0L);
        assert(isinf(truncl(INFINITY)));
        assert(isnan(truncl(NAN)));
    }

    /* ---------- [2] 与 floor/ceil 的关系（仅作语义对照，非条款要求） ---------- */
    /* 对正数 trunc == floor；对负数 trunc == ceil。 */
    assert(trunc(4.9)  == floor(4.9));
    assert(trunc(-4.9) == ceil(-4.9));

    printf("C99 7.12.9.8 trunc/truncf/truncl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参个数必须与原型一致」：
 * trunc 原型为 double trunc(double)，只接受 1 个参数。
 * 期望报错：too many arguments to function 'trunc' / too few arguments。 */
void bad_arg_count(void)
{
    double a = trunc(1.0, 2.0);   /* 实参过多 */
    double b = trunc();           /* 实参过少 */
    (void)a; (void)b;
}

/* 违反约束「实参类型必须可转换为参数类型」：
 * 结构体类型无法隐式转换为 double，不能作为 trunc 的实参。
 * 期望报错：incompatible type for argument 1 of 'trunc'。 */
struct S { int x; };
void bad_arg_type(struct S s)
{
    double r = trunc(s);          /* 结构体不能转换为 double */
    (void)r;
}

/* 违反约束「函数返回类型固定」：
 * trunc 返回 double，不能把返回值赋给不兼容的指针类型。
 * 期望报错：incompatible types in assignment。 */
void bad_return_use(void)
{
    int *p = trunc(1.5);          /* double 不能赋给 int* */
    (void)p;
}

/* 违反约束「trunc 不是左值，不能赋值」：
 * 函数调用结果不是左值，不能作为赋值运算符的左操作数。
 * 期望报错：lvalue required as left operand of assignment。 */
void bad_assign_to_call(void)
{
    trunc(1.5) = 2.0;             /* 对非左值赋值 */
}

/* 违反约束「trunc 不是左值，不能取地址」：
 * 函数调用结果不是左值，不能应用一元 & 运算符。
 * 期望报错：lvalue required as unary '&' operand。 */
void bad_address_of_call(void)
{
    double *p = &trunc(1.5);      /* 对非左值取地址 */
    (void)p;
}

#endif /* 负向测试结束 */