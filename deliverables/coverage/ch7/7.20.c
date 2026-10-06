/*
 * 测试 C99 7.20 <stdlib.h> General utilities
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反 7.20 的约束，编译器应报错。
 *
 * 覆盖段落：
 *   [1] 头文件声明 5 个类型、若干函数、若干宏
 *   [2] 类型 size_t / wchar_t / div_t / ldiv_t / lldiv_t
 *   [3] 宏 NULL / EXIT_FAILURE / EXIT_SUCCESS / RAND_MAX / MB_CUR_MAX
 */

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <wchar.h>

int main(void)
{
    /* ---------- [1] 头文件被包含后，其声明的函数可用 ---------- */
    /* 仅验证几个代表性函数可被调用（完整函数列表见 7.20.4~7.20.6） */
    {
        char *p = (char *)malloc(16);
        assert(p != NULL);
        free(p);

        int *q = (int *)calloc(4, sizeof(int));
        assert(q != NULL);
        assert(q[0] == 0 && q[1] == 0 && q[2] == 0 && q[3] == 0);
        free(q);

        int *r = (int *)realloc(NULL, 8 * sizeof(int));
        assert(r != NULL);
        free(r);

        assert(atoi("123") == 123);
        assert(atol("456") == 456L);
        assert(abs(-7) == 7);
        assert(labs(-8L) == 8L);
    }

    /* ---------- [2] 类型 size_t 与 wchar_t ---------- */
    {
        size_t sz = sizeof(int);
        assert(sz == sizeof(int));

        wchar_t wc = L'A';
        assert(wc == (wchar_t)'A');
    }

    /* ---------- [2] div_t 是 div 返回值的结构体类型 ---------- */
    {
        div_t d = div(17, 5);
        /* div_t 至少含 quot 与 rem 成员 */
        assert(d.quot == 3);
        assert(d.rem == 2);
        /* 类型一致性：div 的返回类型就是 div_t */
        div_t d2 = div(-17, 5);
        assert(d2.quot == -3);
        assert(d2.rem == -2);
    }

    /* ---------- [2] ldiv_t 是 ldiv 返回值的结构体类型 ---------- */
    {
        ldiv_t ld = ldiv(100L, 7L);
        assert(ld.quot == 14L);
        assert(ld.rem == 2L);
        ldiv_t ld2 = ldiv(-100L, 7L);
        assert(ld2.quot == -14L);
        assert(ld2.rem == -2L);
    }

    /* ---------- [2] lldiv_t 是 lldiv 返回值的结构体类型 ---------- */
    {
        lldiv_t lld = lldiv(1000000000000LL, 7LL);
        assert(lld.quot == 142857142857LL);
        assert(lld.rem == 1LL);
        lldiv_t lld2 = lldiv(-1000000000000LL, 7LL);
        assert(lld2.quot == -142857142857LL);
        assert(lld2.rem == -1LL);
    }

    /* ---------- [3] 宏 NULL ---------- */
    {
        void *np = NULL;
        assert(np == NULL);
        /* NULL 可用作指针比较 */
        char *cp = NULL;
        assert(cp == NULL);
    }

    /* ---------- [3] 宏 EXIT_SUCCESS / EXIT_FAILURE ---------- */
    {
        /* 二者均为整数常量表达式，可作为 exit 的实参 */
        int s = EXIT_SUCCESS;
        int f = EXIT_FAILURE;
        assert(s == 0);
        assert(f != 0);
        /* 整数常量表达式：可用于数组维度、case 标签等 */
        char arr_s[EXIT_SUCCESS + 1];
        char arr_f[EXIT_FAILURE];
        assert(sizeof(arr_s) == 1);
        assert(sizeof(arr_f) == (size_t)EXIT_FAILURE);
    }

    /* ---------- [3] 宏 RAND_MAX ---------- */
    {
        /* RAND_MAX 是整数常量表达式，是 rand 返回的最大值 */
        assert(RAND_MAX >= 32767);
        int v = rand();
        assert(v >= 0);
        assert(v <= RAND_MAX);
        /* 整数常量表达式：可用于数组维度 */
        char buf[RAND_MAX > 0 ? 1 : 1];
        assert(sizeof(buf) == 1);
    }

    /* ---------- [3] 宏 MB_CUR_MAX ---------- */
    {
        /* MB_CUR_MAX 是 size_t 类型的正整数表达式 */
        size_t m = MB_CUR_MAX;
        assert(m >= 1);
        /* 永不大于 MB_LEN_MAX */
        assert(m <= (size_t)MB_LEN_MAX);
        /* 类型为 size_t：与 size_t 变量比较不产生符号性警告 */
        size_t cmp = m;
        assert(cmp == MB_CUR_MAX);
    }

    printf("All positive tests for C99 7.20 passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「div_t 是结构体类型，其成员 quot/rem 为 int」：
 * 结构体不能直接参与算术运算，gcc -std=c99 应报错。 */
void neg_div_arith(void)
{
    div_t a = div(1, 1);
    div_t b = div(2, 2);
    div_t c = a + b;   /* error: invalid operands to binary + */
    (void)c;
}

/* 违反约束「ldiv_t 是结构体类型」：
 * 结构体不能直接与整数比较，gcc -std=c99 应报错。 */
void neg_ldiv_cmp(void)
{
    ldiv_t a = ldiv(1L, 1L);
    int x = (a == 0);  /* error: invalid operands to binary == */
    (void)x;
}

/* 违反约束「lldiv_t 是结构体类型」：
 * 结构体不能直接赋值给整数，gcc -std=c99 应报错。 */
void neg_lldiv_assign(void)
{
    lldiv_t a = lldiv(1LL, 1LL);
    long long x = a;   /* error: incompatible types when initializing */
    (void)x;
}

/* 违反约束「EXIT_SUCCESS / EXIT_FAILURE 是整数常量表达式」：
 * 不能对宏展开结果取地址（非常量左值），gcc -std=c99 应报错。 */
void neg_exit_addr(void)
{
    int *p = &EXIT_SUCCESS;   /* error: lvalue required as unary '&' operand */
    (void)p;
}

/* 违反约束「RAND_MAX 是整数常量表达式」：
 * 不能对宏展开结果赋值，gcc -std=c99 应报错。 */
void neg_randmax_assign(void)
{
    RAND_MAX = 1;   /* error: lvalue required as left operand of assignment */
}

/* 违反约束「MB_CUR_MAX 是 size_t 类型的表达式」：
 * 不能对宏展开结果取地址，gcc -std=c99 应报错。 */
void neg_mbcurmax_addr(void)
{
    size_t *p = &MB_CUR_MAX;   /* error: lvalue required as unary '&' operand */
    (void)p;
}

/* 违反约束「NULL 是空指针常量，可用于指针上下文」：
 * 将 NULL 用作结构体类型的初始化器（非指针），gcc -std=c99 应报错。 */
void neg_null_as_struct(void)
{
    div_t d = NULL;   /* error: incompatible types when initializing div_t */
    (void)d;
}

#endif /* 负向测试结束 */