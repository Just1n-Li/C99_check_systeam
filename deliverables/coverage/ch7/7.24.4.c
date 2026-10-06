/*
 * 测试目标：C99 7.24.4 General wide string utilities
 * 预期行为：
 *   正向测试：以下使用 <wchar.h> 宽字符串工具函数的代码应能编译并运行通过。
 *   负向测试：违反约束的代码片段应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：
 *   [1] <wchar.h> 声明宽字符串操作函数；wchar_t* 指向数组首元素（最低地址）。
 *   [2] 参数 size_t n 可为 0；此时指针参数仍须有效（7.1.4）；
 *       n==0 时：查找函数找不到、比较函数返回 0、复制函数复制 0 个宽字符。
 */

#include <stdio.h>
#include <wchar.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <wchar.h> 声明宽字符串工具函数，wchar_t* 指向数组首元素 */
static void test_header_and_pointer_semantics(void)
{
    wchar_t buf[16] = L"hello";
    wchar_t *p = buf;          /* 指向最低地址元素 */
    assert(p == &buf[0]);      /* [1] 指向初始（最低地址）元素 */

    /* 使用 <wchar.h> 中声明的函数 */
    size_t len = wcslen(p);
    assert(len == 5);

    /* wcscmp / wcscpy / wcschr 等均来自 <wchar.h> */
    wchar_t dst[16];
    wcscpy(dst, p);
    assert(wcscmp(dst, L"hello") == 0);
    assert(wcschr(dst, L'e') == &dst[1]);
}

/* [2] n == 0 时：查找函数找不到（返回 NULL） */
static void test_n_zero_search(void)
{
    wchar_t s[] = L"abcdef";
    /* wmemchr：在 n 个宽字符中查找，n==0 时找不到 */
    wchar_t *r = wmemchr(s, L'c', (size_t)0);
    assert(r == NULL);         /* [2] 查找函数找不到 */

    /* wcschr 不涉及 n，但 wcsstr 等也遵循查找语义；此处用 wmemchr 覆盖 n==0 */
    wchar_t *r2 = wmemchr(s, L'a', (size_t)0);
    assert(r2 == NULL);
}

/* [2] n == 0 时：比较函数返回 0 */
static void test_n_zero_compare(void)
{
    wchar_t a[] = L"xyz";
    wchar_t b[] = L"abc";
    /* wmemcmp：比较 n 个宽字符，n==0 时返回 0 */
    int c = wmemcmp(a, b, (size_t)0);
    assert(c == 0);            /* [2] 比较函数返回零 */

    /* 即使内容不同，n==0 也返回 0 */
    int c2 = wmemcmp(a, a, (size_t)0);
    assert(c2 == 0);
}

/* [2] n == 0 时：复制函数复制 0 个宽字符（目标不变） */
static void test_n_zero_copy(void)
{
    wchar_t src[] = L"source";
    wchar_t dst[16];
    wcscpy(dst, L"KEEP");      /* 先写入哨兵内容 */
    /* wmemcpy：复制 n 个宽字符，n==0 时不复制任何字符 */
    wchar_t *ret = wmemcpy(dst, src, (size_t)0);
    assert(ret == dst);        /* 返回目标指针 */
    assert(wcscmp(dst, L"KEEP") == 0);  /* [2] 复制 0 个宽字符，目标未变 */

    /* wmemmove 同理 */
    wchar_t dst2[16];
    wcscpy(dst2, L"STAY");
    wmemmove(dst2, src, (size_t)0);
    assert(wcscmp(dst2, L"STAY") == 0);

    /* wmemset：设置 n 个宽字符，n==0 时不修改 */
    wchar_t dst3[16];
    wcscpy(dst3, L"FIXED");
    wmemset(dst3, L'Z', (size_t)0);
    assert(wcscmp(dst3, L"FIXED") == 0);
}

/* [2] n == 0 时指针参数仍须有效（7.1.4）：使用有效指针调用 */
static void test_n_zero_valid_pointers(void)
{
    wchar_t s[] = L"valid";
    /* 指针有效，n==0，调用合法 */
    assert(wmemchr(s, L'v', (size_t)0) == NULL);
    assert(wmemcmp(s, s, (size_t)0) == 0);
    wchar_t d[8];
    wmemcpy(d, s, (size_t)0);   /* 合法：指针有效 */
    wmemmove(d, s, (size_t)0);
    wmemset(d, L'q', (size_t)0);
}

/* [1] 数组访问不得越界（此处仅做合法访问，越界为 UB 不作负向测试） */
static void test_in_bounds_access(void)
{
    wchar_t s[] = L"bounds";
    size_t n = wcslen(s);
    for (size_t i = 0; i < n; ++i) {
        assert(s[i] != L'\0');
    }
    assert(s[n] == L'\0');     /* 合法访问终止符 */
}

int main(void)
{
    test_header_and_pointer_semantics();
    test_n_zero_search();
    test_n_zero_compare();
    test_n_zero_copy();
    test_n_zero_valid_pointers();
    test_in_bounds_access();

    printf("C99 7.24.4 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wchar_t* 参数必须指向有效对象（7.1.4）」：
 * 传入空指针并期望 n==0 时“安全”，但 7.1.4 要求指针参数有效，
 * 空指针不是有效值，gcc -std=c99 在 -Wall 下应给出警告/错误。
 * 注：此处以“无效指针”演示约束违反，编译器应拒绝或告警。 */
#include <wchar.h>
void bad_null_pointer(void)
{
    wchar_t *p = (wchar_t *)0;      /* 空指针，非有效值 */
    wmemchr(p, L'a', (size_t)0);    /* 违反 7.1.4：指针参数须有效 */
}

/* 违反约束「wchar_t* 参数类型」：
 * 传入 int* 而非 wchar_t*，类型不匹配，应编译报错。 */
void bad_wrong_pointer_type(void)
{
    int arr[4] = {1, 2, 3, 4};
    wcslen(arr);                    /* 违反：参数应为 wchar_t*，实参为 int* */
}

/* 违反约束「size_t n 参数类型」：
 * 传入结构体而非整数类型，应编译报错。 */
struct NotSize { int x; };
void bad_n_type(void)
{
    wchar_t s[] = L"abc";
    struct NotSize n;
    wmemchr(s, L'a', n);            /* 违反：n 应为 size_t，实参为结构体 */
}

/* 违反约束「函数参数个数」：
 * wmemcpy 需要 3 个参数，少传参数应编译报错。 */
void bad_arg_count(void)
{
    wchar_t a[4], b[4];
    wmemcpy(a, b);                  /* 违反：缺少 size_t n 参数 */
}

/* 违反约束「返回类型使用」：
 * wcscmp 返回 int，将其赋给结构体类型应编译报错。 */
struct S { int v; };
void bad_return_use(void)
{
    wchar_t a[] = L"x", b[] = L"y";
    struct S s = wcscmp(a, b);      /* 违反：int 不能初始化结构体 */
}

#endif