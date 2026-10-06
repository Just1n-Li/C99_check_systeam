/*
 * 测试 C99 6.4.4 Constants
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反 6.4.4 [2] 约束的代码应被编译器拒绝（编译报错）。
 *
 * 条款要点：
 *   [1] 语法：constant 由 integer-constant / floating-constant /
 *       enumeration-constant / character-constant 构成。
 *   [2] 约束：每个常量必须有类型，且其值必须在该类型可表示范围内。
 *   [3] 语义：每个常量都有类型，类型由其形式和值决定。
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：integer-constant 的四种形式 */
static void test_integer_constants(void)
{
    /* 十进制 */
    int dec = 42;
    /* 八进制（前导 0） */
    int oct = 052;          /* 八进制 52 == 十进制 42 */
    /* 十六进制（0x / 0X） */
    int hex = 0x2A;         /* 十六进制 2A == 十进制 42 */
    int hexU = 0X2Au;

    assert(dec == 42);
    assert(oct == 42);
    assert(hex == 42);
    assert(hexU == 42u);

    /* 后缀：u/U, l/L, ll/LL 及其组合 */
    unsigned int  ui  = 100u;
    long          l1  = 100l;
    long          l2  = 100L;
    unsigned long ul  = 100ul;
    unsigned long ul2 = 100UL;
    long long     ll  = 100ll;
    long long     ll2 = 100LL;
    unsigned long long ull = 100ull;

    assert(ui  == 100u);
    assert(l1  == 100l && l2 == 100L);
    assert(ul  == 100ul && ul2 == 100UL);
    assert(ll  == 100ll && ll2 == 100LL);
    assert(ull == 100ull);
}

/* [1] 语法：floating-constant 的形式 */
static void test_floating_constants(void)
{
    /* 小数形式 */
    double d1 = 3.14;
    double d2 = .5;         /* 省略整数部分 */
    double d3 = 5.;         /* 省略小数部分 */
    /* 指数形式 */
    double d4 = 1e10;
    double d5 = 1E10;
    double d6 = 1.5e-3;
    double d7 = .5e+2;
    /* 十六进制浮点常量（C99 新增） */
    double d8 = 0x1p4;      /* 1 * 2^4 == 16.0 */
    double d9 = 0x1.8p1;    /* 1.5 * 2^1 == 3.0 */
    /* 后缀 f/F, l/L */
    float  f1 = 1.5f;
    float  f2 = 1.5F;
    long double ld1 = 1.5l;
    long double ld2 = 1.5L;

    assert(d1 > 3.13 && d1 < 3.15);
    assert(d2 == 0.5);
    assert(d3 == 5.0);
    assert(d4 == 1e10);
    assert(d5 == 1e10);
    assert(d6 > 0.0014 && d6 < 0.0016);
    assert(d7 == 50.0);
    assert(d8 == 16.0);
    assert(d9 == 3.0);
    assert(f1 == 1.5f && f2 == 1.5f);
    assert(ld1 == 1.5l && ld2 == 1.5L);
}

/* [1] 语法：enumeration-constant */
enum Color { RED, GREEN = 5, BLUE };

static void test_enumeration_constants(void)
{
    /* 枚举常量是 constant 的一种形式 */
    int a = RED;
    int b = GREEN;
    int c = BLUE;

    assert(a == 0);
    assert(b == 5);
    assert(c == 6);   /* BLUE = GREEN + 1 */
}

/* [1] 语法：character-constant */
static void test_character_constants(void)
{
    /* 普通字符常量 */
    int c1 = 'A';
    /* 转义序列 */
    int c2 = '\n';
    int c3 = '\t';
    int c4 = '\\';
    int c5 = '\'';
    int c6 = '\0';
    /* 八进制转义 */
    int c7 = '\101';        /* 'A' */
    /* 十六进制转义 */
    int c8 = '\x41';        /* 'A' */
    /* 宽字符常量 */
    int wc = L'A';

    assert(c1 == 65);
    assert(c2 == 10);
    assert(c3 == 9);
    assert(c4 == 92);
    assert(c5 == 39);
    assert(c6 == 0);
    assert(c7 == 65);
    assert(c8 == 65);
    assert(wc == 65);
}

/* [3] 语义：常量的类型由其形式和值决定 */
static void test_constant_types(void)
{
    /* 无后缀十进制整数常量：int, long int, long long int 中第一个能表示的 */
    assert(sizeof(1) == sizeof(int));

    /* 无后缀八进制/十六进制：int, unsigned int, long, unsigned long,
       long long, unsigned long long 中第一个能表示的 */
    assert(sizeof(0x1) == sizeof(int));

    /* 带 u 后缀：unsigned int 起 */
    assert(sizeof(1u) == sizeof(unsigned int));

    /* 带 l 后缀：long int 起 */
    assert(sizeof(1l) == sizeof(long));

    /* 带 ll 后缀：long long int 起 */
    assert(sizeof(1ll) == sizeof(long long));

    /* 浮点常量无后缀为 double */
    assert(sizeof(1.0) == sizeof(double));
    /* 带 f 后缀为 float */
    assert(sizeof(1.0f) == sizeof(float));
    /* 带 l 后缀为 long double */
    assert(sizeof(1.0l) == sizeof(long double));

    /* 字符常量的类型为 int（C99 中普通字符常量类型是 int） */
    assert(sizeof('A') == sizeof(int));

    /* 枚举常量的类型为 int */
    assert(sizeof(RED) == sizeof(int));
}

/* [2] 约束：常量的值必须在其类型可表示范围内（合法示例） */
static void test_representable_range(void)
{
    /* 这些值都在各自类型范围内，合法 */
    int  i = 2147483647;            /* INT_MAX（在 32 位 int 上） */
    unsigned int u = 4294967295u;   /* UINT_MAX */
    long long ll = 9223372036854775807ll; /* LLONG_MAX */

    assert(i == INT_MAX);
    assert(u == UINT_MAX);
    assert(ll == LLONG_MAX);

    /* 浮点常量在 double 范围内 */
    double d = 1.7976931348623157e308;  /* 接近 DBL_MAX */
    assert(d > 0.0);
}

int main(void)
{
    test_integer_constants();
    test_floating_constants();
    test_enumeration_constants();
    test_character_constants();
    test_constant_types();
    test_representable_range();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/*
 * 违反 6.4.4 [2] 约束「常量的值必须在其类型可表示范围内」：
 * 整型常量 99999999999999999999999999 超出任何整型可表示范围，
 * gcc -std=c99 应报错（integer constant is too large for its type）。
 */
int too_big_int = 99999999999999999999999999;

/*
 * 违反 6.4.4 [2] 约束「常量的值必须在其类型可表示范围内」：
 * 浮点常量 1e9999 超出 double 可表示范围，
 * gcc -std=c99 应报错（floating constant exceeds range of 'double'）。
 */
double too_big_float = 1e9999;

/*
 * 违反 6.4.4 [1] 语法：constant 必须是 integer-constant /
 * floating-constant / enumeration-constant / character-constant 之一。
 * 下面既不是合法整数常量也不是合法浮点常量（缺少数字），
 * gcc -std=c99 应报错。
 */
int bad_constant = 0x;      /* 十六进制前缀后无数字 */

/*
 * 违反 6.4.4 [1] 语法：浮点常量指数部分缺少数字。
 * gcc -std=c99 应报错。
 */
double bad_float = 1e;      /* 指数标记后无数字 */

/*
 * 违反 6.4.4 [1] 语法：字符常量未闭合。
 * gcc -std=c99 应报错。
 */
int bad_char = 'A;           /* 缺少右单引号 */

#endif