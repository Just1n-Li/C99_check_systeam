/*
 * 测试 C99 6.4.1 Keywords
 *
 * 条款要点：
 *   [1] 列出全部关键字（区分大小写）。
 *   [2] 这些 token 在翻译阶段 7、8 保留为关键字，不得作其他用途。
 *       _Imaginary 保留用于指定虚数类型。
 *
 * 预期行为：
 *   正向测试：使用全部关键字作为关键字（合法用法）的代码应能编译并运行通过。
 *   负向测试：把关键字当作普通标识符（变量名/函数名/成员名等）使用，
 *             违反 [2] 的约束，编译器必须报错。
 */

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <complex.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * 目标：证明这些 token 在合法位置被识别为关键字，程序语义正确。
 * ============================================================ */

/* [1] auto —— 存储类说明符 */
static int test_auto(void)
{
    auto int x = 41;      /* C99 中 auto 合法（块作用域） */
    return x + 1;
}

/* [1] break / continue / while / do / for / if / else / switch / case / default / goto */
static int test_control_flow(void)
{
    int sum = 0;
    int i;

    for (i = 0; i < 10; i++) {
        if (i == 3)
            continue;          /* continue */
        if (i == 8)
            break;             /* break */
        sum += i;
    }
    /* sum = 0+1+2+4+5+6+7 = 25 */

    i = 0;
    do {                       /* do */
        i++;
    } while (i < 5);           /* while */

    switch (i) {               /* switch */
    case 5:                    /* case */
        sum += 100;
        break;
    default:                   /* default */
        sum += 1000;
        break;
    }

    if (sum == 125)            /* if / else */
        goto ok;               /* goto */
    else
        return -1;
ok:
    return sum;
}

/* [1] enum / struct / union / typedef / sizeof */
typedef enum { RED, GREEN, BLUE } Color;   /* enum, typedef */

struct Point { int x, y; };                /* struct */

union U { int i; float f; };               /* union */

static int test_types(void)
{
    Color c = GREEN;
    struct Point p = { 3, 4 };
    union U u;
    u.i = 7;

    assert(c == GREEN);
    assert(p.x == 3 && p.y == 4);
    assert(u.i == 7);
    assert(sizeof(struct Point) == 2 * sizeof(int));  /* sizeof */
    return 0;
}

/* [1] const / volatile / restrict / register / static / extern / inline */
static const int CONST_VAL = 10;           /* const, static */
extern int extern_var;                     /* extern */
int extern_var = 20;

static inline int add(int a, int b)        /* inline, static */
{
    return a + b;
}

static int test_qualifiers(void)
{
    volatile int v = 5;                    /* volatile */
    register int r = 6;                    /* register */
    int arr[4] = { 1, 2, 3, 4 };
    int * restrict rp = arr;               /* restrict */

    assert(v == 5);
    assert(r == 6);
    assert(rp[0] == 1);
    assert(CONST_VAL == 10);
    assert(extern_var == 20);
    assert(add(2, 3) == 5);
    return 0;
}

/* [1] signed / unsigned / short / long / int / char / float / double / void / _Bool */
static _Bool test_bool(void)               /* _Bool */
{
    _Bool b = 1;
    return b;
}

static int test_basic_types(void)
{
    signed int si = -1;                    /* signed */
    unsigned int ui = 1u;                  /* unsigned */
    short s = 2;                           /* short */
    long l = 3L;                           /* long */
    int i = 4;                             /* int */
    char ch = 'A';                         /* char */
    float f = 1.5f;                        /* float */
    double d = 2.5;                        /* double */

    assert(si == -1);
    assert(ui == 1u);
    assert(s == 2);
    assert(l == 3L);
    assert(i == 4);
    assert(ch == 'A');
    assert(f == 1.5f);
    assert(d == 2.5);
    assert(test_bool() == 1);
    return 0;
}

/* [1] _Complex —— 复数类型 */
static int test_complex(void)
{
    double _Complex z = 1.0 + 2.0 * I;     /* _Complex */
    assert(creal(z) == 1.0);
    assert(cimag(z) == 2.0);
    return 0;
}

/* [1] return / void */
static void noop(void)                     /* void */
{
    return;                                /* return */
}

/* [1] 关键字区分大小写：Auto 不是关键字，是普通标识符 */
static int test_case_sensitive(void)
{
    int Auto = 99;      /* 合法：Auto 不是关键字 */
    int Int = 1;        /* 合法：Int 不是关键字 */
    int While = 2;      /* 合法：While 不是关键字 */
    assert(Auto == 99);
    assert(Int == 1);
    assert(While == 2);
    return 0;
}

int main(void)
{
    assert(test_auto() == 42);
    assert(test_control_flow() == 125);
    assert(test_types() == 0);
    assert(test_qualifiers() == 0);
    assert(test_basic_types() == 0);
    assert(test_complex() == 0);
    noop();
    assert(test_case_sensitive() == 0);

    printf("C99 6.4.1 Keywords: all positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 6.4.1 [2] 的约束，应编译报错
 * 约束：关键字 token 保留，不得用作其他用途（如标识符）。
 * ============================================================ */
#if 0

/* 违反 [2]：int 是关键字，不能作变量名。gcc -std=c99 应报错：
 *   error: expected identifier or '(' before 'int' 之类 */
int int = 5;

/* 违反 [2]：while 是关键字，不能作变量名 */
int while = 6;

/* 违反 [2]：return 是关键字，不能作变量名 */
int return = 7;

/* 违反 [2]：sizeof 是关键字，不能作变量名 */
int sizeof = 8;

/* 违反 [2]：struct 是关键字，不能作变量名 */
int struct = 9;

/* 违反 [2]：_Bool 是关键字，不能作变量名 */
int _Bool = 10;

/* 违反 [2]：_Complex 是关键字，不能作变量名 */
int _Complex = 11;

/* 违反 [2]：_Imaginary 是关键字（保留用于虚数类型），不能作变量名 */
int _Imaginary = 12;

/* 违反 [2]：关键字不能作函数名 */
void if(void) { }

/* 违反 [2]：关键字不能作结构体成员名 */
struct S1 { int for; };

/* 违反 [2]：关键字不能作枚举常量名 */
enum E1 { case };

/* 违反 [2]：关键字不能作 typedef 名 */
typedef int double;

/* 违反 [2]：关键字不能作标签名 */
void f(void) { goto switch; switch: ; }

/* 违反 [2]：关键字不能作宏名（宏名必须是标识符） */
#define char 1

/* 违反 [2]：关键字不能作形参名 */
int g(int void) { return 0; }

#endif