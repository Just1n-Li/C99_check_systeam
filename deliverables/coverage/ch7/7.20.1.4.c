/*
 * 测试目标：C99 7.20.1.4 —— strtol / strtoll / strtoul / strtoull
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，不影响本文件正常编译。
 *
 * 覆盖段落：[1] 原型  [2] 三段分解  [3] base 规则  [4] 主题序列定义
 *           [5] 转换与 endptr  [6] 其他 locale  [7] 空/非法主题序列
 *           [8] 返回值与 ERANGE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证签名与 restrict 限定兼容 */
static void test_prototypes(void)
{
    long int        (*p1)(const char *restrict, char **restrict, int) = strtol;
    long long int   (*p2)(const char *restrict, char **restrict, int) = strtoll;
    unsigned long int (*p3)(const char *restrict, char **restrict, int) = strtoul;
    unsigned long long int (*p4)(const char *restrict, char **restrict, int) = strtoull;
    assert(p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL);
}

/* [2] 三段分解：前导空白 + 主题序列 + 尾部未识别字符 */
static void test_three_parts(void)
{
    char *end;
    const char *s = "   \t\n  -1234abc";
    long v = strtol(s, &end, 10);
    assert(v == -1234L);
    /* end 指向尾部未识别字符 'a' */
    assert(strcmp(end, "abc") == 0);
}

/* [3] base == 0：按 6.4.4.1 整数常量规则（0x 十六进制、0 八进制、十进制） */
static void test_base_zero(void)
{
    char *end;
    assert(strtol("0x1F", &end, 0) == 31L);
    assert(*end == '\0');
    assert(strtol("017", &end, 0) == 15L);      /* 八进制 */
    assert(strtol("123", &end, 0) == 123L);     /* 十进制 */
    assert(strtol("-0x10", &end, 0) == -16L);   /* 可选符号 */
    assert(strtol("+42", &end, 0) == 42L);
}

/* [3] base 2..36：字母 a..z / A..Z 取值 10..35，仅允许值 < base 的字符 */
static void test_base_range(void)
{
    char *end;
    assert(strtol("101", &end, 2) == 5L);       /* 二进制 */
    assert(strtol("z", &end, 36) == 35L);       /* 36 进制最大字母 */
    assert(strtol("Z", &end, 36) == 35L);
    assert(strtol("ff", &end, 16) == 255L);
    assert(strtol("FF", &end, 16) == 255L);
    /* base 16 时可选 0x / 0X 前缀（在符号之后） */
    assert(strtol("0x1f", &end, 16) == 31L);
    assert(strtol("0X1F", &end, 16) == 31L);
    assert(strtol("-0x10", &end, 16) == -16L);
    /* 值 >= base 的字符不被接受，主题序列在此截断 */
    assert(strtol("2", &end, 2) == 0L);         /* '2' 在 base 2 下非法 */
    assert(end != NULL && *end == '2');         /* 无转换，endptr = nptr */
}

/* [4] 主题序列 = 从首个非空白字符开始、符合期望形式的最长初始子序列 */
static void test_subject_sequence(void)
{
    char *end;
    /* 最长初始子序列 "123"，遇到 'x' 停止 */
    assert(strtol("123x456", &end, 10) == 123L);
    assert(strcmp(end, "x456") == 0);
    /* 首个非空白字符是符号或允许的字母/数字之外 -> 主题序列为空 */
    assert(strtol("abc", &end, 10) == 0L);
    assert(end != NULL && strcmp(end, "abc") == 0);
}

/* [5] base 0 时按 6.4.4.1 解释；base 2..36 时按 base 解释；
 *     负号 -> 结果取负；endptr 非空时写入指向尾部字符串的指针 */
static void test_conversion_and_endptr(void)
{
    char *end;
    assert(strtol("-0", &end, 0) == 0L);
    assert(strtol("-7", &end, 10) == -7L);
    assert(strtol("+7", &end, 10) == 7L);
    /* endptr 为 NULL 时不得解引用 */
    assert(strtol("99", NULL, 10) == 99L);
    /* endptr 指向终止空字符 */
    assert(strtol("99", &end, 10) == 99L && *end == '\0');
}

/* [6] 其他 locale 可能接受额外的主题序列形式；C locale 下行为确定。
 *     这里只验证 C locale 的确定行为（不依赖具体 locale 实现）。 */
static void test_locale_c(void)
{
    char *end;
    assert(strtol("42", &end, 10) == 42L);
    assert(*end == '\0');
}

/* [7] 主题序列为空或形式不符：不执行转换，endptr 存入 nptr */
static void test_no_conversion(void)
{
    char *end;
    const char *s1 = "";
    const char *s2 = "   ";
    const char *s3 = "xyz";
    const char *s4 = "+";      /* 只有符号，无数字 */

    assert(strtol(s1, &end, 10) == 0L && end == s1);
    assert(strtol(s2, &end, 10) == 0L && end == s2);
    assert(strtol(s3, &end, 10) == 0L && end == s3);
    assert(strtol(s4, &end, 10) == 0L && end == s4);
    /* endptr 为 NULL 时同样不转换，返回 0 */
    assert(strtol("xyz", NULL, 10) == 0L);
}

