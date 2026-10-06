/*
 * 测试 C99 7.8.2.3 —— strtoimax / strtoumax
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器应报错。
 *
 * 覆盖段落：
 *   [1] 函数原型（<inttypes.h>，restrict 限定，返回 intmax_t / uintmax_t）
 *   [2] 语义等价于 strtol/strtoll/strtoul/strtoull，但转换为 intmax_t / uintmax_t
 *   [3] 返回值：成功返回转换值；无法转换返回 0；越界返回 INTMAX_MAX/INTMAX_MIN/UINTMAX_MAX
 *       并置 errno = ERANGE
 */

#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与声明一致（restrict 限定不影响兼容性） */
static intmax_t  (*p_strtoimax)(const char * restrict, char ** restrict, int) = strtoimax;
static uintmax_t (*p_strtoumax)(const char * restrict, char ** restrict, int) = strtoumax;

static void test_basic_conversion(void)
{
    /* [2] 基本十进制转换，等价于 strtol 但结果为 intmax_t */
    char *end;
    intmax_t v;

    errno = 0;
    v = strtoimax("12345", &end, 10);
    assert(v == 12345);
    assert(*end == '\0');
    assert(errno == 0);

    /* [2] 带前导空白与符号 */
    errno = 0;
    v = strtoimax("   -6789xyz", &end, 10);
    assert(v == -6789);
    assert(strcmp(end, "xyz") == 0);
    assert(errno == 0);

    /* [2] 十六进制（base 16，允许 0x 前缀） */
    errno = 0;
    v = strtoimax("0xFF", &end, 16);
    assert(v == 255);
    assert(*end == '\0');

    /* [2] base 0 自动识别前缀 */
    errno = 0;
    v = strtoimax("0x10", &end, 0);
    assert(v == 16);
    assert(*end == '\0');

    errno = 0;
    v = strtoimax("010", &end, 0);
    assert(v == 8);
    assert(*end == '\0');

    /* [2] 无符号版本 */
    errno = 0;
    uintmax_t u = strtoumax("4294967295", &end, 10);
    assert(u == UINTMAX_C(4294967295));
    assert(*end == '\0');
    assert(errno == 0);

    /* [2] 无符号版本接受负号（按模转换，等价于 strtoul） */
    errno = 0;
    u = strtoumax("-1", &end, 10);
    assert(u == UINTMAX_MAX);
    assert(*end == '\0');
    assert(errno == 0); /* 按 strtoul 语义，-1 是合法转换，不置 ERANGE */
}

static void test_no_conversion(void)
{
    /* [3] 无法转换时返回 0 */
    char *end;
    intmax_t v;
    uintmax_t u;

    errno = 0;
    v = strtoimax("abc", &end, 10);
    assert(v == 0);
    assert(end != NULL && strcmp(end, "abc") == 0);
    assert(errno == 0);

    errno = 0;
    u = strtoumax("", &end, 10);
    assert(u == 0);
    assert(end != NULL && *end == '\0');
    assert(errno == 0);

    /* endptr 可以为 NULL */
    errno = 0;
    v = strtoimax("42", NULL, 10);
    assert(v == 42);
    assert(errno == 0);
}

static void test_range_errors(void)
{
    /* [3] 越界：正溢出 -> INTMAX_MAX，errno = ERANGE */
    char *end;
    intmax_t v;
    uintmax_t u;

    errno = 0;
    v = strtoimax("99999999999999999999999999999999999999", &end, 10);
    assert(v == INTMAX_MAX);
    assert(errno == ERANGE);

    /* [3] 越界：负溢出 -> INTMAX_MIN，errno = ERANGE */
    errno = 0;
    v = strtoimax("-99999999999999999999999999999999999999", &end, 10);
    assert(v == INTMAX_MIN);
    assert(errno == ERANGE);

    /* [3] 无符号越界 -> UINTMAX_MAX，errno = ERANGE */
    errno = 0;
    u = strtoumax("99999999999999999999999999999999999999999999", &end, 10);
    assert(u == UINTMAX_MAX);
    assert(errno == ERANGE);

    /* [3] 无符号负溢出 -> UINTMAX_MAX，errno = ERANGE */
    errno = 0;
    u = strtoumax("-99999999999999999999999999999999999999999999", &end, 10);
    assert(u == UINTMAX_MAX);
    assert(errno == ERANGE);
}

