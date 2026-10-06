/*
 * 测试 C99 7.21.4.2 —— strcmp 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int strcmp(const char *s1, const char *s2);  需要 <string.h>
 *   [2] 比较 s1 与 s2 所指向的字符串
 *   [3] 返回值 >0 / ==0 / <0 分别对应 s1 大于 / 等于 / 小于 s2
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型可用性：包含 <string.h> 后 strcmp 可被调用，且返回 int */
static int check_prototype(void)
{
    /* 通过函数指针验证原型签名：int (*)(const char *, const char *) */
    int (*fp)(const char *, const char *) = strcmp;
    return fp("a", "a");
}

/* [2] 比较两个字符串：相同内容返回 0 */
static void check_equal(void)
{
    const char *s1 = "hello";
    const char *s2 = "hello";
    assert(strcmp(s1, s2) == 0);          /* [3] 相等 -> 0 */
    assert(strcmp("", "") == 0);          /* 空串相等 */
    assert(strcmp("abc", "abc") == 0);
}

/* [3] s1 大于 s2 时返回 > 0 */
static void check_greater(void)
{
    assert(strcmp("b", "a") > 0);
    assert(strcmp("abc", "abb") > 0);     /* 逐字符比较，'c' > 'b' */
    assert(strcmp("ab", "a") > 0);        /* 前缀更短者更小 */
    assert(strcmp("b", "") > 0);
}

/* [3] s1 小于 s2 时返回 < 0 */
static void check_less(void)
{
    assert(strcmp("a", "b") < 0);
    assert(strcmp("abb", "abc") < 0);
    assert(strcmp("a", "ab") < 0);
    assert(strcmp("", "a") < 0);
}

/* [2][3] 比较基于字符的数值（unsigned char 语义），而非符号 */
static void check_ordering_consistency(void)
{
    /* 大小关系与返回值符号一致 */
    const char *a = "apple";
    const char *b = "banana";
    int r = strcmp(a, b);
    assert(r < 0);
    assert(strcmp(b, a) > 0);             /* 反对称性 */

    /* 自反性 */
    assert(strcmp(a, a) == 0);
}

/* [2] 参数为 const char *：可传入字符串字面量与 const 数组 */
static void check_const_args(void)
{
    const char s1[] = "xyz";
    const char s2[] = "xyw";
    assert(strcmp(s1, s2) > 0);
    assert(strcmp(s2, s1) < 0);
}

int main(void)
{
    assert(check_prototype() == 0);       /* [1] */
    check_equal();                        /* [2][3] */
    check_greater();                      /* [3] */
    check_less();                         /* [3] */
    check_ordering_consistency();         /* [2][3] */
    check_const_args();                   /* [2] */

    printf("C99 7.21.4.2 strcmp: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strcmp 的第一个参数类型为 const char *」：
 * 传入 int 实参，gcc -std=c99 应报 incompatible type / 参数类型不匹配错误。 */
#include <string.h>
int bad1(void)
{
    int x = 42;
    return strcmp(x, "abc");              /* 错误：int 不能转换为 const char * */
}

/* 违反约束「strcmp 的第二个参数类型为 const char *」：
 * 传入 double 实参，应报错。 */
int bad2(void)
{
    double d = 1.0;
    return strcmp("abc", d);              /* 错误：double 不能转换为 const char * */
}

/* 违反约束「strcmp 需要两个参数」：
 * 参数个数不匹配，应报错。 */
int bad3(void)
{
    return strcmp("abc");                 /* 错误：参数太少 */
}

/* 违反约束「strcmp 需要两个参数」：
 * 参数过多，应报错。 */
int bad4(void)
{
    return strcmp("a", "b", "c");         /* 错误：参数太多 */
}

/* 违反约束「strcmp 返回 int，不能作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
int bad5(void)
{
    strcmp("a", "b") = 0;                 /* 错误：非左值赋值 */
    return 0;
}

/* 违反约束「strcmp 返回 int，不能取地址后解引用赋值」：
 * 对非左值取地址应报错。 */
int bad6(void)
{
    int *p = &strcmp("a", "b");           /* 错误：不能对非左值取地址 */
    return *p;
}

#endif /* 负向测试结束 */