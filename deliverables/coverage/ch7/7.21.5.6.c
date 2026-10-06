/*
 * 测试 C99 7.21.5.6 —— strspn 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 条款要点：
 *   [1] 原型：size_t strspn(const char *s1, const char *s2);
 *   [2] 计算 s1 指向字符串中，由 s2 指向字符串中字符组成的最大初始段的长度。
 *   [3] 返回该段的长度。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：返回类型为 size_t，参数为两个 const char * */
static size_t (*fp)(const char *, const char *) = strspn;

int main(void)
{
    /* [1] 原型可用性：函数指针赋值成功，说明签名匹配 */
    assert(fp != NULL);

    /* [2][3] 基本语义：s1 的初始段全部由 s2 中字符组成 */
    {
        const char *s1 = "abcde123";
        const char *s2 = "abc";
        /* 初始段 "abc" 全部来自 s2，长度 3 */
        assert(strspn(s1, s2) == 3);
    }

    /* [2][3] s1 首字符不在 s2 中：初始段长度为 0 */
    {
        const char *s1 = "xyzabc";
        const char *s2 = "abc";
        assert(strspn(s1, s2) == 0);
    }

    /* [2][3] s1 全部字符都在 s2 中：返回整个字符串长度 */
    {
        const char *s1 = "aaaa";
        const char *s2 = "a";
        assert(strspn(s1, s2) == 4);
    }

    /* [2][3] s2 为空字符串：没有任何字符可匹配，初始段长度为 0 */
    {
        const char *s1 = "abc";
        const char *s2 = "";
        assert(strspn(s1, s2) == 0);
    }

    /* [2][3] s1 为空字符串：初始段长度为 0 */
    {
        const char *s1 = "";
        const char *s2 = "abc";
        assert(strspn(s1, s2) == 0);
    }

    /* [2][3] s1 与 s2 均为空字符串：长度为 0 */
    {
        const char *s1 = "";
        const char *s2 = "";
        assert(strspn(s1, s2) == 0);
    }

    /* [2][3] s2 含重复字符：不影响结果 */
    {
        const char *s1 = "aabbccdd";
        const char *s2 = "abcabc";
        /* 初始段 "aabbcc" 全部来自 {a,b,c}，遇到 'd' 停止，长度 6 */
        assert(strspn(s1, s2) == 6);
    }

    /* [2][3] s2 含空白/标点等非字母字符 */
    {
        const char *s1 = "  \t\nabc";
        const char *s2 = " \t\n";
        /* 初始段为三个空白字符，长度 3 */
        assert(strspn(s1, s2) == 3);
    }

    /* [2][3] 返回类型为 size_t，可安全与 size_t 比较 */
    {
        const char *s1 = "12345abc";
        const char *s2 = "0123456789";
        size_t n = strspn(s1, s2);
        assert(n == (size_t)5);
    }

    /* [2][3] 与 strcspn 互补关系（同一 s1/s2 下二者之和为 s1 长度） */
    {
        const char *s1 = "hello world";
        const char *s2 = "helo";
        size_t a = strspn(s1, s2);
        size_t b = strcspn(s1, s2);
        assert(a + b == strlen(s1));
    }

    /* [2][3] 参数为 const char *：传入字符串字面量不产生警告/错误 */
    {
        assert(strspn("abcdef", "fedcba") == 6);
    }

    printf("All positive tests for C99 7.21.5.6 (strspn) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strspn 原型为 size_t strspn(const char *, const char *)」：
 * 参数个数不匹配（只传一个实参），gcc -std=c99 应报错：
 *   error: too few arguments to function 'strspn' */
{
    const char *s1 = "abc";
    size_t n = strspn(s1);
    (void)n;
}

/* 违反约束「strspn 原型为 size_t strspn(const char *, const char *)」：
 * 参数个数不匹配（传三个实参），gcc -std=c99 应报错：
 *   error: too many arguments to function 'strspn' */
{
    const char *s1 = "abc";
    const char *s2 = "abc";
    size_t n = strspn(s1, s2, s2);
    (void)n;
}

/* 违反约束「参数类型必须为 const char *（即指向字符的指针）」：
 * 传入 int 实参，gcc -std=c99 应报错：
 *   warning/error: passing argument 1 of 'strspn' makes pointer from integer
 *   without a cast（在 -Werror 下为错误） */
{
    size_t n = strspn(42, "abc");
    (void)n;
}

/* 违反约束「参数类型必须为 const char *」：
 * 传入 double 实参，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'strspn' */
{
    size_t n = strspn(3.14, "abc");
    (void)n;
}

/* 违反约束「strspn 返回 size_t，不可作为左值被赋值」：
 * 对函数调用结果赋值，gcc -std=c99 应报错：
 *   error: lvalue required as left operand of assignment */
{
    const char *s1 = "abc";
    const char *s2 = "abc";
    strspn(s1, s2) = 0;
}

/* 违反约束「strspn 返回 size_t，不可取地址」：
 * 对函数调用结果取地址，gcc -std=c99 应报错：
 *   error: lvalue required as unary '&' operand */
{
    const char *s1 = "abc";
    const char *s2 = "abc";
    size_t *p = &strspn(s1, s2);
    (void)p;
}

/* 违反约束「strspn 返回 size_t，不可自增」：
 * 对函数调用结果使用 ++，gcc -std=c99 应报错：
 *   error: lvalue required as increment operand */
{
    const char *s1 = "abc";
    const char *s2 = "abc";
    strspn(s1, s2)++;
}

#endif /* 负向测试结束 */