/* [8] 返回值：正常值 / 无转换返回 0 / 越界返回极值并置 errno = ERANGE */
static void test_returns_and_erange(void)
{
    char *end;

    /* 正常转换 */
    assert(strtol("123", &end, 10) == 123L);

    /* 无转换 -> 0 */
    assert(strtol("abc", &end, 10) == 0L);

    /* strtol 上溢 -> LONG_MAX, errno = ERANGE */
    errno = 0;
    assert(strtol("99999999999999999999999999", &end, 10) == LONG_MAX);
    assert(errno == ERANGE);

    /* strtol 下溢 -> LONG_MIN, errno = ERANGE */
    errno = 0;
    assert(strtol("-99999999999999999999999999", &end, 10) == LONG_MIN);
    assert(errno == ERANGE);

    /* strtoul 上溢 -> ULONG_MAX, errno = ERANGE */
    errno = 0;
    assert(strtoul("99999999999999999999999999", &end, 10) == ULONG_MAX);
    assert(errno == ERANGE);

    /* strtoul 接受负号：结果按无符号取模（-1 -> ULONG_MAX），不置 ERANGE */
    errno = 0;
    assert(strtoul("-1", &end, 10) == ULONG_MAX);
    assert(errno == 0);

    /* strtoll 上溢 -> LLONG_MAX, errno = ERANGE */
    errno = 0;
    assert(strtoll("99999999999999999999999999999999", &end, 10) == LLONG_MAX);
    assert(errno == ERANGE);

    /* strtoll 下溢 -> LLONG_MIN, errno = ERANGE */
    errno = 0;
    assert(strtoll("-99999999999999999999999999999999", &end, 10) == LLONG_MIN);
    assert(errno == ERANGE);

    /* strtoull 上溢 -> ULLONG_MAX, errno = ERANGE */
    errno = 0;
    assert(strtoull("99999999999999999999999999999999", &end, 10) == ULLONG_MAX);
    assert(errno == ERANGE);

    /* strtoull 接受负号：-1 -> ULLONG_MAX，不置 ERANGE */
    errno = 0;
    assert(strtoull("-1", &end, 10) == ULLONG_MAX);
    assert(errno == 0);
}

int main(void)
{
    test_prototypes();
    test_three_parts();
    test_base_zero();
    test_base_range();
    test_subject_sequence();
    test_conversion_and_endptr();
    test_locale_c();
    test_no_conversion();
    test_returns_and_erange();

    printf("C99 7.20.1.4 strtol/strtoll/strtoul/strtoull: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数原型参数类型必须匹配」：
 * strtol 第 3 参数为 int，传入指针类型不兼容，gcc -std=c99 应报错
 * （incompatible type for argument 3 / passing argument 3 ... makes pointer from integer）。 */
void bad_arg_type(void)
{
    char *end;
    long v = strtol("10", &end, (int *)0);   /* 第 3 参数应为 int，不是 int* */
    (void)v;
}

/* 违反约束「实参个数必须与原型一致」：
 * strtol 需要 3 个实参，只给 2 个，gcc -std=c99 应报错
 * （too few arguments to function 'strtol'）。 */
void bad_arg_count(void)
{
    long v = strtol("10", (char **)0);       /* 缺少 base 实参 */
    (void)v;
}

/* 违反约束「实参个数必须与原型一致」：
 * strtoull 需要 3 个实参，给 4 个，gcc -std=c99 应报错
 * （too many arguments to function 'strtoull'）。 */
void bad_arg_count2(void)
{
    unsigned long long v = strtoull("10", (char **)0, 10, 0);
    (void)v;
}

/* 违反约束「返回值类型不可隐式转换为不兼容类型」：
 * strtol 返回 long int，赋给结构体类型对象，gcc -std=c99 应报错
 * （incompatible types when assigning to type 'struct S' from type 'long int'）。 */
struct S { int x; };
void bad_return_assign(void)
{
    struct S s;
    s = strtol("10", (char **)0, 10);        /* long -> struct S 不兼容 */
    (void)s;
}

/* 违反约束「const 限定：nptr 指向的字符不可修改」：
 * strtol 的 nptr 为 const char *，通过它写字符违反 const 约束，
 * gcc -std=c99 应报错（assignment of read-only location）。 */
void bad_const_write(void)
{
    char buf[] = "10";
    const char *p = buf;
    *p = '2';                                /* 通过 const 指针写 -> 违反约束 */
    (void)strtol(p, (char **)0, 10);
}

/* 违反约束「restrict 限定：endptr 指向的对象类型必须为 char *」：
 * 传入 int* 作为 endptr，与 char ** 不兼容，gcc -std=c99 应报错
 * （incompatible pointer type）。 */
void bad_endptr_type(void)
{
    int *ip = 0;
    long v = strtol("10", &ip, 10);          /* int** 与 char** 不兼容 */
    (void)v;
}

#endif /* 负向测试结束 */