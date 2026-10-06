/*
 * 测试条款：C99 7.7  <float.h> —— 浮点类型的特性
 *
 * 预期行为：
 *   正向测试：包含 <float.h> 后，其中定义的宏（FLT_/DBL_/LDBL_ 系列）应能
 *             正常展开为常量表达式，且满足 5.2.4.2.2 中列出的约束与语义。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反 5.2.4.2.2 约束的用法（例如把浮点宏当作左值赋值、
 *             把 FLT_RADIX 当作非整数常量使用等）应导致编译报错。
 *             这些片段放在 #if 0 中，保证本文件仍可正常编译运行。
 *
 * 覆盖段落：
 *   [1] <float.h> 定义若干宏，展开为标准浮点类型的各种极限与参数。
 *   [2] 这些宏的含义、取值约束见 5.2.4.2.2。
 */

#include <float.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件存在且宏可展开：只要包含 <float.h> 就能使用这些宏。
 *     这里用它们初始化对象，验证宏确实展开为可用的常量表达式。 */
static const int    radix        = FLT_RADIX;
static const int    flt_mant     = FLT_MANT_DIG;
static const int    dbl_mant     = DBL_MANT_DIG;
static const int    ldbl_mant    = LDBL_MANT_DIG;
static const int    flt_dig      = FLT_DIG;
static const int    dbl_dig      = DBL_DIG;
static const int    ldbl_dig     = LDBL_DIG;
static const int    flt_min_exp  = FLT_MIN_EXP;
static const int    dbl_min_exp  = DBL_MIN_EXP;
static const int    ldbl_min_exp = LDBL_MIN_EXP;
static const int    flt_max_exp  = FLT_MAX_EXP;
static const int    dbl_max_exp  = DBL_MAX_EXP;
static const int    ldbl_max_exp = LDBL_MAX_EXP;
static const int    flt_min_10   = FLT_MIN_10_EXP;
static const int    dbl_min_10   = DBL_MIN_10_EXP;
static const int    ldbl_min_10  = LDBL_MIN_10_EXP;
static const int    flt_max_10   = FLT_MAX_10_EXP;
static const int    dbl_max_10   = DBL_MAX_10_EXP;
static const int    ldbl_max_10  = LDBL_MAX_10_EXP;
static const float  flt_eps      = FLT_EPSILON;
static const double dbl_eps      = DBL_EPSILON;
static const long double ldbl_eps = LDBL_EPSILON;
static const float  flt_min      = FLT_MIN;
static const double dbl_min      = DBL_MIN;
static const long double ldbl_min = LDBL_MIN;
static const float  flt_max      = FLT_MAX;
static const double dbl_max      = DBL_MAX;
static const long double ldbl_max = LDBL_MAX;

/* [2] 5.2.4.2.2 约束：FLT_RADIX 是浮点表示的基数，必须 >= 2 */
static void test_radix(void)
{
    assert(FLT_RADIX >= 2);
    assert(radix == FLT_RADIX);
}

/* [2] 5.2.4.2.2：FLT_MANT_DIG / DBL_MANT_DIG / LDBL_MANT_DIG 是基数 b 下
 *     尾数位数，必须为正整数。 */
static void test_mant_dig(void)
{
    assert(FLT_MANT_DIG  >= 1);
    assert(DBL_MANT_DIG  >= 1);
    assert(LDBL_MANT_DIG >= 1);
    assert(flt_mant == FLT_MANT_DIG);
    assert(dbl_mant == DBL_MANT_DIG);
    assert(ldbl_mant == LDBL_MANT_DIG);
}

/* [2] 5.2.4.2.2：FLT_DIG / DBL_DIG / LDBL_DIG 是十进制有效数字位数，
 *     必须 >= 6（float）、>= 10（double）、>= 10（long double）。 */
static void test_dig(void)
{
    assert(FLT_DIG  >= 6);
    assert(DBL_DIG  >= 10);
    assert(LDBL_DIG >= 10);
    assert(flt_dig == FLT_DIG);
    assert(dbl_dig == DBL_DIG);
    assert(ldbl_dig == LDBL_DIG);
}

/* [2] 5.2.4.2.2：FLT_MIN_EXP / DBL_MIN_EXP / LDBL_MIN_EXP 是使 b^(e-1)
 *     为规格化数的最小负指数 e，必须为负。 */
static void test_min_exp(void)
{
    assert(FLT_MIN_EXP  < 0);
    assert(DBL_MIN_EXP  < 0);
    assert(LDBL_MIN_EXP < 0);
    assert(flt_min_exp == FLT_MIN_EXP);
    assert(dbl_min_exp == DBL_MIN_EXP);
    assert(ldbl_min_exp == LDBL_MIN_EXP);
}

