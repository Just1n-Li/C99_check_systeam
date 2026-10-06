/*
 * 测试 C99 7.21.5.2 —— strchr 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明：char *strchr(const char *s, int c);
 *   [2] 语义：定位 s 中第一个出现的 c（转换为 char），终止空字符视为字符串一部分。
 *   [3] 返回值：指向被定位字符的指针；若字符未出现则返回空指针。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型：返回 char*，参数为 const char* 与 int。
 *     验证返回类型为 char*（可赋值给 char*），参数接受 int。 */
static char *(*fp_strchr)(const char *, int) = strchr;

int main(void)
{
    /* [1] 通过函数指针调用，确认原型签名匹配 */
    {
        char buf[] = "hello";
        char *p = fp_strchr(buf, 'e');
        assert(p != NULL);
        assert(*p == 'e');
    }

    /* [2] 定位第一个出现的字符 */
    {
        const char *s = "abcabc";
        char *p = strchr(s, 'b');
        assert(p != NULL);
        assert(p == s + 1);          /* 第一个 'b' 在索引 1 */
        assert(*p == 'b');
    }

    /* [2] 只返回第一次出现的位置，而非最后一次 */
    {
        const char *s = "xayaza";
        char *p = strchr(s, 'a');
        assert(p == s + 1);
    }

    /* [2] 参数 c 被转换为 char：传入 'A' 应能匹配 */
    {
        const char *s = "ABC";
        char *p = strchr(s, 'A');
        assert(p == s);
    }

    /* [2] 终止空字符被视为字符串的一部分：
     *     查找 '\0' 应返回指向终止空字符的指针。 */
    {
        const char *s = "abc";
        char *p = strchr(s, '\0');
        assert(p != NULL);
        assert(p == s + 3);          /* 指向终止空字符 */
        assert(*p == '\0');
    }

    /* [2] 空字符串：查找 '\0' 应返回指向该空字符的指针 */
    {
        const char *s = "";
        char *p = strchr(s, '\0');
        assert(p == s);
        assert(*p == '\0');
    }

    /* [3] 字符不存在时返回空指针 */
    {
        const char *s = "hello";
        char *p = strchr(s, 'z');
        assert(p == NULL);
    }

    /* [3] 空字符串中查找非空字符，返回空指针 */
    {
        const char *s = "";
        char *p = strchr(s, 'a');
        assert(p == NULL);
    }

    /* [2][3] 返回的指针可用于修改（s 指向可修改数组时） */
    {
        char s[] = "hello";
        char *p = strchr(s, 'l');
        assert(p != NULL);
        *p = 'L';
        assert(strcmp(s, "heLlo") == 0);
    }

    /* [2] c 转换为 char 的语义：传入 int 值，低 8 位决定匹配字符。
     *     例如 'a' 与 (int)'a' 相同；这里验证传入 int 常量。 */
    {
        const char *s = "abc";
        char *p = strchr(s, 98);     /* 98 == 'b' */
        assert(p == s + 1);
    }

    printf("All positive tests for C99 7.21.5.2 (strchr) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strchr 的第一个参数类型为 const char *」：
 * 传入 int 类型实参，gcc -std=c99 应报错
 * （int 不能隐式转换为指针类型）。 */
{
    int n = 0;
    char *p = strchr(n, 'a');   /* 错误：第一个实参应为 const char * */
    (void)p;
}

/* 违反约束「strchr 的第二个参数类型为 int」：
 * 传入结构体类型实参，gcc -std=c99 应报错。 */
{
    struct S { int x; } sv;
    char *p = strchr("abc", sv);  /* 错误：第二个实参应为 int */
    (void)p;
}

/* 违反约束「strchr 需要两个实参」：
 * 实参个数不足，gcc -std=c99 应报错。 */
{
    char *p = strchr("abc");      /* 错误：缺少第二个实参 */
    (void)p;
}

/* 违反约束「strchr 需要两个实参」：
 * 实参个数过多，gcc -std=c99 应报错。 */
{
    char *p = strchr("abc", 'a', 'b');  /* 错误：实参过多 */
    (void)p;
}

/* 违反约束「strchr 返回 char *，不能对非左值结果赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
{
    strchr("abc", 'a') = (char *)0;  /* 错误：赋值目标不是左值 */
}

#endif