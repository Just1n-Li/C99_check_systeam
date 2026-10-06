/*
 * 测试 C99 6.2.5 Types
 *
 * 正向测试：验证条款 [1]-[20] 中可观察的语义（类型大小、范围、子范围、
 *           无符号回绕、浮点子集、复数表示、字符类型、枚举、派生类型等）。
 *           期望：编译通过，assert 全部成立。
 * 负向测试：验证条款中的约束（constraint）——例如 void 不能作为对象类型、
 *           不完整类型不能定义对象、非左值不能赋值等。
 *           期望：gcc -std=c99 编译报错（片段放在 #if 0 中，不影响本文件编译）。
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <stddef.h>
#include <string.h>

/* 用于 [13] 复数表示测试 */
#include <complex.h>

/* 用于 [16] 枚举 */
enum Color { RED, GREEN, BLUE };

/* 用于 [20] 派生类型 */
struct Point { int x, y; };
typedef int Arr5[5];

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 类型划分：对象类型、函数类型、不完整类型
     *     对象类型完整描述对象；不完整类型缺少大小信息。
     *     用 sizeof 验证对象类型是完整的。 */
    assert(sizeof(int) > 0);
    assert(sizeof(struct Point) > 0);

    /* [2] _Bool 至少能存 0 和 1 */
    {
        _Bool b0 = 0, b1 = 1;
        assert(b0 == 0);
        assert(b1 == 1);
        assert(sizeof(_Bool) >= 1);
    }

    /* [3] char 能存基本执行字符集；基本字符集成员存于 char 保证非负 */
    {
        char c = 'A';
        assert(c == 'A');
        assert(c >= 0);            /* 基本执行字符集成员非负 */
        char d = '0';
        assert(d >= 0);
    }

    /* [4] 五种标准有符号整数类型 */
    {
        signed char sc = -1;
        short int si = -1;
        int i = -1;
        long int li = -1;
        long long int lli = -1;
        assert(sc < 0 && si < 0 && i < 0 && li < 0 && lli < 0);
        /* 转换等级递增，范围应递增（[8]） */
        assert(sizeof(signed char) <= sizeof(short int));
        assert(sizeof(short int) <= sizeof(int));
        assert(sizeof(int) <= sizeof(long int));
        assert(sizeof(long int) <= sizeof(long long int));
    }

    /* [5] signed char 与 plain char 占用相同存储 */
    assert(sizeof(signed char) == sizeof(char));
    /* plain int 的自然大小足以容纳 INT_MIN..INT_MAX */
    assert(sizeof(int) * CHAR_BIT >= 16);
    assert(INT_MAX >= 32767);

    /* [6] 每个有符号整数类型有对应的无符号类型，占用相同存储、相同对齐 */
    {
        assert(sizeof(unsigned char) == sizeof(signed char));
        assert(sizeof(unsigned short) == sizeof(short int));
        assert(sizeof(unsigned int) == sizeof(int));
        assert(sizeof(unsigned long) == sizeof(long int));
        assert(sizeof(unsigned long long) == sizeof(long long int));
        assert(_Alignof(unsigned int) == _Alignof(int));
        assert(_Alignof(unsigned long long) == _Alignof(long long int));
    }

    /* [7] 标准整数类型 = 标准有符号 + 标准无符号（此处仅验证存在性） */
    {
        unsigned int u = 0u;
        int s = 0;
        assert(u == 0 && s == 0);
    }

    /* [8] 同符号、不同转换等级：小等级范围是大等级范围的子范围 */
    {
        assert(SCHAR_MIN >= INT_MIN && SCHAR_MAX <= INT_MAX);
        assert(SHRT_MIN >= INT_MIN && SHRT_MAX <= INT_MAX);
        assert(INT_MIN >= LONG_MIN && INT_MAX <= LONG_MAX);
        assert(LONG_MIN >= LLONG_MIN && LONG_MAX <= LLONG_MAX);
        /* 无符号同理 */
        assert(UCHAR_MAX <= UINT_MAX);
        assert(USHRT_MAX <= UINT_MAX);
        assert(UINT_MAX <= ULONG_MAX);
        assert(ULONG_MAX <= ULLONG_MAX);
    }

    /* [9] 有符号非负范围是对应无符号类型的子范围；
     *     无符号运算不会溢出，结果按模 2^N 回绕。 */
    {
        assert((unsigned int)INT_MAX <= UINT_MAX);
        /* 无符号回绕：UINT_MAX + 1 == 0 */
        unsigned int umax = UINT_MAX;
        unsigned int wrap = umax + 1u;
        assert(wrap == 0u);
        /* 0 - 1 == UINT_MAX */
        unsigned int zero = 0u;
        assert(zero - 1u == UINT_MAX);
        /* 相同值在有符号/无符号中表示相同（非负值） */
        int v = 12345;
        unsigned int uv = (unsigned int)v;
        assert(uv == 12345u);
    }

    /* [10] float 值集是 double 的子集，double 是 long double 的子集 */
    {
        assert(sizeof(float) <= sizeof(double));
        assert(sizeof(double) <= sizeof(long double));
        assert(FLT_MAX <= DBL_MAX);
        assert(DBL_MAX <= LDBL_MAX);
        assert(FLT_MIN >= DBL_MIN);
        assert(DBL_MIN >= LDBL_MIN);
        /* 值集子集：float 能表示的值 double 也能表示 */
        float f = 1.5f;
        double d = (double)f;
        assert((float)d == f);
    }

    /* [11] 三种复数类型 */
    {
        float _Complex fc = 1.0f + 2.0f * I;
        double _Complex dc = 1.0 + 2.0 * I;
        long double _Complex ldc = 1.0L + 2.0L * I;
        assert(crealf(fc) == 1.0f && cimagf(fc) == 2.0f);
        assert(creal(dc) == 1.0 && cimag(dc) == 2.0);
        assert(creall(ldc) == 1.0L && cimagl(ldc) == 2.0L);
    }

    /* [12] 每个浮点类型有对应实类型 */
    {
        /* 实浮点类型的对应实类型是自身 */
        assert(sizeof(float) == sizeof(float));
        /* 复数的对应实类型：去掉 _Complex */
        assert(sizeof(float _Complex) == 2 * sizeof(float));
        assert(sizeof(double _Complex) == 2 * sizeof(double));
        assert(sizeof(long double _Complex) == 2 * sizeof(long double));
    }

    /* [13] 复数类型与含两个对应实类型元素的数组有相同表示和对齐 */
    {
        assert(sizeof(double _Complex) == sizeof(double[2]));
        assert(_Alignof(double _Complex) == _Alignof(double[2]));
        /* 第一个元素是实部，第二个是虚部 */
        double _Complex z = 3.0 + 4.0 * I;
        double parts[2];
        memcpy(parts, &z, sizeof(z));
        assert(parts[0] == 3.0);
        assert(parts[1] == 4.0);
    }

    /* [14] 基本类型：char、有/无符号整数、浮点类型。
     *     即使表示相同，也是不同类型（用类型兼容性/指针类型区分）。 */
    {
        /* char 与 signed char 是不同类型（此处用 _Generic 验证） */
        int is_char = _Generic((char)0, char: 1, default: 0);
        int is_schar = _Generic((signed char)0, signed char: 1, default: 0);
        assert(is_char == 1);
        assert(is_schar == 1);
        /* 它们是不同类型：char 不匹配 signed char 分支 */
        int char_matches_schar = _Generic((char)0, signed char: 1, default: 0);
        assert(char_matches_schar == 0);
    }

    /* [15] char、signed char、unsigned char 是字符类型；
     *      char 与 signed char 或 unsigned char 有相同范围/表示/行为。 */
    {
        assert(sizeof(char) == sizeof(signed char));
        assert(sizeof(char) == sizeof(unsigned char));
        /* char 的范围等于 signed char 或 unsigned char 之一 */
        int char_is_signed = (CHAR_MIN < 0);
        if (char_is_signed) {
            assert(CHAR_MIN == SCHAR_MIN && CHAR_MAX == SCHAR_MAX);
        } else {
            assert(CHAR_MIN == 0 && CHAR_MAX == UCHAR_MAX);
        }
    }

    /* [16] 枚举：一组命名整型常量；每个不同枚举是不同类型 */
    {
        enum Color c = GREEN;
        assert(c == 1);
        assert(RED == 0 && GREEN == 1 && BLUE == 2);
        /* 不同枚举类型不同（用 _Generic 区分） */
        enum Color2 { X = 0 };
        int m = _Generic(c, enum Color: 1, default: 0);
        assert(m == 1);
        int m2 = _Generic((enum Color2){0}, enum Color: 1, default: 0);
        assert(m2 == 0);
    }

    /* [17] 整数类型 = char + 有/无符号整数 + 枚举；
     *     实类型 = 整数 + 实浮点。 */
    {
        /* 枚举属于整数类型：可参与整数运算 */
        enum Color c = RED;
        int n = c + 1;
        assert(n == 1);
        /* 实浮点属于实类型 */
        double d = 1.0;
        assert(d == 1.0);
    }

    /* [18] 算术类型 = 整数 + 浮点；类型域：实域 / 复域 */
    {
        /* 整数与浮点都是算术类型，可做算术运算 */
        int i = 3;
        double d = 2.0;
        double r = i + d;
        assert(r == 5.0);
        /* 复域 */
        double _Complex z = 1.0 + 1.0 * I;
        assert(creal(z) == 1.0);
    }

    /* [19] void 是空值集，是不完整类型，不能完成 */
    {
        /* void 是不完整类型：不能 sizeof(void)（见负向测试） */
        /* 但 void* 是完整对象指针类型 */
        void *p = NULL;
        assert(p == NULL);
        /* 函数返回 void 合法 */
        /* (无返回值函数调用) */
    }

    /* [20] 派生类型：数组、结构、联合、函数、指针等 */
    {
        /* 数组类型：连续分配的非空对象集合 */
        Arr5 a = {1, 2, 3, 4, 5};
        assert(sizeof(a) == 5 * sizeof(int));
        assert(a[0] == 1 && a[4] == 5);
        /* 数组元素类型为 int */
        assert(_Generic(a[0], int: 1, default: 0) == 1);

        /* 结构类型：成员序列 */
        struct Point pt = {3, 4};
        assert(pt.x == 3 && pt.y == 4);
        assert(sizeof(struct Point) >= 2 * sizeof(int));

        /* 联合类型 */
        union U { int i; float f; };
        union U u;
        u.i = 42;
        assert(u.i == 42);

        /* 函数类型 */
        int (*fp)(void) = NULL;
        assert(fp == NULL);

        /* 指针类型 */
        int x = 7;
        int *px = &x;
        assert(*px == 7);

        /* 不完整类型：struct 前向声明，不能定义对象（见负向测试） */
        struct Incomplete;   /* 声明为不完整类型 */
        struct Incomplete *pi = NULL;  /* 指向不完整类型的指针合法 */
        assert(pi == NULL);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反 [19]：void 是不完整类型，不能作为对象类型定义对象。
 * 期望：error: variable has incomplete type 'void' */
void v;

/* 违反 [19]：void 是不完整类型，sizeof(void) 非法。
 * 期望：error: invalid application of 'sizeof' to incomplete type 'void' */
int sz = sizeof(void);

/* 违反 [20]：不完整类型不能定义对象（大小未知）。
 * 期望：error: storage size of 'inc' isn't known */
struct Incomplete;
struct Incomplete inc;

/* 违反 [20]：不完整数组类型（元素类型不完整）不能定义对象。
 * 期望：error: array type has incomplete element type */
struct Incomplete arr[10];

/* 违反 [20]：函数类型不能作为对象类型定义对象。
 * 期望：error: function 'f' is initialized like a variable */
int f(void);
int f(void) { return 0; }

/* 违反 [20]：数组元素类型不能是函数类型。
 * 期望：error: declaration of 'fa' as array of functions */
int fa[3](void);

/* 违反 [20]：函数不能返回数组类型。
 * 期望：error: 'g' declared as function returning an array */
int g(void)[3];

/* 违反 [20]：函数不能返回函数类型。
 * 期望：error: 'h' declared as function returning a function */
int h(void)(void);

/* 违反 [2]/[14]：_Bool 是基本类型，不能对其取地址后赋给不兼容指针
 * （此处演示 _Bool 对象不能存任意大值——但这是语义非约束，故略）。
 * 改为：违反 [19] void 不能解引用。
 * 期望：error: 'void' is not a pointer-to-object type / dereferencing 'void *' */
void *vp;
int bad = *vp;

/* 违反 [20]：不能定义 void 类型的数组。
 * 期望：error: array has incomplete element type 'void' */
void va[5];

/* 违反 [20]：不能定义 void 类型的函数参数对象（除 (void) 表示无参）。
 * 期望：error: 'v' has void type */
void func(void v);

/* 违反 [20]：不能对不完整类型取 sizeof。
 * 期望：error: invalid application of 'sizeof' to incomplete type */
struct Incomplete;
int sz2 = sizeof(struct Incomplete);

/* 违反 [20]：不能对函数类型取 sizeof。
 * 期望：error: invalid application of 'sizeof' to a function type */
int sz3 = sizeof(int (void));

/* 违反 [20]：不能对函数类型赋值。
 * 期望：error: assignment to expression with function type */
int fn(void) { return 0; }
void assign_fn(void) { fn = fn; }

/* 违反 [20]：不能对数组类型赋值。
 * 期望：error: assignment to expression with array type */
void assign_arr(void) {
    int a[3], b[3];
    a = b;
}

/* 违反 [20]：不能对不完整类型解引用。
 * 期望：error: dereferencing pointer to incomplete type */
struct Incomplete;
void deref_inc(void) {
    struct Incomplete *p;
    struct Incomplete x = *p;
}

#endif