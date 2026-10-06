/*
 * 测试条款：C99 7.24.4.3.1  wcscat 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <wchar.h>
 *   [2] 语义：把 s2（含结尾空宽字符）追加到 s1 末尾，
 *       s2 的首个宽字符覆盖 s1 末尾的空宽字符
 *   [3] 返回值：返回 s1 的值
 */

#include <wchar.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <wchar.h> 提供 wcscat 声明，原型为：
 *     wchar_t *wcscat(wchar_t * restrict s1, const wchar_t * restrict s2);
 *     下面通过取函数指针并赋给匹配的原型来静态验证签名。 */
static wchar_t *(*fp_wcscat)(wchar_t * restrict, const wchar_t * restrict) = wcscat;

static void test_basic_append(void)
{
    /* [2] 基本追加：s2 的内容（含结尾空宽字符）接到 s1 末尾，
     *     s2 的首字符覆盖 s1 原来的结尾空宽字符。 */
    wchar_t s1[32] = L"Hello";
    const wchar_t s2[] = L", World";

    wchar_t *ret = wcscat(s1, s2);

    /* [3] 返回值等于 s1 */
    assert(ret == s1);
    /* [2] 结果字符串正确 */
    assert(wcscmp(s1, L"Hello, World") == 0);
    /* 长度检查：5 + 7 = 12 */
    assert(wcslen(s1) == 12);
}

static void test_empty_s2(void)
{
    /* [2] s2 为空宽字符串：只有结尾空宽字符，
     *     它覆盖 s1 末尾的空宽字符，s1 内容不变。 */
    wchar_t s1[16] = L"abc";
    const wchar_t s2[] = L"";

    wchar_t *ret = wcscat(s1, s2);

    assert(ret == s1);
    assert(wcscmp(s1, L"abc") == 0);
    assert(wcslen(s1) == 3);
}

static void test_empty_s1(void)
{
    /* [2] s1 为空宽字符串：s2 的首字符覆盖 s1 的空宽字符，
     *     结果等于 s2 的拷贝。 */
    wchar_t s1[16] = L"";
    const wchar_t s2[] = L"xyz";

    wchar_t *ret = wcscat(s1, s2);

    assert(ret == s1);
    assert(wcscmp(s1, L"xyz") == 0);
    assert(wcslen(s1) == 3);
}

static void test_both_empty(void)
{
    /* [2] 两者都为空：结果仍为空宽字符串。 */
    wchar_t s1[8] = L"";
    const wchar_t s2[] = L"";

    wchar_t *ret = wcscat(s1, s2);

    assert(ret == s1);
    assert(s1[0] == L'\0');
    assert(wcslen(s1) == 0);
}

static void test_return_value_identity(void)
{
    /* [3] 返回值就是传入的 s1 指针本身（同一对象）。 */
    wchar_t s1[32] = L"AA";
    const wchar_t s2[] = L"BB";

    wchar_t *ret = wcscat(s1, s2);

    assert(ret == s1);
    /* 通过返回值访问到的内容与 s1 一致 */
    assert(wcscmp(ret, L"AABB") == 0);
    assert(ret[0] == L'A');
    assert(ret[4] == L'\0');
}

static void test_chained_use_of_return(void)
{
    /* [3] 返回值可直接用于链式调用（返回 s1）。 */
    wchar_t s1[32] = L"1";
    const wchar_t s2[] = L"2";
    const wchar_t s3[] = L"3";

    wchar_t *ret = wcscat(wcscat(s1, s2), s3);

    assert(ret == s1);
    assert(wcscmp(s1, L"123") == 0);
}

static void test_overwrite_null_terminator(void)
{
    /* [2] 明确验证：s2 的首字符覆盖 s1 末尾的空宽字符。
     *     追加前 s1[2] == L'\0'，追加后 s1[2] == L'X'。 */
    wchar_t s1[16] = L"ab";
    const wchar_t s2[] = L"XY";

    assert(s1[2] == L'\0');       /* 追加前：空宽字符位置 */
    wcscat(s1, s2);
    assert(s1[2] == L'X');        /* 被 s2 首字符覆盖 */
    assert(s1[3] == L'Y');
    assert(s1[4] == L'\0');       /* 新的结尾空宽字符 */
}

