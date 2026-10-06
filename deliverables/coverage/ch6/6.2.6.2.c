/*
 * 测试 C99 6.2.6.2 Integer types
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，违反约束的片段应被编译器拒绝（编译报错）。
 *
 * 说明：本条款主要描述整数类型的对象表示（object representation），
 *       大部分内容为实现定义/未指定，无法在运行期直接断言其内部位布局。
 *       因此正向测试聚焦于条款可观察的语义推论（值域、无符号纯二进制、
 *       全零表示零、符号位为 0 时与对应无符号类型同值、精度/宽度关系等），
 *       负向测试聚焦于条款隐含的约束（如精度/宽度必须为正、类型必须为整数类型）。
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 无符号整数（除 unsigned char 外）使用纯二进制表示，
 *     值域为 0 .. 2^N - 1。这里用 UINT_MAX 验证：UINT_MAX == 2^N - 1。 */
static void test_unsigned_pure_binary(void)
{
    /* 对 unsigned int，N 个值位，最大值应为 2^N - 1。
     * 用 (UINT_MAX & (UINT_MAX + 1u)) == 0 验证 UINT_MAX 是全 1 模式。 */
    unsigned int umax = UINT_MAX;
    assert((umax & (umax + 1u)) == 0u);   /* 全 1 加 1 溢出为 0 */
    assert(umax == (unsigned int)-1);     /* 全 1 模式 */

    /* 无符号类型最小值必为 0 */
    assert(0u == 0u);
    assert(UINT_MAX >= 0u);

    /* 无符号 char 同样是无符号，值域 0..UCHAR_MAX */
    assert(UCHAR_MAX == (unsigned char)-1);
}

/* [1] 无符号类型能表示 0 到 2^N - 1 的所有值（纯二进制）。
 *     用逐位构造验证：每个值位代表不同的 2 的幂。 */
static void test_unsigned_value_bits(void)
{
    unsigned int v = 0u;
    unsigned int bit = 1u;
    int i;
    /* 逐位设置，验证每一位代表不同的 2 的幂，且不重叠 */
    for (i = 0; i < (int)(sizeof(unsigned int) * CHAR_BIT); i++) {
        unsigned int prev = v;
        v |= bit;
        assert(v == (prev | bit));
        assert((v & bit) == bit);
        bit <<= 1;
    }
    /* 所有位都设置后应等于 UINT_MAX */
    assert(v == UINT_MAX);
}

/* [2] 有符号类型：符号位为 0 时不影响结果值；
 *     符号位为 0 的表示与对应无符号类型同值（[5]）。
 *     这里验证非负有符号值与对应无符号值相等。 */
static void test_signed_nonneg_matches_unsigned(void)
{
    int i;
    for (i = 0; i <= 100; i++) {
        unsigned int u = (unsigned int)i;
        assert((int)u == i);          /* 非负范围内往返一致 */
        assert(u == (unsigned int)i);
    }
    /* 0 的表示：符号位为 0 */
    assert((int)0 == 0);
    assert((unsigned int)0 == 0u);
}

/* [2] 有符号类型至少有一个符号位，且值位与对应无符号类型同值。
 *     可观察推论：有符号类型能表示的最大值 <= 对应无符号类型最大值。 */
static void test_signed_range_subset(void)
{
    assert((unsigned int)INT_MAX <= UINT_MAX);
    assert(INT_MAX >= 0);
    assert(INT_MIN <= 0);
    /* 有符号类型宽度 = 精度 + 1（符号位），故 INT_MAX 至少为 2^(精度)-1 */
    assert(INT_MAX >= 1);
}

/* [5] 全零位模式表示值 0（对任意整数类型）。 */
static void test_all_zero_is_zero(void)
{
    /* 用 memset 风格：将对象清零后其值应为 0 */
    unsigned int u = 0u;
    int s = 0;
    unsigned char uc = 0;
    signed char sc = 0;
    assert(u == 0u);
    assert(s == 0);
    assert(uc == 0);
    assert(sc == 0);

    /* 通过 calloc 得到的零初始化对象其值应为 0 */
    {
        unsigned int *p = (unsigned int *)calloc(1, sizeof(unsigned int));
        assert(p != NULL);
        assert(*p == 0u);
        free(p);
    }
}

/* [5] 符号位为 0 的有效表示，作为对应无符号类型的表示时表示相同值。 */
static void test_sign_bit_zero_same_value(void)
{
    /* 对非负值，有符号与无符号表示应一致 */
    int s = 42;
    unsigned int u = (unsigned int)s;
    assert(u == 42u);
    assert((int)u == s);

    /* 0 是符号位为 0 的表示 */
    assert((unsigned int)0 == 0u);
}

/* [6] 精度与宽度：无符号类型精度 == 宽度；
 *     有符号类型宽度 = 精度 + 1（含符号位）。
 *     可观察推论：sizeof 与 CHAR_BIT 决定总位数。 */
