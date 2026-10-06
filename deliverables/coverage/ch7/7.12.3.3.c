/*
 * 测试条款：C99 7.12.3.3  The isinf macro
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 头文件 <math.h> 与原型 int isinf(real-floating x);
 *   [2] 判断参数是否为无穷（正或负）；宽于语义类型的实参先转换为语义类型；
 *       判定基于实参的类型。
 *   [3] 当且仅当实参为无穷值时返回非零值。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <math.h> 提供 isinf 宏；此处已包含，可直接使用。 */

int main(void)
{
    /* ---------- [3] 基本语义：无穷返回非零，有限返回 0 ---------- */

    /* 正无穷 */
    double pinf = INFINITY;                 /* C99 <math.h> 提供 INFINITY */
    assert(isinf(pinf) != 0);               /* [3] 正无穷 -> 非零 */

    /* 负无穷 */
    double ninf = -INFINITY;
    assert(isinf(ninf) != 0);               /* [3] 负无穷 -> 非零 */

    /* 有限值 -> 0 */
    assert(isinf(0.0) == 0);
    assert(isinf(1.0) == 0);
    assert(isinf(-1.0) == 0);
    assert(isinf(DBL_MAX) == 0);            /* 最大有限 double 不是无穷 */
    assert(isinf(-DBL_MAX) == 0);

    /* 通过运算产生无穷 */
    double ovf = DBL_MAX * 2.0;             /* 溢出为 +inf（IEEE 环境） */
    assert(isinf(ovf) != 0);
    double novf = -DBL_MAX * 2.0;           /* 溢出为 -inf */
    assert(isinf(novf) != 0);

    /* 1.0 / 0.0 在 IEEE 环境下为 +inf */
    double divinf = 1.0 / 0.0;
    assert(isinf(divinf) != 0);

    /* ---------- [2] 判定基于实参的类型：float / double / long double ---------- */

    /* float 类型实参 */
    float finf = (float)INFINITY;
    assert(isinf(finf) != 0);               /* [2][3] float 无穷 */
    assert(isinf(1.0f) == 0);

    /* long double 类型实参 */
    long double linf = (long double)INFINITY;
    assert(isinf(linf) != 0);               /* [2][3] long double 无穷 */
    assert(isinf(1.0L) == 0);

    /* ---------- [2] 宽于语义类型的实参先转换为语义类型 ---------- */

    /* 将 long double 无穷赋给 double 变量，再判断：转换后仍为无穷 */
    double from_ld = (double)linf;
    assert(isinf(from_ld) != 0);

    /* 将 double 无穷赋给 float 变量，再判断：转换后仍为无穷 */
    float from_d = (float)pinf;
    assert(isinf(from_d) != 0);

    /* ---------- [3] 返回值语义：非零（不要求具体值），有限时为 0 ---------- */

    /* 非零即可，不假定等于 1 */
    assert(isinf(pinf) != 0);
    assert(isinf(ninf) != 0);
    /* 有限值必须恰好为 0 */
    assert(isinf(3.14) == 0);
    assert(isinf(-2.71) == 0);

    /* ---------- [2] 与 NaN 区分：NaN 不是无穷 ---------- */

    double nan_val = NAN;                   /* C99 <math.h> 提供 NAN */
    assert(isinf(nan_val) == 0);            /* NaN 不是无穷 -> 0 */

    /* ---------- [2] 与 isnan 配合，覆盖无穷/有限/NaN 三类 ---------- */

    assert(isinf(pinf) != 0 && isnan(pinf) == 0);
    assert(isinf(nan_val) == 0 && isnan(nan_val) != 0);
    assert(isinf(1.0) == 0 && isnan(1.0) == 0);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isinf 的实参必须为 real-floating 类型（float/double/long double）」：
 * 传入结构体类型，gcc -std=c99 应报错（类型不兼容 / 无法转换）。 */
struct S { int x; } s;
isinf(s);   /* 错误：实参不是 real-floating 类型 */

/* 违反约束「实参必须为 real-floating 类型」：
 * 传入指针类型，应编译报错。 */
double *p = 0;
isinf(p);   /* 错误：指针不是 real-floating 类型 */

/* 违反约束「实参必须为 real-floating 类型」：
 * 传入复数类型（C99 中 _Complex 不是 real-floating），应编译报错。 */
double _Complex z = 0.0;
isinf(z);   /* 错误：复数不是 real-floating 类型 */

/* 违反约束「实参必须为 real-floating 类型」：
 * 传入整数类型，应编译报错（整数不是 real-floating）。 */
isinf(42);  /* 错误：int 不是 real-floating 类型 */

/* 违反约束「isinf 是宏，需要 <math.h> 声明」：
 * 未包含 <math.h> 时使用 isinf，应报隐式声明错误（C99 禁止隐式函数声明）。 */
/* 注：本文件顶部已包含 <math.h>，此处仅示意；单独编译该片段应报错。 */
/* isinf(1.0); */

#endif