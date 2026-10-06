/*
 * 测试 C99 7.12.4.6 —— sin 函数族 (sin / sinf / sinl)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 概要：<math.h> 中声明 double sin(double);
 *             float sinf(float); long double sinl(long double);
 *   [2] 描述：计算 x（弧度）的正弦。
 *   [3] 返回值：返回 sin x。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* 用于浮点近似比较的辅助函数 */
static int nearly_equal(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* [1] 概要：三个函数均可用，且返回类型分别为 double / float / long double。
     *     通过赋值给对应类型变量并检查类型宽度来验证。 */
    double      d;
    float       f;
    long double ld;

    d  = sin(0.0);
    f  = sinf(0.0f);
    ld = sinl(0.0L);

    /* 返回类型宽度检查（sizeof 反映声明类型） */
    assert(sizeof(d)  == sizeof(double));
    assert(sizeof(f)  == sizeof(float));
    assert(sizeof(ld) == sizeof(long double));

    /* [2][3] 描述与返回值：sin(0) == 0 */
    assert(nearly_equal(sin(0.0), 0.0, 1e-15));
    assert(nearly_equal((double)sinf(0.0f), 0.0, 1e-6));
    assert(nearly_equal((double)sinl(0.0L), 0.0, 1e-15));

    /* [2][3] sin(pi/2) == 1 */
    {
        double half_pi = 1.57079632679489661923; /* pi/2 */
        assert(nearly_equal(sin(half_pi), 1.0, 1e-12));
        assert(nearly_equal((double)sinf((float)half_pi), 1.0, 1e-6));
        assert(nearly_equal((double)sinl((long double)half_pi), 1.0, 1e-12));
    }

    /* [2][3] sin(pi) == 0 */
    {
        double pi = 3.14159265358979323846;
        assert(nearly_equal(sin(pi), 0.0, 1e-12));
        assert(nearly_equal((double)sinf((float)pi), 0.0, 1e-6));
        assert(nearly_equal((double)sinl((long double)pi), 0.0, 1e-12));
    }

    /* [2][3] sin(-x) == -sin(x)（奇函数性质，验证弧度语义） */
    {
        double x = 0.7;
        assert(nearly_equal(sin(-x), -sin(x), 1e-15));
        assert(nearly_equal((double)sinf(-0.7f), -(double)sinf(0.7f), 1e-6));
        assert(nearly_equal((double)sinl(-0.7L), -(double)sinl(0.7L), 1e-15));
    }

    /* [2][3] 一般值：sin(1.0) 的已知近似值 */
    assert(nearly_equal(sin(1.0), 0.84147098480789650665, 1e-12));
    assert(nearly_equal((double)sinf(1.0f), 0.84147098480789650665, 1e-6));
    assert(nearly_equal((double)sinl(1.0L), 0.84147098480789650665, 1e-12));

    /* [2][3] 周期性：sin(x + 2*pi) == sin(x) */
    {
        double pi = 3.14159265358979323846;
        double x  = 0.3;
        assert(nearly_equal(sin(x + 2.0 * pi), sin(x), 1e-12));
    }

    /* [3] 返回值在 [-1, 1] 范围内 */
    {
        double vals[] = { -10.0, -3.0, -1.0, 0.0, 1.0, 3.0, 10.0 };
        int i;
        for (i = 0; i < 7; ++i) {
            double r = sin(vals[i]);
            assert(r >= -1.0 && r <= 1.0);
        }
    }

    /* [1] 函数可被取地址（说明是真正的函数声明，而非宏） */
    {
        double (*fp)(double) = sin;
        float  (*fpf)(float) = sinf;
        long double (*fpl)(long double) = sinl;
        assert(nearly_equal(fp(0.0), 0.0, 1e-15));
        assert(nearly_equal((double)fpf(0.0f), 0.0, 1e-6));
        assert(nearly_equal((double)fpl(0.0L), 0.0, 1e-15));
    }

    printf("C99 7.12.4.6 sin functions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参必须为算术类型」：结构体不能传给 sin。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'sin' */
struct S { int x; };
struct S s;
double bad1 = sin(s);

/* 违反约束「实参必须为算术类型」：指针不能传给 sin。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'sin' */
double bad2 = sin("hello");

/* 违反约束「实参个数必须匹配」：sin 只接受 1 个实参。
 * gcc -std=c99 应报错：too many arguments to function 'sin' */
double bad3 = sin(1.0, 2.0);

/* 违反约束「实参个数必须匹配」：sin 需要 1 个实参。
 * gcc -std=c99 应报错：too few arguments to function 'sin' */
double bad4 = sin();

/* 违反约束「sinf 的实参必须为算术类型」：结构体不能传给 sinf。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'sinf' */
float bad5 = sinf(s);

/* 违反约束「sinl 的实参必须为算术类型」：指针不能传给 sinl。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'sinl' */
long double bad6 = sinl(&s);

/* 违反约束「返回值不可作为左值赋值」：函数调用结果不是左值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
void bad7(void) { sin(0.0) = 1.0; }

/* 违反约束「返回值不可取地址」：函数调用结果不是左值。
 * gcc -std=c99 应报错：lvalue required as unary '&' operand */
void bad8(void) { double *p = &sin(0.0); (void)p; }

#endif /* 负向测试结束 */