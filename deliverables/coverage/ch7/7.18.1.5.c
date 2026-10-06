/*
 * 测试 C99 7.18.1.5 —— Greatest-width integer types
 *
 * 条款要求：
 *   [1] 必须提供 intmax_t  —— 能表示任意有符号整数类型的所有值的有符号整数类型
 *   [1] 必须提供 uintmax_t —— 能表示任意无符号整数类型的所有值的无符号整数类型
 *
 * 预期行为：
 *   正向测试：包含 <stdint.h> 后 intmax_t / uintmax_t 可用，
 *             且其宽度 >= 所有标准有符号/无符号整数类型的宽度，
 *             能无损容纳各类型的最小值/最大值，程序编译并运行通过。
 *   负向测试：违反约束的片段（如未包含 <stdint.h> 就使用 intmax_t、
 *             把 intmax_t 当结构体用等）应导致编译报错。
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] intmax_t / uintmax_t 必须存在且为整数类型 */
static intmax_t  g_signed_max   = 0;
static uintmax_t g_unsigned_max = 0;

int main(void)
{
    /* [1] 类型存在性：能声明对象即证明类型被定义 */
    intmax_t  im  = 0;
    uintmax_t uim = 0;
    (void)im;
    (void)uim;

    /* [1] intmax_t 是有符号整数类型：可表示负值 */
    im = -1;
    assert(im < 0);

    /* [1] uintmax_t 是无符号整数类型：-1 转换为该类型的最大值 */
    uim = (uintmax_t)-1;
    assert(uim > 0);
    assert(uim == (uintmax_t)UINTMAX_MAX);

    /* [1] 宽度不小于任何有符号整数类型：能无损容纳各类型极值 */
    {
        signed char        sc = SCHAR_MIN;
        short              sh = SHRT_MIN;
        int                i  = INT_MIN;
        long               l  = LONG_MIN;
        long long          ll = LLONG_MIN;

        intmax_t a = sc;
        intmax_t b = sh;
        intmax_t c = i;
        intmax_t d = l;
        intmax_t e = ll;

        assert(a == (intmax_t)SCHAR_MIN);
        assert(b == (intmax_t)SHRT_MIN);
        assert(c == (intmax_t)INT_MIN);
        assert(d == (intmax_t)LONG_MIN);
        assert(e == (intmax_t)LLONG_MIN);

        /* 各类型最大值也应无损容纳 */
        assert((intmax_t)SCHAR_MAX == (intmax_t)SCHAR_MAX);
        assert((intmax_t)SHRT_MAX  == (intmax_t)SHRT_MAX);
        assert((intmax_t)INT_MAX   == (intmax_t)INT_MAX);
        assert((intmax_t)LONG_MAX  == (intmax_t)LONG_MAX);
        assert((intmax_t)LLONG_MAX == (intmax_t)LLONG_MAX);
    }

    /* [1] 宽度不小于任何无符号整数类型：能无损容纳各类型极值 */
    {
        unsigned char      uc = UCHAR_MAX;
        unsigned short     us = USHRT_MAX;
        unsigned int       ui = UINT_MAX;
        unsigned long      ul = ULONG_MAX;
        unsigned long long ull = ULLONG_MAX;

        uintmax_t a = uc;
        uintmax_t b = us;
        uintmax_t c = ui;
        uintmax_t d = ul;
        uintmax_t e = ull;

        assert(a == (uintmax_t)UCHAR_MAX);
        assert(b == (uintmax_t)USHRT_MAX);
        assert(c == (uintmax_t)UINT_MAX);
        assert(d == (uintmax_t)ULONG_MAX);
        assert(e == (uintmax_t)ULLONG_MAX);
    }

    /* [1] 宽度关系：intmax_t 至少与 long long 一样宽 */
    assert(sizeof(intmax_t)  >= sizeof(long long));
    assert(sizeof(uintmax_t) >= sizeof(unsigned long long));

    /* [1] 有符号/无符号对应关系：两者宽度相同 */
    assert(sizeof(intmax_t) == sizeof(uintmax_t));

    /* [1] 宏 INTMAX_MAX / UINTMAX_MAX 与类型一致 */
    assert((intmax_t)INTMAX_MAX  == (intmax_t)INTMAX_MAX);
    assert((uintmax_t)UINTMAX_MAX == (uintmax_t)UINTMAX_MAX);

    /* [1] 全局对象也可用该类型 */
    g_signed_max   = INTMAX_MAX;
    g_unsigned_max = UINTMAX_MAX;
    assert(g_signed_max   == INTMAX_MAX);
    assert(g_unsigned_max == UINTMAX_MAX);

    /* [1] 算术运算在该类型上正常工作 */
    {
        intmax_t  x = (intmax_t)1000000 * (intmax_t)1000000;
        uintmax_t y = (uintmax_t)1000000u * (uintmax_t)1000000u;
        assert(x == (intmax_t)1000000000000LL);
        assert(y == (uintmax_t)1000000000000ULL);
    }

    printf("C99 7.18.1.5 positive tests passed.\n");
    printf("sizeof(intmax_t)  = %zu\n", sizeof(intmax_t));
    printf("sizeof(uintmax_t) = %zu\n", sizeof(uintmax_t));
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「intmax_t 是 <stdint.h> 中声明的类型名」：
 * 未包含 <stdint.h> 时使用 intmax_t，gcc -std=c99 应报错
 *   error: unknown type name 'intmax_t'
 */
intmax_t no_header_use;

/* 违反约束「intmax_t 是整数类型」：
 * 把 intmax_t 当作结构体标签使用，应报错
 *   error: 'intmax_t' defined as wrong kind of tag
 */
struct intmax_t { int x; };

/* 违反约束「uintmax_t 是整数类型」：
 * 对 uintmax_t 类型的值做成员访问，应报错
 *   error: request for member 'x' in something not a structure or union
 */
void bad_member(void)
{
    uintmax_t u = 0;
    u.x;            /* uintmax_t 不是结构体/联合体 */
}

/* 违反约束「intmax_t 是标量类型」：
 * 对标量类型做数组下标以外的非法解引用，应报错
 *   error: invalid type argument of unary '*' (have 'intmax_t')
 */
void bad_deref(void)
{
    intmax_t v = 0;
    *v;             /* 不能对整数解引用 */
}

/* 违反约束「intmax_t 是算术类型」：
 * 对 intmax_t 类型的值做函数调用，应报错
 *   error: called object is not a function or function pointer
 */
void bad_call(void)
{
    intmax_t f = 0;
    f();            /* 整数不是可调用对象 */
}

#endif