/*
 * 测试条款：C99 7.24.4.2.1  wcscpy 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明：wchar_t *wcscpy(wchar_t * restrict s1, const wchar_t * restrict s2);
 *   [2] 语义：把 s2 指向的宽字符串（含结尾空宽字符）复制到 s1 指向的数组。
 *   [3] 返回值：返回 s1 的值。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，类型必须与标准原型一致。
 *     若原型不匹配（例如参数类型错误），此处会编译报错。 */
static wchar_t *(*fp_wcscpy)(wchar_t *restrict, const wchar_t *restrict) = wcscpy;

int main(void)
{
    /* [1] 头文件 <wchar.h> 提供 wcscpy 声明，且返回类型为 wchar_t * */
    wchar_t dst[32];
    const wchar_t *src = L"Hello, wide world!";

    /* [2] 复制：s2 指向的宽字符串（含结尾空宽字符）复制到 s1 指向的数组 */
    wchar_t *ret = wcscpy(dst, src);

    /* [3] 返回值必须等于 s1（即 dst） */
    assert(ret == dst);

    /* [2] 内容必须逐字符相等，包括结尾空宽字符 */
    {
        size_t i = 0;
        for (;;) {
            assert(dst[i] == src[i]);
            if (src[i] == L'\0')
                break;
            ++i;
        }
        /* 结尾空宽字符确实被复制 */
        assert(dst[i] == L'\0');
        /* 长度一致 */
        assert(wcslen(dst) == wcslen(src));
    }

    /* [2] 空宽字符串（仅含结尾空宽字符）也应被正确复制 */
    {
        wchar_t empty_dst[4] = L"XYZ"; /* 预置非零内容 */
        wchar_t *r2 = wcscpy(empty_dst, L"");
        assert(r2 == empty_dst);
        assert(empty_dst[0] == L'\0');
    }

    /* [2][3] 通过函数指针调用，验证原型与行为一致 */
    {
        wchar_t dst2[16];
        wchar_t *r3 = fp_wcscpy(dst2, L"abc");
        assert(r3 == dst2);
        assert(dst2[0] == L'a' && dst2[1] == L'b' &&
               dst2[2] == L'c' && dst2[3] == L'\0');
    }

    /* [2] 复制后源字符串保持不变（s2 为 const，语义上不应被修改） */
    assert(src[0] == L'H' && src[1] == L'e' && src[2] == L'l');

    /* [3] 返回值可用于链式使用：wcscpy 的返回值就是目标指针 */
    {
        wchar_t a[8], b[8];
        wchar_t *ra = wcscpy(a, L"one");
        wchar_t *rb = wcscpy(b, ra); /* 用上一次的返回值作为源 */
        assert(ra == a && rb == b);
        assert(wcscmp(a, b) == 0);
        assert(wcscmp(b, L"one") == 0);
    }

    printf("All positive tests for C99 7.24.4.2.1 (wcscpy) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcscpy 的第一个参数类型为 wchar_t *」：
 * 传入 const wchar_t * 作为目标（丢弃 const 限定），
 * gcc -std=c99 应报错：passing argument 1 ... discards 'const' qualifier。 */
{
    const wchar_t buf[16];
    wcscpy(buf, L"x");
}

/* 违反约束「wcscpy 的第二个参数类型为 const wchar_t *」：
 * 传入 int * 作为源，类型不兼容，
 * gcc -std=c99 应报错：incompatible pointer type。 */
{
    wchar_t dst[16];
    int nums[4] = {0};
    wcscpy(dst, nums);
}

/* 违反约束「wcscpy 的第二个参数类型为 const wchar_t *」：
 * 传入窄字符指针 char *，类型不兼容，
 * gcc -std=c99 应报错：incompatible pointer type。 */
{
    wchar_t dst[16];
    char narrow[8] = "abc";
    wcscpy(dst, narrow);
}

/* 违反约束「wcscpy 返回 wchar_t *，不能作为赋值目标」：
 * 函数调用结果不是左值，对其赋值应编译报错：
 * lvalue required as left operand of assignment。 */
{
    wchar_t dst[16];
    wcscpy(dst, L"x") = dst;
}

/* 违反约束「wcscpy 需要两个实参」：
 * 实参个数不足，应编译报错：too few arguments to function 'wcscpy'。 */
{
    wchar_t dst[16];
    wcscpy(dst);
}

/* 违反约束「wcscpy 需要两个实参」：
 * 实参个数过多，应编译报错：too many arguments to function 'wcscpy'。 */
{
    wchar_t dst[16];
    wcscpy(dst, L"x", L"y");
}

/* 违反约束「wcscpy 返回 wchar_t *，不能用于需要算术类型的场合」：
 * 对指针返回值做乘法，应编译报错：invalid operands to binary *。 */
{
    wchar_t dst[16];
    wchar_t *p = wcscpy(dst, L"x");
    (void)(p * 2);
}

#endif /* 负向测试结束 */