/*
 * 测试 C99 6.4.4.1 Integer constants
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反 6.4.4.1 的语法/约束，
 *             若单独取出编译，gcc -std=c99 应报错。
 *
 * 覆盖段落：[1] 语法、[2] 描述、[3] 各进制构成、[4] 求值基数、
 *           [5] 类型选择表、[6] 扩展整数类型/无类型。
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1][3] 十进制常量：以非零数字开头，仅含十进制数字 */
static int test_decimal(void)
{
    int a = 0;
    int b = 7;
    int c = 12345;
    int d = 2147483647;      /* INT_MAX 假设为 32 位 int */
    assert(a == 0);
    assert(b == 7);
    assert(c == 12345);
    assert(d == 2147483647);
    return 0;
}

/* [1][3][4] 八进制常量：前缀 0，后跟 0..7，按 8 进制求值 */
static int test_octal(void)
{
    int a = 0;        /* 八进制 0 */
    int b = 07;       /* 7 */
    int c = 010;      /* 8 */
    int d = 017;      /* 15 */
    int e = 0777;     /* 511 */
    assert(a == 0);
    assert(b == 7);
    assert(c == 8);
    assert(d == 15);
    assert(e == 511);
    return 0;
}

/* [1][3][4] 十六进制常量：前缀 0x / 0X，按 16 进制求值 */
static int test_hex(void)
{
    int a = 0x0;
    int b = 0xA;      /* 10 */
    int c = 0Xa;      /* 10，大小写前缀与数字均可 */
    int d = 0xff;     /* 255 */
    int e = 0XFF;     /* 255 */
    int f = 0x10;     /* 16 */
    assert(a == 0);
    assert(b == 10);
    assert(c == 10);
    assert(d == 255);
    assert(e == 255);
    assert(f == 16);
    return 0;
}

/* [1][5] 后缀：u/U, l/L, ll/LL 及其组合 */
static int test_suffix(void)
{
    unsigned int      u1 = 10u;
    unsigned int      u2 = 10U;
    long int          l1 = 10l;
    long int          l2 = 10L;
    unsigned long int ul1 = 10ul;
    unsigned long int ul2 = 10UL;
    unsigned long int ul3 = 10lu;
    unsigned long int ul4 = 10LU;
    long long int     ll1 = 10ll;
    long long int     ll2 = 10LL;
    unsigned long long int ull1 = 10ull;
    unsigned long long int ull2 = 10ULL;
    unsigned long long int ull3 = 10llu;
    unsigned long long int ull4 = 10LLU;

    assert(u1 == 10u && u2 == 10u);
    assert(l1 == 10L && l2 == 10L);
    assert(ul1 == 10UL && ul2 == 10UL && ul3 == 10UL && ul4 == 10UL);
    assert(ll1 == 10LL && ll2 == 10LL);
    assert(ull1 == 10ULL && ull2 == 10ULL && ull3 == 10ULL && ull4 == 10ULL);
    return 0;
}

/* [5] 类型选择：无后缀十进制常量在 int 装不下时升级为 long int */
static int test_type_selection(void)
{
    /* 十进制无后缀：int -> long int -> long long int */
    assert(sizeof(1) == sizeof(int));
    assert(sizeof(2147483647) == sizeof(int));   /* INT_MAX 仍为 int */

    /* 八进制/十六进制无后缀：int -> unsigned int -> long int -> ... */
    /* 0xFFFFFFFF 在 32 位 int 下不能表示为 int，应选 unsigned int */
    assert(sizeof(0xFFFFFFFF) == sizeof(unsigned int));
    assert(0xFFFFFFFFu == 4294967295u);

    /* 后缀 u：unsigned int -> unsigned long int -> unsigned long long int */
    assert(sizeof(1u) == sizeof(unsigned int));

    /* 后缀 l：long int -> long long int */
    assert(sizeof(1l) == sizeof(long int));

    /* 后缀 ul：unsigned long int -> unsigned long long int */
    assert(sizeof(1ul) == sizeof(unsigned long int));

    /* 后缀 ll：long long int */
    assert(sizeof(1ll) == sizeof(long long int));

    /* 后缀 ull：unsigned long long int */
    assert(sizeof(1ull) == sizeof(unsigned long long int));

    return 0;
}