static void test_boundary_values(void)
{
    /* [3] 恰好等于边界值不应置 ERANGE */
    char buf[64];
    char *end;
    intmax_t v;
    uintmax_t u;

    /* INTMAX_MAX 的十进制表示 */
    snprintf(buf, sizeof buf, "%jd", (intmax_t)INTMAX_MAX);
    errno = 0;
    v = strtoimax(buf, &end, 10);
    assert(v == INTMAX_MAX);
    assert(*end == '\0');
    assert(errno == 0);

    /* INTMAX_MIN 的十进制表示 */
    snprintf(buf, sizeof buf, "%jd", (intmax_t)INTMAX_MIN);
    errno = 0;
    v = strtoimax(buf, &end, 10);
    assert(v == INTMAX_MIN);
    assert(*end == '\0');
    assert(errno == 0);

    /* UINTMAX_MAX 的十进制表示 */
    snprintf(buf, sizeof buf, "%ju", (uintmax_t)UINTMAX_MAX);
    errno = 0;
    u = strtoumax(buf, &end, 10);
    assert(u == UINTMAX_MAX);
    assert(*end == '\0');
    assert(errno == 0);
}

static void test_equivalence_with_strtol(void)
{
    /* [2] 与 strtol 等价性（在 long 范围内） */
    const char *samples[] = { "0", "1", "-1", "123456", "-98765", "0x7f", "0777" };
    int bases[] = { 10, 10, 10, 10, 10, 16, 8 };
    size_t i;

    for (i = 0; i < sizeof samples / sizeof samples[0]; ++i) {
        char *e1, *e2;
        long l;
        intmax_t m;

        errno = 0;
        l = strtol(samples[i], &e1, bases[i]);
        errno = 0;
        m = strtoimax(samples[i], &e2, bases[i]);

        assert((intmax_t)l == m);
        assert(strcmp(e1, e2) == 0);
    }
}

int main(void)
{
    /* [1] 原型指针非空 */
    assert(p_strtoimax == strtoimax);
    assert(p_strtoumax == strtoumax);

    test_basic_conversion();
    test_no_conversion();
    test_range_errors();
    test_boundary_values();
    test_equivalence_with_strtol();

    printf("All positive tests for C99 7.8.2.3 passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strtoimax 的第一个参数类型为 const char *」：
 * 传入 int 指针，gcc -std=c99 应报 incompatible pointer type 警告/错误。 */
{
    int x = 0;
    intmax_t v = strtoimax(&x, NULL, 10); /* 期望：类型不兼容 */
    (void)v;
}

/* 违反约束「strtoimax 的第二个参数类型为 char **」：
 * 传入 int **，gcc -std=c99 应报 incompatible pointer type。 */
{
    int *p = 0;
    intmax_t v = strtoimax("1", &p, 10); /* 期望：类型不兼容 */
    (void)v;
}

/* 违反约束「strtoimax 的第三个参数类型为 int」：
 * 传入指针，gcc -std=c99 应报 incompatible type for argument。 */
{
    intmax_t v = strtoimax("1", NULL, (int *)0); /* 期望：类型不兼容 */
    (void)v;
}

/* 违反约束「strtoimax 返回 intmax_t，不能赋给结构体」：
 * 期望：类型不兼容。 */
{
    struct S { int a; } s;
    s = strtoimax("1", NULL, 10); /* 期望：类型不兼容 */
}

/* 违反约束「strtoumax 返回 uintmax_t，不能赋给指针」：
 * 期望：类型不兼容。 */
{
    char *p;
    p = strtoumax("1", NULL, 10); /* 期望：类型不兼容 */
}

/* 违反约束「调用 strtoimax 时实参个数必须为 3」：
 * 期望：too few arguments to function 'strtoimax'。 */
{
    intmax_t v = strtoimax("1", NULL); /* 期望：参数个数错误 */
    (void)v;
}

/* 违反约束「调用 strtoumax 时实参个数必须为 3」：
 * 期望：too many arguments to function 'strtoumax'。 */
{
    uintmax_t u = strtoumax("1", NULL, 10, 0); /* 期望：参数个数错误 */
    (void)u;
}

/* 违反约束「strtoimax 的返回类型为 intmax_t，不能作为函数指针赋给不兼容类型」：
 * 期望：类型不兼容。 */
{
    void (*fp)(void) = strtoimax; /* 期望：类型不兼容 */
    (void)fp;
}

#endif /* 负向测试结束 */