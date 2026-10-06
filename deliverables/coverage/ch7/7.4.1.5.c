/*
 * 测试 C99 7.4.1.5 —— isdigit 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 条款要点：
 *   [1] 头文件 <ctype.h>，原型 int isdigit(int c);
 *   [2] 测试十进制数字字符（'0'..'9'，见 5.2.1）。
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在且返回 int，参数为 int */
static int (*fp_isdigit)(int) = isdigit;

int main(void)
{
    /* [1] 通过函数指针调用，验证原型签名 int isdigit(int) */
    assert(fp_isdigit('5') != 0);

    /* [2] 所有十进制数字字符 '0'..'9' 必须被识别 */
    {
        int c;
        for (c = '0'; c <= '9'; ++c) {
            assert(isdigit(c) != 0);   /* 数字字符：非零 */
        }
    }

    /* [2] 非数字字符必须返回 0 */
    assert(isdigit('a') == 0);
    assert(isdigit('z') == 0);
    assert(isdigit('A') == 0);
    assert(isdigit('Z') == 0);
    assert(isdigit(' ') == 0);
    assert(isdigit('\t') == 0);
    assert(isdigit('\n') == 0);
    assert(isdigit('+') == 0);
    assert(isdigit('-') == 0);
    assert(isdigit('.') == 0);
    assert(isdigit('@') == 0);
    assert(isdigit('[') == 0);
    assert(isdigit('`') == 0);
    assert(isdigit('{') == 0);
    assert(isdigit('~') == 0);

    /* [2] 边界：'0'-1 与 '9'+1 不是数字字符 */
    assert(isdigit('/') == 0);   /* '/' == '0' - 1 */
    assert(isdigit(':') == 0);   /* ':' == '9' + 1 */

    /* [2] 数字字符的返回值只要求“非零”，不要求特定值 */
    assert(isdigit('0') != 0);
    assert(isdigit('9') != 0);

    /* [1] 参数为 int，可传入 EOF（负值），行为由实现定义但不应崩溃 */
    (void)isdigit(EOF);

    /* [1] 参数为 int，可传入任意 int 值（如 0..255 范围） */
    {
        int c;
        for (c = 0; c < 256; ++c) {
            int r = isdigit(c);
            /* 仅验证返回值语义：数字字符非零，其余为零 */
            if (c >= '0' && c <= '9') {
                assert(r != 0);
            } else {
                assert(r == 0);
            }
        }
    }

    printf("C99 7.4.1.5 isdigit: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束「isdigit 的原型为 int isdigit(int)」：
 * 用不兼容的指针类型初始化函数指针，gcc -std=c99 应报错
 * （incompatible pointer type / initialization from incompatible pointer type）。 */
int (*bad_fp)(double) = isdigit;

/* 违反约束「isdigit 的原型为 int isdigit(int)」：
 * 用不兼容的指针类型赋值，gcc -std=c99 应报错。 */
void bad_assign(void)
{
    int (*p)(char *) = isdigit;   /* 不兼容指针类型 */
    (void)p;
}

/* 违反约束「isdigit 的原型为 int isdigit(int)」：
 * 以不兼容类型调用（参数为结构体），gcc -std=c99 应报错。 */
struct S { int x; };
void bad_call(void)
{
    struct S s;
    (void)isdigit(s);   /* 参数类型不兼容 */
}

/* 违反约束「isdigit 的原型为 int isdigit(int)」：
 * 以不兼容类型调用（参数为指针），gcc -std=c99 应报错。 */
void bad_call_ptr(void)
{
    char *p = "x";
    (void)isdigit(p);   /* 参数类型不兼容 */
}

/* 违反约束「isdigit 的原型为 int isdigit(int)」：
 * 以不兼容类型调用（参数为 double），gcc -std=c99 应报错。 */
void bad_call_double(void)
{
    (void)isdigit(3.14);   /* 参数类型不兼容 */
}

#endif /* 负向测试结束 */