/* [2] 5.2.4.2.2：FLT_MAX_EXP / DBL_MAX_EXP / LDBL_MAX_EXP 是使 b^(e-1)
 *     为可表示有限数的最大正指数 e，必须为正。 */
static void test_max_exp(void)
{
    assert(FLT_MAX_EXP  > 0);
    assert(DBL_MAX_EXP  > 0);
    assert(LDBL_MAX_EXP > 0);
    assert(flt_max_exp == FLT_MAX_EXP);
    assert(dbl_max_exp == DBL_MAX_EXP);
    assert(ldbl_max_exp == LDBL_MAX_EXP);
}

/* [2] 5.2.4.2.2：FLT_MIN_10_EXP / DBL_MIN_10_EXP / LDBL_MIN_10_EXP 是
 *     使 10^n 为规格化数的最小负十进制指数 n，必须为负。 */
static void test_min_10_exp(void)
{
    assert(FLT_MIN_10_EXP  < 0);
    assert(DBL_MIN_10_EXP  < 0);
    assert(LDBL_MIN_10_EXP < 0);
    assert(flt_min_10 == FLT_MIN_10_EXP);
    assert(dbl_min_10 == DBL_MIN_10_EXP);
    assert(ldbl_min_10 == LDBL_MIN_10_EXP);
}

/* [2] 5.2.4.2.2：FLT_MAX_10_EXP / DBL_MAX_10_EXP / LDBL_MAX_10_EXP 是
 *     使 10^n 为可表示有限数的最大正十进制指数 n，必须为正。 */
static void test_max_10_exp(void)
{
    assert(FLT_MAX_10_EXP  > 0);
    assert(DBL_MAX_10_EXP  > 0);
    assert(LDBL_MAX_10_EXP > 0);
    assert(flt_max_10 == FLT_MAX_10_EXP);
    assert(dbl_max_10 == DBL_MAX_10_EXP);
    assert(ldbl_max_10 == LDBL_MAX_10_EXP);
}

/* [2] 5.2.4.2.2：FLT_EPSILON / DBL_EPSILON / LDBL_EPSILON 是 1 与大于 1
 *     的最小可表示数之差，必须为正且 < 1。 */
static void test_epsilon(void)
{
    assert(FLT_EPSILON  > 0.0f);
    assert(DBL_EPSILON  > 0.0);
    assert(LDBL_EPSILON > 0.0L);
    assert(FLT_EPSILON  < 1.0f);
    assert(DBL_EPSILON  < 1.0);
    assert(LDBL_EPSILON < 1.0L);
    assert(flt_eps == FLT_EPSILON);
    assert(dbl_eps == DBL_EPSILON);
    assert(ldbl_eps == LDBL_EPSILON);
}

/* [2] 5.2.4.2.2：FLT_MIN / DBL_MIN / LDBL_MIN 是最小规格化正数，必须 > 0。 */
static void test_min(void)
{
    assert(FLT_MIN  > 0.0f);
    assert(DBL_MIN  > 0.0);
    assert(LDBL_MIN > 0.0L);
    assert(flt_min == FLT_MIN);
    assert(dbl_min == DBL_MIN);
    assert(ldbl_min == LDBL_MIN);
}

/* [2] 5.2.4.2.2：FLT_MAX / DBL_MAX / LDBL_MAX 是最大有限数，必须 > 0，
 *     且大于对应的 MIN。 */
static void test_max(void)
{
    assert(FLT_MAX  > 0.0f);
    assert(DBL_MAX  > 0.0);
    assert(LDBL_MAX > 0.0L);
    assert(FLT_MAX  > FLT_MIN);
    assert(DBL_MAX  > DBL_MIN);
    assert(LDBL_MAX > LDBL_MIN);
    assert(flt_max == FLT_MAX);
    assert(dbl_max == DBL_MAX);
    assert(ldbl_max == LDBL_MAX);
}

/* [2] 5.2.4.2.2：类型宽度关系 —— long double 的精度/范围不小于 double，
 *     double 不小于 float（标准要求 DBL_MANT_DIG >= FLT_MANT_DIG 等）。 */
static void test_type_ordering(void)
{
    assert(DBL_MANT_DIG  >= FLT_MANT_DIG);
    assert(LDBL_MANT_DIG >= DBL_MANT_DIG);
    assert(DBL_MAX_EXP   >= FLT_MAX_EXP);
    assert(LDBL_MAX_EXP  >= DBL_MAX_EXP);
    assert(DBL_MIN_EXP   <= FLT_MIN_EXP);
    assert(LDBL_MIN_EXP  <= DBL_MIN_EXP);
    assert(DBL_DIG       >= FLT_DIG);
    assert(LDBL_DIG      >= DBL_DIG);
}

/* [2] 5.2.4.2.2：FLT_EPSILON 与 FLT_MANT_DIG 的关系：
 *     FLT_EPSILON == b^(1 - FLT_MANT_DIG)（当 b 为 2 时精确成立）。
 *     这里用 pow 做数值验证，允许浮点误差。 */
