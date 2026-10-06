/*
 * 测试目标：C99 7.24.4.6.1 —— wcslen 函数
 *   正向：验证 wcslen 计算宽字符串中终止空宽字符之前的宽字符个数，
 *         并验证其声明形式 size_t wcslen(const wchar_t *s)。
 *   负向：验证违反约束的调用（参数类型不匹配、缺少原型声明等）应编译报错。
 * 预期：正向部分编译运行通过（assert 全部成立）；
 *       负向部分位于 #if 0 中，单独取出编译时应报错。
 */

#include <wchar.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 声明形式检查：wcslen 返回 size_t，参数为 const wchar_t *。
 *     通过函数指针赋值来静态验证原型。 */
static size_t (*fp_wcslen)(const wchar_t *) = wcslen;

int main(void)
{
    /* [1] 原型可用性：能取地址并赋给匹配的函数指针类型 */
    assert(fp_wcslen == wcslen);

    /* [2][3] 基本语义：空宽字符串长度为 0 */
    {
        const wchar_t s[] = L"";
        size_t n = wcslen(s);
        assert(n == 0);
    }

    /* [2][3] 单字符宽字符串长度为 1 */
    {
        const wchar_t s[] = L"A";
        assert(wcslen(s) == 1);
    }

    /* [2][3] 多字符宽字符串：终止空宽字符之前的宽字符个数 */
    {
        const wchar_t s[] = L"hello";
        assert(wcslen(s) == 5);
    }

    /* [2][3] 含内嵌非 ASCII 宽字符（每个宽字符计 1，而非字节数） */
    {
        const wchar_t s[] = L"\u4e2d\u6587"; /* 两个宽字符 */
        assert(wcslen(s) == 2);
    }

    /* [2][3] 含空格与标点，仍按宽字符计数 */
    {
        const wchar_t s[] = L"a b, c!";
        assert(wcslen(s) == 7);
    }

    /* [2][3] 终止空宽字符之后的内容不计入长度 */
    {
        const wchar_t s[] = L"abc\0def";
        assert(wcslen(s) == 3);
    }

    /* [2][3] 通过指针偏移调用，验证参数为 const wchar_t * */
    {
        const wchar_t s[] = L"abcdef";
        const wchar_t *p = s + 2;
        assert(wcslen(p) == 4);
    }

    /* [2][3] 返回值类型为 size_t，可与 size_t 变量比较 */
    {
        const wchar_t s[] = L"1234567890";
        size_t n = wcslen(s);
        assert(n == (size_t)10);
        assert(n == sizeof(L"1234567890") / sizeof(wchar_t) - 1);
    }

    /* [2][3] 与 sizeof 推导的长度一致（不含终止空宽字符） */
    {
        const wchar_t s[] = L"wide string";
        assert(wcslen(s) == sizeof(s) / sizeof(s[0]) - 1);
    }

    printf("All positive tests for C99 7.24.4.6.1 (wcslen) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与形参 const wchar_t * 兼容」：
 * 传入 char * 而非 wchar_t *，gcc -std=c99 应报 incompatible pointer type 警告/错误。 */
#include <wchar.h>
void bad1(void)
{
    char s[] = "abc";
    size_t n = wcslen(s); /* 错误：char * 不能传给 const wchar_t * */
    (void)n;
}

/* 违反约束「实参必须是指针类型」：
 * 传入整数常量，gcc -std=c99 应报 incompatible integer to pointer 错误。 */
#include <wchar.h>
void bad2(void)
{
    size_t n = wcslen(0); /* 错误：0 作为整型常量传给指针形参 */
    (void)n;
}

/* 违反约束「实参必须是指针类型」：
 * 传入 wchar_t 值而非指针，gcc -std=c99 应报错误。 */
#include <wchar.h>
void bad3(void)
{
    wchar_t c = L'x';
    size_t n = wcslen(c); /* 错误：wchar_t 不能传给 const wchar_t * */
    (void)n;
}

/* 违反约束「函数调用必须使用正确原型」：
 * 未包含 <wchar.h> 且未声明 wcslen，C99 中隐式函数声明为约束违反，
 * gcc -std=c99 应报 implicit declaration 错误。 */
void bad4(void)
{
    const wchar_t s[] = L"abc";
    size_t n = wcslen(s); /* 错误：无原型声明 */
    (void)n;
}

/* 违反约束「形参为 const wchar_t *，不能用于修改」：
 * 通过 wcslen 的形参类型推导出的指针不应被写入；
 * 这里直接对 const 指针解引用赋值，gcc -std=c99 应报 read-only 错误。 */
#include <wchar.h>
void bad5(void)
{
    const wchar_t s[] = L"abc";
    const wchar_t *p = s;
    *p = L'z'; /* 错误：对 const 限定对象赋值 */
}

#endif /* 负向测试结束 */