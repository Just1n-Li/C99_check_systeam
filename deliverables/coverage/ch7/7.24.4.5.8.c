/*
 * 测试 C99 7.24.4.5.8 —— wmemchr 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 条款要点：
 *   [1] 原型：wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n);
 *   [2] 在 s 指向对象的前 n 个宽字符中定位 c 的首次出现。
 *   [3] 返回指向该宽字符的指针；若未出现则返回空指针。
 */

#include <wchar.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证函数原型可用、返回类型为 wchar_t *、参数类型匹配 */
static void test_prototype(void)
{
    /* 通过函数指针类型检查原型签名 */
    wchar_t *(*fp)(const wchar_t *, wchar_t, size_t) = wmemchr;
    assert(fp != NULL);

    /* 直接调用，验证可编译 */
    const wchar_t s[] = L"abc";
    wchar_t *p = wmemchr(s, L'b', 3);
    assert(p != NULL);
}

/* [2] 在前 n 个宽字符中定位 c 的首次出现 */
static void test_locate_first(void)
{
    const wchar_t s[] = L"hello world";

    /* 定位 'o'：首次出现在索引 4 */
    wchar_t *p = wmemchr(s, L'o', 11);
    assert(p != NULL);
    assert(p == &s[4]);
    assert(*p == L'o');

    /* 定位 'h'：索引 0 */
    p = wmemchr(s, L'h', 11);
    assert(p == &s[0]);

    /* 定位 'd'：索引 10 */
    p = wmemchr(s, L'd', 11);
    assert(p == &s[10]);
}

/* [2] n 限制搜索范围：只在初始 n 个宽字符中查找 */
static void test_n_limit(void)
{
    const wchar_t s[] = L"abcdef";

    /* 'd' 在索引 3，但 n=3 只覆盖索引 0..2，应找不到 */
    wchar_t *p = wmemchr(s, L'd', 3);
    assert(p == NULL);

    /* n=4 覆盖索引 0..3，应找到 */
    p = wmemchr(s, L'd', 4);
    assert(p != NULL);
    assert(p == &s[3]);

    /* n=0：不搜索任何字符，必返回 NULL */
    p = wmemchr(s, L'a', 0);
    assert(p == NULL);
}

/* [2] 搜索包含嵌入空宽字符的数组（wmemchr 按宽字符计数，不受 L'\0' 影响） */
static void test_embedded_null(void)
{
    const wchar_t s[] = L"ab\0cd";   /* 6 个宽字符含结尾 L'\0' */

    /* 在索引 2 处有 L'\0' */
    wchar_t *p = wmemchr(s, L'\0', 6);
    assert(p != NULL);
    assert(p == &s[2]);

    /* 'c' 在索引 3，n=6 应能找到（越过嵌入的 L'\0'） */
    p = wmemchr(s, L'c', 6);
    assert(p != NULL);
    assert(p == &s[3]);

    /* 若 n 限制在 L'\0' 之前，则找不到 'c' */
    p = wmemchr(s, L'c', 2);
    assert(p == NULL);
}

/* [3] 未出现时返回空指针 */
static void test_not_found(void)
{
    const wchar_t s[] = L"xyz";

    wchar_t *p = wmemchr(s, L'a', 3);
    assert(p == NULL);

    p = wmemchr(s, L'z', 2);   /* 'z' 在索引 2，n=2 覆盖不到 */
    assert(p == NULL);
}

/* [3] 返回的指针可用于修改（s 非 const 时） */
static void test_return_writable(void)
{
    wchar_t s[] = L"hello";

    wchar_t *p = wmemchr(s, L'l', 5);
    assert(p == &s[2]);
    *p = L'L';                 /* 通过返回指针修改 */
    assert(s[2] == L'L');
    assert(wcscmp(s, L"heLlo") == 0);
}

/* [2][3] 综合：多次出现时返回首次出现 */
static void test_first_of_many(void)
{
    const wchar_t s[] = L"aXbXcX";

    wchar_t *p = wmemchr(s, L'X', 6);
    assert(p == &s[1]);        /* 首次出现，而非索引 3 或 5 */

    /* 从首次出现之后继续搜索，可找到下一次 */
    p = wmemchr(p + 1, L'X', 5);
    assert(p == &s[3]);
}

int main(void)
{
    test_prototype();
    test_locate_first();
    test_n_limit();
    test_embedded_null();
    test_not_found();
    test_return_writable();
    test_first_of_many();

    printf("C99 7.24.4.5.8 wmemchr: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与原型匹配」：
 * wmemchr 第二个参数类型为 wchar_t，传入 int 字面量（非 wchar_t）
 * 在严格原型下属于约束违反，gcc -std=c99 -Werror 应报错/告警。
 * 注：此处用指针实参制造明确的类型不匹配，确保报错。 */
#include <wchar.h>
void bad_arg_type(void)
{
    const wchar_t s[] = L"abc";
    int *wrong = 0;
    wmemchr(s, wrong, 3);      /* 第二个参数应为 wchar_t，传 int* 应报错 */
}

/* 违反约束「第一个参数必须是指向 const wchar_t 的指针」：
 * 传入 int* 而非 const wchar_t*，类型不兼容，应报错。 */
void bad_first_arg(void)
{
    int buf[4] = {0};
    wmemchr(buf, L'a', 4);     /* int* 不能隐式转换为 const wchar_t* */
}

/* 违反约束「第三个参数类型为 size_t」：
 * 传入指针类型，无法转换为 size_t，应报错。 */
void bad_third_arg(void)
{
    const wchar_t s[] = L"abc";
    int *p = 0;
    wmemchr(s, L'a', p);       /* 第三个参数应为 size_t，传 int* 应报错 */
}

/* 违反约束「返回值类型为 wchar_t*」：
 * 将返回值赋给不兼容的指针类型（无强制转换），应报错。 */
void bad_return_use(void)
{
    const wchar_t s[] = L"abc";
    int *p = wmemchr(s, L'a', 3);   /* wchar_t* 赋给 int* 应报错 */
    (void)p;
}

/* 违反约束「调用参数个数必须与原型一致」：
 * 原型有 3 个参数，只传 2 个，应报错。 */
void bad_arg_count(void)
{
    const wchar_t s[] = L"abc";
    wmemchr(s, L'a');          /* 缺少第三个参数，应报错 */
}

#endif /* 负向测试结束 */