/*
 * 测试 C99 7.20.1.2 —— atoi / atol / atoll 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束的片段应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：
 *   [1] 原型声明（<stdlib.h> 中 int atoi(const char*); long atol(const char*);
 *       long long atoll(const char*);）
 *   [2] 语义：转换 nptr 指向字符串的初始部分为 int / long / long long，
 *       除错误行为外等价于 strtol(nptr, NULL, 10) / strtoll(nptr, NULL, 10)。
 *   [3] 返回值：返回转换后的值。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型可用性：包含 <stdlib.h> 后三个函数均可调用，且返回类型正确。
 *     用函数指针类型检查签名（const char * 参数）。 */
static int  (*p_atoi)(const char *)  = atoi;
static long (*p_atol)(const char *)  = atol;
static long long (*p_atoll)(const char *) = atoll;

static void test_prototypes(void)
{
    /* [1] 通过函数指针调用，验证原型与返回类型 */
    assert(p_atoi  != NULL);
    assert(p_atol  != NULL);
    assert(p_atoll != NULL);

    /* 返回类型大小/符号检查 */
    assert(sizeof(atoi("0"))  == sizeof(int));
    assert(sizeof(atol("0"))  == sizeof(long));
    assert(sizeof(atoll("0")) == sizeof(long long));
}

static void test_basic_conversion(void)
{
    /* [2][3] 基本十进制转换 */
    assert(atoi("0") == 0);
    assert(atoi("123") == 123);
    assert(atoi("-123") == -123);
    assert(atoi("+456") == 456);

    assert(atol("0") == 0L);
    assert(atol("123456") == 123456L);
    assert(atol("-123456") == -123456L);

    assert(atoll("0") == 0LL);
    assert(atoll("1234567890123") == 1234567890123LL);
    assert(atoll("-1234567890123") == -1234567890123LL);
}

static void test_initial_portion(void)
{
    /* [2] 只转换“初始部分”：遇到非数字字符停止 */
    assert(atoi("123abc") == 123);
    assert(atoi("42xyz") == 42);
    assert(atoi("  789") == 789);      /* 前导空白由 strtol 处理 */
    assert(atoi("\t\n 55") == 55);
    assert(atoi("7 8 9") == 7);        /* 空格终止 */

    assert(atol("1000rest") == 1000L);
    assert(atoll("999999999999zzz") == 999999999999LL);
}

static void test_equivalence_with_strtol(void)
{
    /* [2] 除错误行为外，等价于 strtol(nptr, NULL, 10) / strtoll(nptr, NULL, 10) */
    const char *samples[] = {
        "0", "1", "-1", "+1", "12345", "-98765",
        "  42", "\t\n 7", "123abc", "abc", "", "   ",
        "2147483647", "-2147483648", "9999999999999999999"
    };
    size_t i;
    for (i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        const char *s = samples[i];
        /* 注意：错误行为（如溢出）不要求一致，这里只比较非溢出样本 */
        long  l  = strtol(s, (char **)NULL, 10);
        long long ll = strtoll(s, (char **)NULL, 10);
        /* 对未溢出的样本，atoi/atol/atoll 应与 strtol/strtoll 一致 */
        if (l >= INT_MIN && l <= INT_MAX) {
            assert(atoi(s) == (int)l);
        }
        assert(atol(s) == l);
        assert(atoll(s) == ll);
    }
}

static void test_no_conversion(void)
{
    /* [2][3] 无法转换时返回 0（错误行为，标准未规定，但常见实现返回 0；
     * 这里只验证“初始部分为空”的常见情形，不依赖具体错误值，
     * 因此仅检查不崩溃并打印，不做强断言）。 */
    (void)atoi("abc");
    (void)atol("abc");
    (void)atoll("abc");
    (void)atoi("");
    (void)atol("");
    (void)atoll("");
    printf("no-conversion results: atoi=%d atol=%ld atoll=%lld\n",
           atoi("abc"), atol("abc"), atoll("abc"));
}

static void test_limits(void)
{
    /* [2][3] 边界值（在可表示范围内） */
    assert(atoi("2147483647") == INT_MAX);
    assert(atoi("-2147483648") == INT_MIN);

    /* long / long long 的边界由实现决定，这里用字符串比较验证一致性 */
    {
        char buf[64];
        sprintf(buf, "%ld", LONG_MAX);
        assert(atol(buf) == LONG_MAX);
        sprintf(buf, "%ld", LONG_MIN);
        assert(atol(buf) == LONG_MIN);
    }
    {
        char buf[64];
        sprintf(buf, "%lld", LLONG_MAX);
        assert(atoll(buf) == LLONG_MAX);
        sprintf(buf, "%lld", LLONG_MIN);
        assert(atoll(buf) == LLONG_MIN);
    }
}

int main(void)
{
    test_prototypes();
    test_basic_conversion();
    test_initial_portion();
    test_equivalence_with_strtol();
    test_no_conversion();
    test_limits();

    printf("C99 7.20.1.2 atoi/atol/atoll: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数参数类型必须匹配」：atoi 的参数是 const char *，
 * 传入 int 应报错（gcc -std=c99 报 incompatible type / passing argument）。 */
int bad1 = atoi(123);

/* 违反约束「函数参数个数必须匹配」：atoi 只接受 1 个参数。 */
int bad2 = atoi("123", 10);

/* 违反约束「函数参数个数必须匹配」：atol 只接受 1 个参数。 */
long bad3 = atol("123", 0);

/* 违反约束「函数参数个数必须匹配」：atoll 只接受 1 个参数。 */
long long bad4 = atoll("123", 0);

/* 违反约束「函数必须已声明」：未包含 <stdlib.h> 且未声明 atoi 时调用，
 * 在 C99 中隐式函数声明是约束违反（gcc -std=c99 -Werror=implicit-function-declaration 报错）。 */
/* 注意：本文件已包含 <stdlib.h>，此片段仅示意；实际测试需单独文件。 */
/* int bad5 = undeclared_atoi("1"); */

/* 违反约束「返回类型不可赋值给不兼容类型」：atoi 返回 int，
 * 不能直接初始化结构体类型。 */
struct S { int x; };
struct S bad6 = atoi("1");

/* 违反约束「函数返回类型不可作为数组初始化」： */
int bad7[3] = atoi("1");

/* 违反约束「函数调用结果不是左值」：atoi 的返回值不能取地址。 */
int *bad8 = &atoi("1");

/* 违反约束「函数调用结果不是左值」：atoi 的返回值不能赋值。 */
void bad9(void) { atoi("1") = 5; }

#endif /* 负向测试结束 */