static void test_wide_characters(void)
{
    /* [2] 处理真正的宽字符（非 ASCII），验证逐宽字符追加。 */
    wchar_t s1[32] = L"\u4F60\u597D";   /* 你好 */
    const wchar_t s2[] = L"\u4E16\u754C"; /* 世界 */

    wchar_t *ret = wcscat(s1, s2);

    assert(ret == s1);
    assert(wcslen(s1) == 4);
    assert(s1[0] == L'\u4F60');
    assert(s1[1] == L'\u597D');
    assert(s1[2] == L'\u4E16');
    assert(s1[3] == L'\u754C');
    assert(s1[4] == L'\0');
}

static void test_const_s2(void)
{
    /* [1] 第二个参数是 const wchar_t *：可以传入 const 数组。 */
    wchar_t s1[16] = L"a";
    const wchar_t s2[] = L"bc";

    wchar_t *ret = wcscat(s1, s2);
    assert(ret == s1);
    assert(wcscmp(s1, L"abc") == 0);
}

int main(void)
{
    /* 静态验证函数指针签名匹配（[1]） */
    assert(fp_wcscat == wcscat);

    test_basic_append();
    test_empty_s2();
    test_empty_s1();
    test_both_empty();
    test_return_value_identity();
    test_chained_use_of_return();
    test_overwrite_null_terminator();
    test_wide_characters();
    test_const_s2();

    printf("All positive tests for C99 7.24.4.3.1 (wcscat) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcscat 的第一个参数类型为 wchar_t *（可修改的宽字符指针）」：
 * 传入 const wchar_t * 会丢弃 const 限定，gcc -std=c99 应报错
 * （discards qualifiers / assignment from incompatible pointer type）。 */
void neg_const_first_arg(void)
{
    const wchar_t s1[16] = L"abc";
    const wchar_t s2[] = L"def";
    wcscat(s1, s2);   /* 错误：s1 为 const wchar_t *，不能传给 wchar_t * */
}

/* 违反约束「wcscat 的第一个参数类型为 wchar_t *」：
 * 传入窄字符指针 char *，类型不兼容，应报错。 */
void neg_wrong_type_first_arg(void)
{
    char s1[16] = "abc";
    const wchar_t s2[] = L"def";
    wcscat(s1, s2);   /* 错误：char * 与 wchar_t * 不兼容 */
}

/* 违反约束「wcscat 的第二个参数类型为 const wchar_t *」：
 * 传入窄字符指针 char *，类型不兼容，应报错。 */
void neg_wrong_type_second_arg(void)
{
    wchar_t s1[16] = L"abc";
    char s2[16] = "def";
    wcscat(s1, s2);   /* 错误：char * 与 const wchar_t * 不兼容 */
}

/* 违反约束「wcscat 需要两个参数」：
 * 参数个数不匹配，应报错（too few arguments to function 'wcscat'）。 */
void neg_too_few_args(void)
{
    wchar_t s1[16] = L"abc";
    wcscat(s1);       /* 错误：缺少第二个参数 */
}

/* 违反约束「wcscat 需要两个参数」：
 * 参数过多，应报错（too many arguments to function 'wcscat'）。 */
void neg_too_many_args(void)
{
    wchar_t s1[16] = L"abc";
    const wchar_t s2[] = L"def";
    wcscat(s1, s2, s2);   /* 错误：参数过多 */
}

/* 违反约束「wcscat 的返回类型为 wchar_t *」：
 * 把返回值赋给不兼容的指针类型（int *），应报错。 */
void neg_wrong_return_type(void)
{
    wchar_t s1[16] = L"abc";
    const wchar_t s2[] = L"def";
    int *p = wcscat(s1, s2);   /* 错误：wchar_t * 不能初始化 int * */
    (void)p;
}

/* 违反约束「wcscat 的第一个参数必须是可修改的左值所指向的对象」：
 * 传入字符串字面量（类型为 wchar_t[N]，会退化为 wchar_t *，
 * 但字面量本身不可修改；此处用取地址方式制造类型不匹配以触发编译错误）。 */
void neg_string_literal_first_arg(void)
{
    const wchar_t s2[] = L"def";
    wcscat(L"abc", s2);   /* 错误：L"abc" 为 const wchar_t *，不能传给 wchar_t * */
}

#endif /* 负向测试结束 */