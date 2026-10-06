/*
 * 测试条款：C99 7.24.4.4.1  wcscmp 函数
 *
 * 预期行为：
 *   正向测试：包含 <wchar.h>，调用 wcscmp 比较宽字符串，返回值符号正确，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如参数类型错误、缺少原型声明等）应导致编译报错，
 *             这些片段放在 #if 0 中，保证本文件本身仍可编译运行。
 *
 * 覆盖段落：
 *   [1] 概要：声明 int wcscmp(const wchar_t *, const wchar_t *);
 *   [2] 描述：比较 s1 与 s2 所指向的宽字符串
 *   [3] 返回值：>0 / ==0 / <0 分别对应 s1 大于 / 等于 / 小于 s2
 */

#include <wchar.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：验证函数原型可用，返回类型为 int，参数为 const wchar_t * */
static int (*fp)(const wchar_t *, const wchar_t *) = wcscmp;

int main(void)
{
    /* [1] 概要：函数可被正常调用，参数为宽字符串字面量 */
    const wchar_t *a = L"abc";
    const wchar_t *b = L"abd";
    const wchar_t *c = L"abc";
    const wchar_t *empty1 = L"";
    const wchar_t *empty2 = L"";

    /* [2] 描述 + [3] 返回值：s1 小于 s2 时返回负值 */
    int r1 = wcscmp(a, b);
    assert(r1 < 0);

    /* [2] 描述 + [3] 返回值：s1 大于 s2 时返回正值 */
    int r2 = wcscmp(b, a);
    assert(r2 > 0);

    /* [2] 描述 + [3] 返回值：s1 等于 s2 时返回 0 */
    int r3 = wcscmp(a, c);
    assert(r3 == 0);

    /* [3] 返回值：两个空宽字符串相等，返回 0 */
    int r4 = wcscmp(empty1, empty2);
    assert(r4 == 0);

    /* [3] 返回值：空串小于非空串，返回负值 */
    int r5 = wcscmp(empty1, a);
    assert(r5 < 0);

    /* [3] 返回值：非空串大于空串，返回正值 */
    int r6 = wcscmp(a, empty1);
    assert(r6 > 0);

    /* [2] 描述：比较基于宽字符的编码值（前缀相同，比较后续字符） */
    const wchar_t *p = L"hello";
    const wchar_t *q = L"help";
    int r7 = wcscmp(p, q);
    assert(r7 < 0);   /* 'l' < 'p' */

    /* [2] 描述：前缀关系——短串小于以其为前缀的长串 */
    const wchar_t *pre  = L"ab";
    const wchar_t *full = L"abc";
    int r8 = wcscmp(pre, full);
    assert(r8 < 0);
    int r9 = wcscmp(full, pre);
    assert(r9 > 0);

    /* [1] 概要：通过函数指针调用，验证原型签名一致 */
    int r10 = fp(a, b);
    assert(r10 < 0);

    /* [3] 返回值：仅要求符号，不要求具体数值；验证符号一致性 */
    assert((wcscmp(a, b) < 0) && (wcscmp(b, a) > 0) && (wcscmp(a, c) == 0));

    printf("C99 7.24.4.4.1 wcscmp: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与形参 const wchar_t * 兼容」：
 * 传入 char * 而非 wchar_t *，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 1 of 'wcscmp' from incompatible pointer type） */
{
    char *s1 = "abc";
    char *s2 = "abd";
    wcscmp(s1, s2);
}

/* 违反约束「实参类型必须与形参 const wchar_t * 兼容」：
 * 传入 int 而非指针，gcc -std=c99 应报错
 * （passing argument 1 of 'wcscmp' makes pointer from integer without a cast） */
{
    wcscmp(1, 2);
}

/* 违反约束「函数调用实参个数必须与原型一致」：
 * 只传一个参数，gcc -std=c99 应报错
 * （too few arguments to function 'wcscmp'） */
{
    const wchar_t *s = L"abc";
    wcscmp(s);
}

/* 违反约束「函数调用实参个数必须与原型一致」：
 * 传三个参数，gcc -std=c99 应报错
 * （too many arguments to function 'wcscmp'） */
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abd";
    wcscmp(s1, s2, s1);
}

/* 违反约束「函数返回值不可作为左值被赋值」：
 * wcscmp 返回 int（非左值），对其赋值应报错
 * （lvalue required as left operand of assignment） */
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abd";
    wcscmp(s1, s2) = 0;
}

/* 违反约束「函数返回值不可取地址」：
 * 对非左值取地址应报错
 * （lvalue required as unary '&' operand） */
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abd";
    int *p = &wcscmp(s1, s2);
    (void)p;
}

/* 违反约束「函数返回值不可自增/自减」：
 * 对非左值使用 ++ 应报错
 * （lvalue required as increment operand） */
{
    const wchar_t *s1 = L"abc";
    const wchar_t *s2 = L"abd";
    wcscmp(s1, s2)++;
}

#endif /* 负向测试结束 */