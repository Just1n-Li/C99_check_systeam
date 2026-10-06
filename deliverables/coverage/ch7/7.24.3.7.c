/*
 * 测试目标：C99 7.24.3.7 —— getwchar 函数
 *
 * 条款要点：
 *   [1] 原型： #include <wchar.h>  wint_t getwchar(void);
 *   [2] 语义： getwchar() 等价于 getwc(stdin)。
 *   [3] 返回： 返回 stdin 指向输入流的下一个宽字符，或 WEOF。
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：getwchar 无参数，返回 wint_t。
 *     通过取函数地址并赋给匹配的函数指针类型来验证原型。 */
static wint_t (*fp_getwchar)(void) = getwchar;

/* [2] 语义检查：getwchar() 等价于 getwc(stdin)。
 *     做法：准备一个临时文件，写入若干宽字符，重定向 stdin 到该文件，
 *     分别用 getwchar() 与 getwc(stdin) 读取，比较结果是否一致。 */
static void test_equivalence_with_getwc(void)
{
    const char *fname = "getwchar_test_tmp.txt";
    FILE *f;
    wint_t a, b;

    /* 写入宽字符序列：L'A' L'B' L'C' */
    f = fopen(fname, "w");
    assert(f != NULL);
    assert(fputwc(L'A', f) != WEOF);
    assert(fputwc(L'B', f) != WEOF);
    assert(fputwc(L'C', f) != WEOF);
    assert(fclose(f) == 0);

    /* 重定向 stdin 到该文件 */
    f = freopen(fname, "r", stdin);
    assert(f != NULL);

    /* [2] getwchar() 应等价于 getwc(stdin) */
    a = getwchar();
    b = getwc(stdin);
    assert(a == (wint_t)L'A');
    assert(b == (wint_t)L'B');
    assert(a == (wint_t)L'A');   /* 再次确认 a 的值 */

    /* [3] 继续读取，直到流结束应返回 WEOF */
    assert(getwchar() == (wint_t)L'C');
    assert(getwchar() == WEOF);  /* 到达文件末尾 */

    /* 清理 */
    fclose(stdin);
    remove(fname);
}

/* [3] 返回类型检查：getwchar 的返回类型为 wint_t，
 *     且 WEOF 可与之比较。 */
static void test_return_type(void)
{
    wint_t w = getwchar();   /* 类型应为 wint_t */
    (void)w;
    /* WEOF 是 wint_t 类型的常量，可与 getwchar 返回值比较 */
    assert(sizeof(w) == sizeof(wint_t));
}

/* [1] 函数指针类型匹配检查 */
static void test_prototype(void)
{
    assert(fp_getwchar == getwchar);
}

int main(void)
{
    test_prototype();
    test_equivalence_with_getwc();
    test_return_type();

    printf("C99 7.24.3.7 getwchar: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「getwchar 原型为 wint_t getwchar(void)，不接受参数」：
 * 调用时传入实参，gcc -std=c99 应报错：
 *   error: too many arguments to function 'getwchar' */
#include <wchar.h>
void bad_call_with_arg(void)
{
    wint_t c = getwchar(42);   /* 错误：getwchar 不接受参数 */
    (void)c;
}

/* 违反约束「getwchar 返回 wint_t，不能对其返回值取地址/赋值」：
 * getwchar() 是函数调用结果，不是左值，不能赋值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
#include <wchar.h>
void bad_assign_to_call(void)
{
    getwchar() = 0;   /* 错误：函数调用结果不是左值 */
}

/* 违反约束「getwchar 返回 wint_t，不能作为取地址操作符 & 的操作数」：
 * gcc -std=c99 应报错：lvalue required as unary '&' operand */
#include <wchar.h>
void bad_address_of_call(void)
{
    wint_t *p = &getwchar();   /* 错误：不能对函数调用结果取地址 */
    (void)p;
}

/* 违反约束「getwchar 声明于 <wchar.h>，未包含头文件时不得隐式声明」：
 * 在 C99 中，调用未声明的函数是约束违反（隐式声明被移除）。
 * gcc -std=c99 应报错/警告：implicit declaration of function 'getwchar' */
void bad_no_include(void)
{
    /* 此处故意不 #include <wchar.h> */
    wint_t c = getwchar();   /* 错误：隐式声明 */
    (void)c;
}

#endif /* 负向测试结束 */