static void test_epsilon_relation(void)
{
    double expected = pow((double)FLT_RADIX, 1.0 - (double)FLT_MANT_DIG);
    double got      = (double)FLT_EPSILON;
    double rel      = fabs(got - expected) / expected;
    assert(rel < 1e-6);
}

/* [2] 5.2.4.2.2：FLT_MIN 与 FLT_MIN_EXP 的关系：
 *     FLT_MIN == b^(FLT_MIN_EXP - 1)（b 为 2 时精确成立）。 */
static void test_min_relation(void)
{
    double expected = pow((double)FLT_RADIX, (double)FLT_MIN_EXP - 1.0);
    double got      = (double)FLT_MIN;
    double rel      = fabs(got - expected) / expected;
    assert(rel < 1e-6);
}

/* [2] 5.2.4.2.2：FLT_MAX 与 FLT_MAX_EXP 的关系：
 *     FLT_MAX == (1 - b^(-FLT_MANT_DIG)) * b^FLT_MAX_EXP（b 为 2 时成立）。 */
static void test_max_relation(void)
{
    double b        = (double)FLT_RADIX;
    double expected = (1.0 - pow(b, -(double)FLT_MANT_DIG)) * pow(b, (double)FLT_MAX_EXP);
    double got      = (double)FLT_MAX;
    double rel      = fabs(got - expected) / expected;
    assert(rel < 1e-6);
}

/* [1] 宏可用于常量表达式（如数组维度、case 标签、静态初始化）。 */
static int arr[FLT_MANT_DIG > 0 ? 1 : -1];   /* 编译期常量表达式 */

static void test_constant_expression(void)
{
    int local[DBL_MANT_DIG > 0 ? 1 : -1];
    (void)local;
    assert(sizeof(arr) == sizeof(int));
}

int main(void)
{
    test_radix();
    test_mant_dig();
    test_dig();
    test_min_exp();
    test_max_exp();
    test_min_10_exp();
    test_max_10_exp();
    test_epsilon();
    test_min();
    test_max();
    test_type_ordering();
    test_epsilon_relation();
    test_min_relation();
    test_max_relation();
    test_constant_expression();

    printf("C99 7.7 <float.h> 正向测试全部通过\n");
    printf("FLT_RADIX=%d FLT_MANT_DIG=%d DBL_MANT_DIG=%d LDBL_MANT_DIG=%d\n",
           FLT_RADIX, FLT_MANT_DIG, DBL_MANT_DIG, LDBL_MANT_DIG);
    printf("FLT_DIG=%d DBL_DIG=%d LDBL_DIG=%d\n",
           FLT_DIG, DBL_DIG, LDBL_DIG);
    printf("FLT_MIN=%e FLT_MAX=%e FLT_EPSILON=%e\n",
           (double)FLT_MIN, (double)FLT_MAX, (double)FLT_EPSILON);
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「<float.h> 中的宏展开为常量表达式，不是左值」：
 * 对 FLT_MAX 赋值应编译报错（gcc -std=c99 报 "lvalue required as left operand of assignment"）。 */
void neg_assign_macro(void)
{
    FLT_MAX = 1.0f;
}

/* 违反约束「FLT_RADIX 展开为整数常量表达式」：
 * 把它用作需要整型常量表达式的场合（如 case 标签）时，若被当作非整数则报错；
 * 这里故意把它当作浮点左值使用，应报错。 */
void neg_radix_as_lvalue(void)
{
    FLT_RADIX = 10;
}

/* 违反约束「FLT_EPSILON 等宏展开为浮点常量，不是可修改对象」：
 * 对 DBL_EPSILON 取地址并赋值应报错（宏不是对象，无法取地址）。 */
void neg_take_address(void)
{
    double *p = &DBL_EPSILON;
    *p = 0.0;
}

/* 违反约束「宏展开为算术常量表达式」：
 * 把 FLT_MANT_DIG 用作结构体成员名/标签等非表达式场合应报错。 */
struct neg_struct {
    int FLT_MANT_DIG;   /* 宏展开为整数常量，不能作为成员名 */
};

/* 违反约束「FLT_MAX 是浮点常量」：
 * 在需要整型常量表达式的位域宽度中使用浮点常量应报错。 */
struct neg_bitfield {
    unsigned int b : FLT_MAX;   /* 位域宽度必须是整型常量表达式 */
};

/* 违反约束「LDBL_MAX 是 long double 常量」：
 * 在 _Static_assert 风格的整型常量要求处使用浮点常量应报错
 * （C99 无 _Static_assert，这里用数组维度演示）。 */
int neg_array_dim[LDBL_MAX];   /* 数组维度必须是整型常量表达式 */

#endif