/* [4] 词法上第一个数字是最高有效位 */
static int test_most_significant(void)
{
    assert(1234 == 1*1000 + 2*100 + 3*10 + 4);
    assert(01234 == 1*512 + 2*64 + 3*8 + 4);   /* 八进制 */
    assert(0x1234 == 1*4096 + 2*256 + 3*16 + 4); /* 十六进制 */
    return 0;
}

/* [2] 整数常量无小数点、无指数部分（与浮点常量区分） */
static int test_no_period_no_exponent(void)
{
    int i = 100;
    double d = 1e2;   /* 这是浮点常量，不是整数常量 */
    assert(i == 100);
    assert(d == 100.0);
    return 0;
}

/* [6] 扩展整数类型：若常量超出所有标准类型，可用扩展类型；
 *     本测试仅验证标准范围内常量类型正确，不强制扩展类型存在。 */
static int test_extended_note(void)
{
    /* 标准范围内常量类型必须落在 [5] 的列表中 */
    assert(sizeof(0) == sizeof(int));
    assert(sizeof(0u) == sizeof(unsigned int));
    return 0;
}

int main(void)
{
    test_decimal();
    test_octal();
    test_hex();
    test_suffix();
    test_type_selection();
    test_most_significant();
    test_no_period_no_exponent();
    test_extended_note();
    printf("C99 6.4.4.1 integer constants: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反 [1][3]：八进制常量只能含 0..7，出现 8 应报错
 * 期望：gcc -std=c99 报 "invalid digit '8' in octal constant" */
int bad_octal = 08;

/* 违反 [1][3]：八进制常量出现 9 应报错
 * 期望：gcc -std=c99 报 "invalid digit '9' in octal constant" */
int bad_octal2 = 019;

/* 违反 [1]：十六进制前缀 0x 后必须至少有一个十六进制数字
 * 期望：gcc -std=c99 报 "invalid suffix 'x' on integer constant" 或类似错误 */
int bad_hex = 0x;

/* 违反 [1]：十六进制常量中出现非十六进制字符 g
 * 期望：gcc -std=c99 报 "invalid suffix 'g' on integer constant" */
int bad_hex2 = 0xg;

/* 违反 [1]：十进制常量不能以 0 开头后跟非八进制数字（08 已在上方），
 * 这里用 0 后跟 8 的另一种写法，同样违反八进制数字约束 */
int bad_octal3 = 0008;

/* 违反 [1]：整数后缀只能是 u/U, l/L, ll/LL 及其组合，
 * 后缀 "uu" 非法
 * 期望：gcc -std=c99 报 "invalid suffix 'uu' on integer constant" */
int bad_suffix = 1uu;

/* 违反 [1]：后缀 "lll" 非法
 * 期望：gcc -std=c99 报 "invalid suffix 'lll' on integer constant" */
int bad_suffix2 = 1lll;

/* 违反 [1]：后缀 "ul" 合法，但 "lu" 也合法；这里用 "lul" 非法
 * 期望：gcc -std=c99 报 "invalid suffix 'lul' on integer constant" */
int bad_suffix3 = 1lul;

/* 违反 [1]：后缀 "uL" 合法，但 "uLL" 合法；这里用 "uLl" 非法（大小写混用 ll 不合法）
 * 期望：gcc -std=c99 报 "invalid suffix 'uLl' on integer constant" */
int bad_suffix4 = 1uLl;

/* 违反 [2]：整数常量不能含小数点
 * 期望：gcc -std=c99 报 "invalid suffix '.5' on integer constant" 或类似错误 */
int bad_period = 1.5;

/* 违反 [2]：整数常量不能含指数部分
 * 期望：gcc -std=c99 报 "invalid suffix 'e2' on integer constant" */
int bad_exponent = 1e2;

#endif