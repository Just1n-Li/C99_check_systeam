/*
 * 测试条款：C99 7.24.4.3.2  wcsncat 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <wchar.h>
 *   [2] 语义：最多追加 n 个宽字符；s2 的首字符覆盖 s1 末尾的 L'\0'；
 *       结果总是以空宽字符结尾；s2 中空宽字符及其后字符不追加。
 *   [3] 返回值：返回 s1 的值。
 *   Footnote 298：s1 中最终最多可有 wcslen(s1)+n+1 个宽字符。
 */

#include <wchar.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，类型必须匹配 */
static wchar_t *(*fp_wcsncat)(wchar_t * restrict, const wchar_t * restrict, size_t)
    = wcsncat;

int main(void)
{
    /* [1] 原型可用，函数指针非空 */
    assert(fp_wcsncat == wcsncat);

    /* [2] 基本追加：n 足够大，等价于 wcscat 的效果 */
    {
        wchar_t s1[32] = L"Hello";
        const wchar_t s2[] = L", World";
        wchar_t *r = wcsncat(s1, s2, 100);
        assert(wcscmp(s1, L"Hello, World") == 0);
        /* [3] 返回值必须等于 s1 */
        assert(r == s1);
    }

    /* [2] 只追加不超过 n 个宽字符：n 小于 s2 长度 */
    {
        wchar_t s1[32] = L"abc";
        const wchar_t s2[] = L"defgh";
        wchar_t *r = wcsncat(s1, s2, 2);   /* 只追加 "de" */
        assert(wcscmp(s1, L"abcde") == 0);
        assert(r == s1);
        /* 结果总是以空宽字符结尾 */
        assert(s1[wcslen(L"abcde")] == L'\0');
    }

    /* [2] n == 0：不追加任何字符，但终止空宽字符仍被写入 */
    {
        wchar_t s1[16] = L"xyz";
        const wchar_t s2[] = L"QQQ";
        wchar_t *r = wcsncat(s1, s2, 0);
        assert(wcscmp(s1, L"xyz") == 0);
        assert(s1[3] == L'\0');
        assert(r == s1);
    }

    /* [2] s2 中的空宽字符及其后字符不被追加 */
    {
        wchar_t s1[32] = L"AB";
        /* s2 逻辑内容为 "CD"，其后还有 'E','F' 但不应被追加 */
        const wchar_t s2[] = L"CD\0EF";
        wchar_t *r = wcsncat(s1, s2, 10);
        assert(wcscmp(s1, L"ABCD") == 0);
        assert(s1[4] == L'\0');
        assert(r == s1);
    }

    /* [2] s2 首字符覆盖 s1 末尾的空宽字符（验证覆盖而非插入） */
    {
        wchar_t s1[16] = L"one";
        const wchar_t s2[] = L"two";
        wcsncat(s1, s2, 3);
        assert(s1[3] == L't');   /* 原 L'\0' 位置被 s2[0] 覆盖 */
        assert(wcscmp(s1, L"onetwo") == 0);
    }

    /* [2] s2 为空宽字符串：不追加字符，但终止空宽字符仍被写入 */
    {
        wchar_t s1[16] = L"keep";
        const wchar_t s2[] = L"";
        wchar_t *r = wcsncat(s1, s2, 5);
        assert(wcscmp(s1, L"keep") == 0);
        assert(s1[4] == L'\0');
        assert(r == s1);
    }

    /* [2] s1 为空宽字符串：结果就是 s2 的前 n 个字符 */
    {
        wchar_t s1[16] = L"";
        const wchar_t s2[] = L"12345";
        wcsncat(s1, s2, 3);
        assert(wcscmp(s1, L"123") == 0);
    }

    /* Footnote 298：最终长度 <= wcslen(s1_orig) + n + 1 */
    {
        wchar_t s1[64] = L"prefix";
        const wchar_t s2[] = L"0123456789";
        size_t n = 4;
        size_t before = wcslen(s1);
        wcsncat(s1, s2, n);
        size_t after = wcslen(s1);
        assert(after <= before + n + 1);
        assert(after == before + n);   /* 本例恰好等于 */
        assert(wcscmp(s1, L"prefix0123") == 0);
    }

    /* [3] 返回值可用于链式调用 */
    {
        wchar_t s1[32] = L"a";
        const wchar_t s2[] = L"b";
        const wchar_t s3[] = L"c";
        wchar_t *r = wcsncat(wcsncat(s1, s2, 1), s3, 1);
        assert(r == s1);
        assert(wcscmp(s1, L"abc") == 0);
    }

    /* [2] 追加后 s1 仍是合法宽字符串（以空宽字符结尾） */
    {
        wchar_t s1[8] = L"xy";
        const wchar_t s2[] = L"z";
        wcsncat(s1, s2, 1);
        assert(s1[wcslen(s1)] == L'\0');
    }

    printf("All positive tests for C99 7.24.4.3.2 wcsncat passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与原型匹配」：
 * wcsncat 的第一个参数类型为 wchar_t *，传入 int * 应报错。
 * 期望：gcc -std=c99 报 incompatible pointer type / passing argument 1 ... */
{
    int buf[16];
    const wchar_t *src = L"x";
    wcsncat(buf, src, 1);
}

/* 违反约束「实参类型必须与原型匹配」：
 * 第二个参数类型为 const wchar_t *，传入 char * 应报错。
 * 期望：gcc -std=c99 报 incompatible pointer type。 */
{
    wchar_t dst[16] = L"";
    char src[4] = "abc";
    wcsncat(dst, src, 1);
}

/* 违反约束「实参个数必须与原型一致」：
 * wcsncat 需要 3 个实参，只给 2 个应报错。
 * 期望：gcc -std=c99 报 too few arguments to function 'wcsncat'。 */
{
    wchar_t dst[16] = L"";
    const wchar_t *src = L"x";
    wcsncat(dst, src);
}

/* 违反约束「实参个数必须与原型一致」：
 * 多给一个实参应报错。
 * 期望：gcc -std=c99 报 too many arguments to function 'wcsncat'。 */
{
    wchar_t dst[16] = L"";
    const wchar_t *src = L"x";
    wcsncat(dst, src, 1, 2);
}

/* 违反约束「第三个参数必须为整型（size_t）」：
 * 传入结构体类型应报错。
 * 期望：gcc -std=c99 报 incompatible type for argument 3。 */
{
    struct S { int x; } bad;
    wchar_t dst[16] = L"";
    const wchar_t *src = L"x";
    wcsncat(dst, src, bad);
}

/* 违反约束「第一个参数指向的对象必须可修改」：
 * 传入 const wchar_t * 作为 s1，丢弃 const 限定应报错。
 * 期望：gcc -std=c99 报 passing argument 1 ... discards 'const' qualifier。 */
{
    const wchar_t dst[16] = L"";
    const wchar_t *src = L"x";
    wcsncat(dst, src, 1);
}

/* 违反约束「函数返回值不可作为左值赋值」：
 * wcsncat 返回 wchar_t *（非左值），对其赋值应报错。
 * 期望：gcc -std=c99 报 lvalue required as left operand of assignment。 */
{
    wchar_t dst[16] = L"";
    const wchar_t *src = L"x";
    wcsncat(dst, src, 1) = dst;
}

#endif /* 负向测试结束 */