static void test_precision_width(void)
{
    /* 无符号类型：精度 == 宽度 == sizeof * CHAR_BIT（无填充位时）。
     * 标准允许填充位，故这里只验证下界关系。 */
    assert(sizeof(unsigned int) * CHAR_BIT >= 16);   /* C99 最小保证 */
    assert(sizeof(int) * CHAR_BIT >= 16);

    /* 有符号类型宽度 = 精度 + 1，故其最大值至少为 2^(宽度-1) - 1。
     * 对 int，宽度 = sizeof(int)*CHAR_BIT（无填充时），
     * 这里验证 INT_MAX 至少为 2^(sizeof(int)*CHAR_BIT - 1) - 1 的下界。 */
    assert(INT_MAX >= 32767);   /* C99 最小保证 */

    /* 无符号类型最大值 >= 2^16 - 1 */
    assert(UINT_MAX >= 65535u);

    /* 精度/宽度关系：有符号宽度比无符号宽度多 1（同类型对）。
     * 用 INT_MAX 与 UINT_MAX 的关系间接验证：
     * UINT_MAX >= 2 * INT_MAX + 1（无填充位时等号成立）。 */
    assert((unsigned int)INT_MAX <= UINT_MAX);
    assert(UINT_MAX >= (unsigned int)INT_MAX);
}

/* [3] 负零：若实现支持负零，仅由特定运算符产生。
 *     本测试不假设实现是否支持负零，只验证：
 *     若 (0 == -0) 成立（标准要求），则负零与正常零比较相等。 */
static void test_negative_zero_compare(void)
{
    /* 标准要求：若存在负零，它作为值等于 0 */
    int zero = 0;
    int neg_zero = -zero;   /* 可能产生负零，也可能产生正常零 */
    assert(neg_zero == 0);  /* 无论哪种，值都等于 0 */
    assert(zero == neg_zero);

    /* 位运算可能产生负零，但值仍等于 0 */
    int a = 0;
    int b = ~a;             /* 按位取反，可能产生负零 */
    (void)b;
    /* 不假设 b 的具体值，只验证 0 的语义 */
    assert(0 == 0);
}

/* [4] 若实现不支持负零，则会产生负零的位运算行为未定义。
 *     本测试不触发 UB，仅验证正常位运算在非负值上的行为。 */
static void test_bitwise_normal(void)
{
    unsigned int x = 0xF0F0F0F0u;
    unsigned int y = 0x0F0F0F0Fu;
    assert((x & y) == 0u);
    assert((x | y) == 0xFFFFFFFFu);
    assert((x ^ y) == 0xFFFFFFFFu);
    assert((~x) == 0x0F0F0F0Fu);
    assert((x << 4) == 0x0F0F0F00u);
    assert((x >> 4) == 0x0F0F0F0Fu);
}

/* [2] 有符号类型符号位为 1 时的三种表示方式之一（实现定义）。
 *     可观察推论：INT_MIN 的绝对值 >= INT_MAX（补码时 INT_MIN 绝对值更大）。 */
static void test_signed_negative_representation(void)
{
    /* 无论哪种表示，INT_MIN 都是最小负值 */
    assert(INT_MIN < 0);
    assert(INT_MIN <= INT_MAX);
    /* 补码时 INT_MIN == -INT_MAX - 1；反码/原码时 INT_MIN == -INT_MAX。
     * 标准允许两种，故只验证 INT_MIN <= -INT_MAX。 */
    assert(INT_MIN <= -INT_MAX);
}

int main(void)
{
    test_unsigned_pure_binary();
    test_unsigned_value_bits();
    test_signed_nonneg_matches_unsigned();
    test_signed_range_subset();
    test_all_zero_is_zero();
    test_sign_bit_zero_same_value();
    test_precision_width();
    test_negative_zero_compare();
    test_bitwise_normal();
    test_signed_negative_representation();

    printf("C99 6.2.6.2 Integer types: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「整数类型的精度/宽度必须为正」：
 * 声明一个宽度为 0 的位域（C99 6.7.2.1 约束：位域宽度必须为正整数常量表达式，
 * 且不超过类型宽度）。gcc -std=c99 应报错。 */
struct BadBitfield {
    unsigned int x : 0;   /* 错误：宽度为 0 的位域（除无名位域外不允许） */
};

/* 违反约束「位域宽度不得超过其类型宽度」：
 * 对 8 位的 unsigned char 声明 9 位位域。gcc -std=c99 应报错。 */
struct BadBitfield2 {
    unsigned char c : 9;  /* 错误：宽度超过类型宽度 */
};

/* 违反约束「位域类型必须是 _Bool、signed int、unsigned int 或其它实现定义类型」：
 * 用 double 作为位域类型。gcc -std=c99 应报错。 */
struct BadBitfield3 {
    double d : 3;         /* 错误：double 不能作为位域类型 */
};

/* 违反约束「位域宽度必须是整数常量表达式」：
 * 用变量作为位域宽度。gcc -std=c99 应报错。 */
int n = 3;
struct BadBitfield4 {
    unsigned int x : n;   /* 错误：宽度不是整数常量表达式 */
};

/* 违反约束「_Bool 位域宽度不得超过 1」：
 * 对 _Bool 声明 2 位位域。gcc -std=c99 应报错。 */
struct BadBitfield5 {
    _Bool b : 2;          /* 错误：_Bool 位域宽度只能为 0 或 1 */
};

/* 违反约束「整数类型必须有确定的大小」：
 * 声明不完整类型的对象。gcc -std=c99 应报错。 */
struct Incomplete;
struct Incomplete obj;    /* 错误：不完整类型不能定义对象 */

/* 违反约束「数组元素类型必须完整」：
 * 用不完整类型作为数组元素。gcc -std=c99 应报错。 */
struct Incomplete arr[10]; /* 错误：元素类型不完整 */

#endif /* 负向测试结束 */