/*
 * 测试 C99 7.20.6.2 —— div / ldiv / lldiv 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             这些片段统一放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] 函数原型（Synopsis）：div_t div(int,int);
 *       ldiv_t ldiv(long,long); lldiv_t lldiv(long long,long long);
 *   [2] 语义：一次运算同时计算 numer/denom 与 numer%denom。
 *   [3] 返回值：返回结构体，含成员 quot 与 rem，类型与实参相同；
 *       结果不可表示时行为未定义（UB，不作为负向测试）。
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，若原型不匹配则编译失败 */
static div_t  (*p_div)(int, int)                 = div;
static ldiv_t (*p_ldiv)(long, long)              = ldiv;
static lldiv_t(*p_lldiv)(long long, long long)   = lldiv;

/* [3] 结构体成员 quot / rem 存在，且类型与实参类型相同 */
static void check_member_types(void)
{
    div_t d;
    ldiv_t ld;
    lldiv_t lld;

    /* 成员类型必须与实参类型一致：int / long / long long */
    int        *pq  = &d.quot;   /* div_t.quot 为 int */
    int        *pr  = &d.rem;    /* div_t.rem  为 int */
    long       *plq = &ld.quot;  /* ldiv_t.quot 为 long */
    long       *plr = &ld.rem;   /* ldiv_t.rem  为 long */
    long long  *pllq= &lld.quot; /* lldiv_t.quot 为 long long */
    long long  *pllr= &lld.rem;  /* lldiv_t.rem  为 long long */

    (void)pq; (void)pr; (void)plq; (void)plr; (void)pllq; (void)pllr;
}

/* [2][3] div：同时得到商与余数，且满足 numer == quot*denom + rem */
static void test_div(void)
{
    div_t r;

    r = div(7, 3);
    assert(r.quot == 2);
    assert(r.rem  == 1);
    assert(7 == r.quot * 3 + r.rem);

    r = div(-7, 3);
    assert(r.quot == -2);
    assert(r.rem  == -1);
    assert(-7 == r.quot * 3 + r.rem);

    r = div(7, -3);
    assert(r.quot == -2);
    assert(r.rem  == 1);
    assert(7 == r.quot * -3 + r.rem);

    r = div(-7, -3);
    assert(r.quot == 2);
    assert(r.rem  == -1);
    assert(-7 == r.quot * -3 + r.rem);

    /* 整除情形：余数为 0 */
    r = div(12, 4);
    assert(r.quot == 3);
    assert(r.rem  == 0);

    /* 被除数绝对值小于除数 */
    r = div(2, 5);
    assert(r.quot == 0);
    assert(r.rem  == 2);

    /* 边界：INT_MIN / 1 可表示 */
    r = div(INT_MIN, 1);
    assert(r.quot == INT_MIN);
    assert(r.rem  == 0);
}

/* [2][3] ldiv */
static void test_ldiv(void)
{
    ldiv_t r;

    r = ldiv(100000L, 7L);
    assert(r.quot == 14285L);
    assert(r.rem  == 5L);
    assert(100000L == r.quot * 7L + r.rem);

    r = ldiv(-100000L, 7L);
    assert(r.quot == -14285L);
    assert(r.rem  == -5L);
    assert(-100000L == r.quot * 7L + r.rem);

    r = ldiv(0L, 9L);
    assert(r.quot == 0L);
    assert(r.rem  == 0L);

    r = ldiv(LONG_MIN, 1L);
    assert(r.quot == LONG_MIN);
    assert(r.rem  == 0L);
}

/* [2][3] lldiv */
static void test_lldiv(void)
{
    lldiv_t r;

    r = lldiv(10000000000LL, 7LL);
    assert(r.quot == 1428571428LL);
    assert(r.rem  == 4LL);
    assert(10000000000LL == r.quot * 7LL + r.rem);

    r = lldiv(-10000000000LL, 7LL);
    assert(r.quot == -1428571428LL);
    assert(r.rem  == -4LL);
    assert(-10000000000LL == r.quot * 7LL + r.rem);

    r = lldiv(0LL, 3LL);
    assert(r.quot == 0LL);
    assert(r.rem  == 0LL);

    r = lldiv(LLONG_MIN, 1LL);
    assert(r.quot == LLONG_MIN);
    assert(r.rem  == 0LL);
}

/* [3] 返回值是结构体（非左值），可直接读取成员 */
static void test_return_is_struct(void)
{
    /* 直接对函数返回值取成员，合法（读取） */
    assert(div(9, 2).quot == 4);
    assert(div(9, 2).rem  == 1);
    assert(ldiv(9L, 2L).quot == 4L);
    assert(lldiv(9LL, 2LL).rem == 1LL);
}

int main(void)
{
    check_member_types();
    test_div();
    test_ldiv();
    test_lldiv();
    test_return_is_struct();

    printf("C99 7.20.6.2 div/ldiv/lldiv: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数返回值不是左值」：对 div() 返回结构体的成员赋值应报错。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment" */
void neg_assign_to_return_member(void)
{
    div(9, 2).quot = 5;   /* error: 非左值不能赋值 */
}

/* 违反约束「函数返回值不是左值」：对 ldiv() 返回结构体的成员赋值应报错。 */
void neg_assign_to_return_member2(void)
{
    ldiv(9L, 2L).rem = 0; /* error: 非左值不能赋值 */
}

/* 违反约束「函数返回值不是左值」：对 lldiv() 返回结构体的成员赋值应报错。 */
void neg_assign_to_return_member3(void)
{
    lldiv(9LL, 2LL).quot = 1; /* error: 非左值不能赋值 */
}

/* 违反约束「实参类型必须与原型匹配」：div 需要 int，传指针应报错。
 * 期望：gcc -std=c99 报 incompatible type / 参数类型不匹配 */
void neg_wrong_arg_type(void)
{
    int x = 1;
    div(&x, 2);           /* error: 第一个实参应为 int，不是 int* */
}

/* 违反约束「实参个数必须与原型一致」：div 需要两个实参。 */
void neg_too_few_args(void)
{
    div(9);               /* error: 实参个数太少 */
}

/* 违反约束「实参个数必须与原型一致」：ldiv 需要两个实参。 */
void neg_too_many_args(void)
{
    ldiv(9L, 2L, 3L);     /* error: 实参个数太多 */
}

/* 违反约束「div_t 成员名固定为 quot / rem」：不存在名为 quotient 的成员。
 * 期望：gcc -std=c99 报 "no member named 'quotient'" */
void neg_wrong_member_name(void)
{
    div_t d = div(9, 2);
    int q = d.quotient;   /* error: div_t 无此成员 */
    (void)q;
}

/* 违反约束「div_t 成员名固定为 quot / rem」：不存在名为 remainder 的成员。 */
void neg_wrong_member_name2(void)
{
    ldiv_t d = ldiv(9L, 2L);
    long r = d.remainder; /* error: ldiv_t 无此成员 */
    (void)r;
}

/* 违反约束「div 返回 div_t，不能赋给不兼容类型」：
 * 结构体不能隐式转换为 int。 */
void neg_incompatible_return(void)
{
    int q = div(9, 2);    /* error: div_t 不能隐式转换为 int */
    (void)q;
}

/* 违反约束「div 返回 div_t，不能赋给不兼容类型」：
 * 结构体不能隐式转换为 long。 */
void neg_incompatible_return2(void)
{
    long q = ldiv(9L, 2L); /* error: ldiv_t 不能隐式转换为 long */
    (void)q;
}

#endif /* 负向测试结束 */