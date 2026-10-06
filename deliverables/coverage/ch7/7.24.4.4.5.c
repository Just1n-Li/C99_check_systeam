/*
 * 测试 C99 7.24.4.4.5 —— wmemcmp 函数
 *
 * 预期行为：
 *   正向测试：包含 <wchar.h>，调用 wmemcmp 比较两个宽字符对象的前 n 个宽字符，
 *             返回值 >0 / ==0 / <0 分别对应 s1 大于 / 等于 / 小于 s2。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），期望编译器报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：声明 int wmemcmp(const wchar_t *, const wchar_t *, size_t);
 *   [2] Description：比较前 n 个宽字符
 *   [3] Returns：返回 >0 / ==0 / <0
 */

#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证函数可被调用，且参数类型为 const wchar_t * 与 size_t，
 *     返回类型为 int。通过函数指针赋值来静态检查签名。 */
static int (*fp_wmemcmp)(const wchar_t *, const wchar_t *, size_t) = wmemcmp;

int main(void)
{
    /* [1] 函数指针签名匹配 */
    assert(fp_wmemcmp == wmemcmp);

    /* [2][3] 相等：前 n 个宽字符完全相同 -> 返回 0 */
    {
        const wchar_t a[] = L"abcdef";
        const wchar_t b[] = L"abcdef";
        int r = wmemcmp(a, b, 6);
        assert(r == 0);
    }

    /* [2][3] 相等：n 为 0 时，不比较任何字符 -> 返回 0 */
    {
        const wchar_t a[] = L"xyz";
        const wchar_t b[] = L"abc";
        int r = wmemcmp(a, b, (size_t)0);
        assert(r == 0);
    }

    /* [2][3] s1 大于 s2：第一个不同宽字符处 s1 的值更大 -> 返回 > 0 */
    {
        const wchar_t a[] = L"abd";
        const wchar_t b[] = L"abc";
        int r = wmemcmp(a, b, 3);
        assert(r > 0);
    }

    /* [2][3] s1 小于 s2：第一个不同宽字符处 s1 的值更小 -> 返回 < 0 */
    {
        const wchar_t a[] = L"abc";
        const wchar_t b[] = L"abd";
        int r = wmemcmp(a, b, 3);
        assert(r < 0);
    }

    /* [2] 只比较前 n 个宽字符：第 n 个之后的差异不影响结果 */
    {
        const wchar_t a[] = L"abcX";
        const wchar_t b[] = L"abcY";
        int r = wmemcmp(a, b, 3);   /* 只比较 "abc" 与 "abc" */
        assert(r == 0);
    }

    /* [2] 前 n 个宽字符中第 n 个位置不同，应被比较到 */
    {
        const wchar_t a[] = L"abcX";
        const wchar_t b[] = L"abcY";
        int r = wmemcmp(a, b, 4);   /* 比较到第 4 个字符 'X' vs 'Y' */
        assert(r < 0);              /* 'X' < 'Y' */
    }

    /* [2] 比较的是宽字符值（wchar_t），不是字节 */
    {
        const wchar_t a[] = { 0x0041, 0x0042, 0x0000 };  /* L"A", L"B", L"\0" */
        const wchar_t b[] = { 0x0041, 0x0043, 0x0000 };  /* L"A", L"C", L"\0" */
        int r = wmemcmp(a, b, 3);
        assert(r < 0);              /* 0x0042 < 0x0043 */
    }

    /* [2] 嵌入的 L'\0' 也参与比较（wmemcmp 不因空字符停止） */
    {
        const wchar_t a[] = { L'a', L'\0', L'b' };
        const wchar_t b[] = { L'a', L'\0', L'c' };
        int r = wmemcmp(a, b, 3);
        assert(r < 0);              /* 第三个字符 'b' < 'c' */
    }

    /* [3] 返回值符号一致性：交换参数后符号相反 */
    {
        const wchar_t a[] = L"hello";
        const wchar_t b[] = L"world";
        int r1 = wmemcmp(a, b, 5);
        int r2 = wmemcmp(b, a, 5);
        assert(r1 < 0);
        assert(r2 > 0);
    }

    /* [3] 返回值符号一致性：相等时交换仍为 0 */
    {
        const wchar_t a[] = L"same";
        const wchar_t b[] = L"same";
        assert(wmemcmp(a, b, 4) == 0);
        assert(wmemcmp(b, a, 4) == 0);
    }

    printf("All positive tests for C99 7.24.4.4.5 wmemcmp passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与形参兼容」：
 * wmemcmp 的第一个形参是 const wchar_t *，传入 int * 不兼容，
 * gcc -std=c99 应报错（incompatible pointer type / 参数类型不匹配）。 */
{
    int x[4] = {1, 2, 3, 4};
    int y[4] = {1, 2, 3, 4};
    wmemcmp(x, y, 4);   /* 错误：int * 不能转换为 const wchar_t * */
}

/* 违反约束「实参个数必须与形参个数一致」：
 * wmemcmp 需要 3 个实参，只给 2 个，应报错（too few arguments）。 */
{
    const wchar_t a[] = L"ab";
    const wchar_t b[] = L"ab";
    wmemcmp(a, b);      /* 错误：缺少第 3 个实参 size_t n */
}

/* 违反约束「实参个数必须与形参个数一致」：
 * 给 4 个实参，应报错（too many arguments）。 */
{
    const wchar_t a[] = L"ab";
    const wchar_t b[] = L"ab";
    wmemcmp(a, b, 2, 0); /* 错误：实参过多 */
}

/* 违反约束「第三个实参必须为整数类型（size_t）」：
 * 传入浮点常量，应报错（不能隐式转换为 size_t 的实参类型不匹配）。 */
{
    const wchar_t a[] = L"ab";
    const wchar_t b[] = L"ab";
    wmemcmp(a, b, 2.5);  /* 错误：double 实参传给 size_t 形参 */
}

/* 违反约束「返回值不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错（lvalue required as left operand of assignment）。 */
{
    const wchar_t a[] = L"ab";
    const wchar_t b[] = L"ab";
    wmemcmp(a, b, 2) = 0; /* 错误：函数返回值不是左值 */
}

/* 违反约束「const 限定对象不可被修改」：
 * 通过 wmemcmp 的 const wchar_t * 形参无法修改，但若试图直接修改
 * 传入的 const 数组元素，应报错（assignment of read-only location）。 */
{
    const wchar_t a[] = L"ab";
    a[0] = L'z';          /* 错误：a 是 const 限定，不可赋值 */
}

#endif /* 负向测试结束 */