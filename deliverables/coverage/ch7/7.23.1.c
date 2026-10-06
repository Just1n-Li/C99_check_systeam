/*
 * 测试 C99 条款 7.23.1 —— <time.h> 的 Components of time
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] <time.h> 定义宏、声明类型与函数；日历时间/本地时间/夏令时概念。
 *   [2] 宏 NULL 与 CLOCKS_PER_SEC（类型为 clock_t 的表达式）。
 *   [3] 类型 size_t、clock_t、time_t（算术类型）、struct tm。
 *   [4] struct tm 至少含 9 个成员及其语义/范围；tm_isdst 的取值含义。
 */

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] CLOCKS_PER_SEC 展开为类型为 clock_t 的表达式 */
static clock_t check_clocks_per_sec_type(void)
{
    clock_t cps = CLOCKS_PER_SEC;   /* 类型应为 clock_t */
    return cps;
}

/* [2] NULL 宏（7.17 描述）在 <time.h> 中可用 */
static void *check_null_macro(void)
{
    void *p = NULL;
    return p;
}

/* [3] size_t 类型可用（7.17 描述） */
static size_t check_size_t_type(void)
{
    size_t s = sizeof(int);
    return s;
}

/* [3] clock_t 与 time_t 是算术类型 */
static void check_arithmetic_types(void)
{
    clock_t c = (clock_t)1;
    time_t  t = (time_t)1;

    /* 算术类型支持 + - * / 等运算 */
    c = c + (clock_t)1;
    t = t + (time_t)1;
    c = c * (clock_t)2;
    t = t - (time_t)1;

    assert(c >= (clock_t)0);
    (void)t;
}

/* [4] struct tm 至少包含 9 个成员，且成员类型为 int */
static void check_tm_members(void)
{
    struct tm tmv;

    /* 逐个成员赋值，验证成员存在且为 int 类型 */
    tmv.tm_sec   = 0;    /* [0, 60] */
    tmv.tm_min   = 0;    /* [0, 59] */
    tmv.tm_hour  = 0;    /* [0, 23] */
    tmv.tm_mday  = 1;    /* [1, 31] */
    tmv.tm_mon   = 0;    /* [0, 11] */
    tmv.tm_year  = 0;    /* years since 1900 */
    tmv.tm_wday  = 0;    /* [0, 6] */
    tmv.tm_yday  = 0;    /* [0, 365] */
    tmv.tm_isdst = 0;    /* DST flag */

    /* 成员类型为 int：用 int 指针接收成员地址，验证类型匹配 */
    {
        int *psec   = &tmv.tm_sec;
        int *pmin   = &tmv.tm_min;
        int *phour  = &tmv.tm_hour;
        int *pmday  = &tmv.tm_mday;
        int *pmon   = &tmv.tm_mon;
        int *pyear  = &tmv.tm_year;
        int *pwday  = &tmv.tm_wday;
        int *pyday  = &tmv.tm_yday;
        int *pisdst = &tmv.tm_isdst;
        assert(*psec == 0 && *pmin == 0 && *phour == 0);
        assert(*pmday == 1 && *pmon == 0 && *pyear == 0);
        assert(*pwday == 0 && *pyday == 0 && *pisdst == 0);
    }

    /* [4] tm_sec 范围允许闰秒 [0, 60] */
    tmv.tm_sec = 60;
    assert(tmv.tm_sec == 60);

    /* [4] tm_isdst 语义：正=夏令时生效，0=不生效，负=信息不可用 */
    tmv.tm_isdst = 1;
    assert(tmv.tm_isdst > 0);   /* 夏令时生效 */
    tmv.tm_isdst = 0;
    assert(tmv.tm_isdst == 0);  /* 夏令时不生效 */
    tmv.tm_isdst = -1;
    assert(tmv.tm_isdst < 0);   /* 信息不可用 */
}

/* [1] 使用 <time.h> 声明的函数处理日历时间/本地时间 */
static void check_time_functions(void)
{
    time_t now = time(NULL);        /* 日历时间 */
    struct tm *lt = localtime(&now);/* 本地时间（含夏令时处理） */
    struct tm *gt = gmtime(&now);   /* UTC 时间 */

    assert(now != (time_t)-1);
    assert(lt != NULL);
    assert(gt != NULL);

    /* 本地时间各分量应在正常范围内 */
    assert(lt->tm_sec  >= 0 && lt->tm_sec  <= 60);
    assert(lt->tm_min  >= 0 && lt->tm_min  <= 59);
    assert(lt->tm_hour >= 0 && lt->tm_hour <= 23);
    assert(lt->tm_mday >= 1 && lt->tm_mday <= 31);
    assert(lt->tm_mon  >= 0 && lt->tm_mon  <= 11);
    assert(lt->tm_wday >= 0 && lt->tm_wday <= 6);
    assert(lt->tm_yday >= 0 && lt->tm_yday <= 365);

    /* clock() 返回处理器时间，配合 CLOCKS_PER_SEC 使用 */
    {
        clock_t start = clock();
        clock_t end   = clock();
        assert(end >= start);
        (void)CLOCKS_PER_SEC;
    }
}

int main(void)
{
    /* [2] */
    assert(check_clocks_per_sec_type() > (clock_t)0);
    assert(check_null_macro() == NULL);

    /* [3] */
    assert(check_size_t_type() == sizeof(int));
    check_arithmetic_types();

    /* [4] */
    check_tm_members();

    /* [1] */
    check_time_functions();

    printf("C99 7.23.1 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「struct tm 成员为 int 类型」：
 * 将成员地址赋给非 int 指针（如 double*）应报错（类型不兼容）。
 * gcc -std=c99 期望：assignment ... from incompatible pointer type / 类型不匹配。 */
void neg_tm_member_type(void)
{
    struct tm tmv;
    double *p = &tmv.tm_sec;   /* 错误：tm_sec 是 int，不是 double */
    (void)p;
}

/* 违反约束「CLOCKS_PER_SEC 是类型为 clock_t 的表达式」：
 * 将其赋给不兼容的指针类型应报错。 */
void neg_clocks_per_sec_type(void)
{
    int *p = CLOCKS_PER_SEC;   /* 错误：CLOCKS_PER_SEC 是算术表达式，非指针 */
    (void)p;
}

/* 违反约束「clock_t / time_t 是算术类型」：
 * 对算术类型使用结构体成员访问运算符 . 应报错。 */
void neg_arithmetic_not_struct(void)
{
    clock_t c = 0;
    c.tm_sec = 1;              /* 错误：clock_t 是算术类型，无成员 */
}

/* 违反约束「struct tm 成员为 int」：
 * 用结构体类型给 int 成员赋值应报错。 */
void neg_assign_struct_to_int_member(void)
{
    struct tm tmv;
    struct tm other;
    tmv.tm_sec = other;        /* 错误：不能把 struct tm 赋给 int */
}

/* 违反约束「NULL 是空指针常量」：
 * 把 NULL 当作结构体对象使用（成员访问）应报错。 */
void neg_null_member_access(void)
{
    NULL.tm_sec = 0;           /* 错误：NULL 不是结构体对象 */
}

#endif /* 负向测试结束 */