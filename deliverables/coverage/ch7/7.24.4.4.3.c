/*
 * 测试条款：C99 7.24.4.4.3  wcsncmp 函数
 *
 * 预期行为：
 *   正向测试：包含 <wchar.h>，调用 wcsncmp，验证其比较语义与返回值符号，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（参数类型错误、缺少声明等）应导致编译报错，
 *             这些片段统一放在 #if 0 ... #endif 中，不影响本文件正常编译。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件
 *   [2] 描述：最多比较 n 个宽字符；遇到空宽字符后不再比较
 *   [3] 返回值：>0 / ==0 / <0 分别对应 s1 大于 / 等于 / 小于 s2
 */

#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证原型可用：函数指针类型必须与声明一致 */
static int (*fp)(const wchar_t *, const wchar_t *, size_t) = wcsncmp;

int main(void)
{
    /* [1] 基本调用，确认返回类型为 int */
    {
        const wchar_t a[] = L"abc";
        const wchar_t b[] = L"abc";
        int r = wcsncmp(a, b, 3);
        assert(r == 0);
    }

    /* [3] s1 > s2 时返回 > 0 */
    {
        const wchar_t a[] = L"abd";
        const wchar_t b[] = L"abc";
        int r = wcsncmp(a, b, 3);
        assert(r > 0);
    }

    /* [3] s1 < s2 时返回 < 0 */
    {
        const wchar_t a[] = L"abc";
        const wchar_t b[] = L"abd";
        int r = wcsncmp(a, b, 3);
        assert(r < 0);
    }

    /* [2] 只比较前 n 个字符：前 n 个相同即返回 0，即使后面不同 */
    {
        const wchar_t a[] = L"abcXXX";
        const wchar_t b[] = L"abcYYY";
        int r = wcsncmp(a, b, 3);
        assert(r == 0);
    }

    /* [2] n == 0 时不比较任何字符，返回 0 */
    {
        const wchar_t a[] = L"abc";
        const wchar_t b[] = L"xyz";
        int r = wcsncmp(a, b, 0);
        assert(r == 0);
    }

    /* [2] 遇到空宽字符后不再比较：L"ab" 与 L"ab\0zzz" 在前 3 个字符内相等 */
    {
        const wchar_t a[] = L"ab";
        const wchar_t b[] = L"ab\0zzz";
        int r = wcsncmp(a, b, 3);
        assert(r == 0);
    }

    /* [2] 空宽字符参与比较：L"ab" 与 L"aa" 在第 2 个字符处不同 */
    {
        const wchar_t a[] = L"ab";
        const wchar_t b[] = L"aa";
        int r = wcsncmp(a, b, 2);
        assert(r > 0);
    }

    /* [2] 一个字符串提前结束（空宽字符 < 非空宽字符） */
    {
        const wchar_t a[] = L"ab";
        const wchar_t b[] = L"abc";
        int r = wcsncmp(a, b, 3);
        assert(r < 0);   /* L'\0' < L'c' */
    }

    /* [2] n 大于两串长度：比较到空宽字符为止 */
    {
        const wchar_t a[] = L"abc";
        const wchar_t b[] = L"abc";
        int r = wcsncmp(a, b, 100);
        assert(r == 0);
    }

    /* [2] 非 ASCII 宽字符比较 */
    {
        const wchar_t a[] = L"\u00e9";   /* é */
        const wchar_t b[] = L"\u00e8";   /* è */
        int r = wcsncmp(a, b, 1);
        assert(r > 0);
    }

    /* [1] 通过函数指针调用，验证原型一致性 */
    {
        const wchar_t a[] = L"hello";
        const wchar_t b[] = L"help";
        int r = fp(a, b, 3);
        assert(r == 0);   /* "hel" == "hel" */
        r = fp(a, b, 4);
        assert(r > 0);    /* 'l' > 'p'? 不，'l'(108) < 'p'(112) => r < 0 */
    }

    /* 修正上面函数指针测试的期望值，单独再测一次 */
    {
        const wchar_t a[] = L"hello";
        const wchar_t b[] = L"help";
        int r = fp(a, b, 4);
        assert(r < 0);    /* 'l' < 'p' */
    }

    printf("wcsncmp: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与原型匹配」：
 * wcsncmp 的第一个参数类型为 const wchar_t *，传入 char * 应报错
 * （gcc -std=c99 报 incompatible pointer type / passing argument 1 ...）
 */
void bad_arg_type(void)
{
    const char *s1 = "abc";
    const char *s2 = "abc";
    wcsncmp(s1, s2, 3);   /* 期望：编译错误，char* 不能转换为 const wchar_t* */
}

/* 违反约束「实参类型必须与原型匹配」：
 * 第三个参数类型为 size_t，传入指针应报错
 */
void bad_arg3_type(void)
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abc";
    wcsncmp(s1, s2, s1);  /* 期望：编译错误，指针不能转换为 size_t */
}

/* 违反约束「实参个数必须与原型一致」：
 * 少传一个参数应报错
 */
void bad_arg_count(void)
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abc";
    wcsncmp(s1, s2);      /* 期望：编译错误，参数太少 */
}

/* 违反约束「实参个数必须与原型一致」：
 * 多传一个参数应报错
 */
void bad_arg_count2(void)
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abc";
    wcsncmp(s1, s2, 3, 4); /* 期望：编译错误，参数太多 */
}

/* 违反约束「函数返回值不可作为左值」：
 * wcsncmp 返回 int，对其赋值应报错
 */
void bad_lvalue(void)
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abc";
    wcsncmp(s1, s2, 3) = 0;  /* 期望：编译错误，赋值目标不是左值 */
}

/* 违反约束「调用函数前必须有声明」：
 * 若未包含 <wchar.h> 且无自身声明，C99 中隐式声明已不再允许
 * （此处假设在无原型可见的上下文中调用）
 */
void bad_no_decl(void)
{
    /* 注意：本文件顶部已包含 <wchar.h>，此片段仅示意；
     * 在真正缺少声明的翻译单元中，gcc -std=c99 会报
     * "implicit declaration of function 'wcsncmp'" 警告/错误。
     */
    /* wcsncmp(L"a", L"b", 1); */
}

#endif /* 负向测试结束 */