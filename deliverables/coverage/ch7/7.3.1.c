/*
 * 测试 C99 7.3.1 <complex.h> Introduction
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，违反约束的片段应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：
 *   [1] <complex.h> 定义宏并声明支持复数运算的函数；每个 synopsis 指定一族函数
 *       （主函数 + f/l 后缀对应函数）。
 *   [2] complex 展开为 _Complex；_Complex_I 展开为 const float _Complex 类型的
 *       常量表达式，值为虚数单位。
 *   [3] imaginary 与 _Imaginary_I 当且仅当实现支持虚数类型时定义；若定义，
 *       展开为 _Imaginary 和 const float _Imaginary 类型的常量表达式。
 *   [4] I 展开为 _Imaginary_I 或 _Complex_I；若 _Imaginary_I 未定义，则 I 必须
 *       展开为 _Complex_I。
 *   [5] 尽管有 7.1.3 的规定，程序可以 undef 并（可能）重新定义宏 complex、
 *       imaginary 和 I。
 */

#include <complex.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <complex.h> 定义宏并声明函数族：主函数 + f/l 后缀对应函数。
 *     这里验证 double complex / float complex / long double complex 三个版本
 *     的函数均被声明且可调用。 */
static void test_family_declarations(void)
{
    double complex zd = 1.0 + 2.0 * I;
    float  complex zf = 1.0f + 2.0f * I;
    long double complex zl = 1.0L + 2.0L * I;

    /* cabs / cabsf / cabsl 属于同一族函数 */
    double ad = cabs(zd);
    float  af = cabsf(zf);
    long double al = cabsl(zl);

    assert(fabs(ad - sqrt(5.0)) < 1e-12);
    assert(fabs((double)af - sqrt(5.0)) < 1e-5);
    assert(fabsl(al - sqrtl(5.0L)) < 1e-15L);

    /* creal / crealf / creall 同族 */
    assert(creal(zd) == 1.0);
    assert(crealf(zf) == 1.0f);
    assert(creall(zl) == 1.0L);

    /* cimag / cimagf / cimagl 同族 */
    assert(cimag(zd) == 2.0);
    assert(cimagf(zf) == 2.0f);
    assert(cimagl(zl) == 2.0L);
}

/* [2] complex 展开为 _Complex；_Complex_I 展开为 const float _Complex 类型的
 *     常量表达式，值为虚数单位。 */
static void test_complex_macro(void)
{
    /* complex 展开为 _Complex：两者应可互换使用 */
    complex double a = 3.0 + 4.0 * I;
    _Complex double b = 3.0 + 4.0 * I;
    assert(creal(a) == creal(b));
    assert(cimag(a) == cimag(b));

    /* _Complex_I 是常量表达式，可用于静态初始化 */
    static const double complex ci = _Complex_I;
    assert(creal(ci) == 0.0);
    assert(cimag(ci) == 1.0);

    /* _Complex_I 的类型为 const float _Complex：其值作为 float 复数虚部为 1 */
    float complex fc = _Complex_I;
    assert(crealf(fc) == 0.0f);
    assert(cimagf(fc) == 1.0f);

    /* 虚数单位性质：i*i == -1 */
    double complex i2 = _Complex_I * _Complex_I;
    assert(fabs(creal(i2) + 1.0) < 1e-12);
    assert(fabs(cimag(i2)) < 1e-12);
}

/* [3] imaginary 与 _Imaginary_I 当且仅当实现支持虚数类型时定义。
 *     若实现不支持虚数类型，则这两个宏未定义——此时本测试跳过。
 *     若定义，则 imaginary 展开为 _Imaginary，_Imaginary_I 展开为
 *     const float _Imaginary 类型的常量表达式，值为虚数单位。 */
static void test_imaginary_macros(void)
{
#ifdef _Imaginary_I
    /* 实现支持虚数类型：imaginary 展开为 _Imaginary */
    imaginary float if1 = _Imaginary_I;
    _Imaginary float if2 = _Imaginary_I;
    (void)if1;
    (void)if2;

    /* _Imaginary_I 是常量表达式，可用于静态初始化 */
    static const float _Imaginary ii = _Imaginary_I;
    (void)ii;

    /* 虚数单位性质：i*i == -1（在支持虚数类型的实现上） */
    float _Imaginary sq = _Imaginary_I * _Imaginary_I;
    assert(sq == -1.0f);
#else
    /* 实现不支持虚数类型：imaginary 与 _Imaginary_I 均未定义。
     * 这里仅验证它们确实未定义（通过 #ifdef 分支到达此处）。 */
    assert(1);
#endif
}

/* [4] I 展开为 _Imaginary_I 或 _Complex_I；若 _Imaginary_I 未定义，
 *     则 I 必须展开为 _Complex_I。 */
static void test_I_macro(void)
{
    /* I 必须已定义 */
    double complex z = I;
    assert(creal(z) == 0.0);
    assert(cimag(z) == 1.0);

#ifdef _Imaginary_I
    /* 若 _Imaginary_I 已定义，I 可展开为 _Imaginary_I 或 _Complex_I，
     * 两者值均为虚数单位，故 I 的虚部为 1、实部为 0。 */
    assert(cimag(z) == 1.0);
#else
    /* 若 _Imaginary_I 未定义，I 必须展开为 _Complex_I。
     * 通过比较 I 与 _Complex_I 的值来验证。 */
    double complex ci = _Complex_I;
    assert(creal(z) == creal(ci));
    assert(cimag(z) == cimag(ci));
#endif
}

/* [5] 尽管有 7.1.3 的规定，程序可以 undef 并（可能）重新定义宏
 *     complex、imaginary 和 I。 */
static void test_undef_redefine(void)
{
    /* 先 undef complex 并重新定义 */
#undef complex
#define complex _Complex
    complex double a = 1.0 + 1.0 * I;
    assert(creal(a) == 1.0 && cimag(a) == 1.0);

    /* undef I 并重新定义 */
#undef I
#define I _Complex_I
    double complex b = 2.0 + 3.0 * I;
    assert(creal(b) == 2.0 && cimag(b) == 3.0);

    /* undef imaginary（无论是否定义过，undef 一个未定义的宏是合法的） */
#undef imaginary
#ifdef _Imaginary_I
#undef _Imaginary_I
#endif
    /* 重新定义 imaginary 为 _Imaginary（仅当实现支持虚数类型时可用） */
#ifdef _Imaginary_I
#define imaginary _Imaginary
#endif

    /* 恢复标准宏，避免影响后续代码 */
#undef complex
#define complex _Complex
#undef I
#define I _Complex_I
}

int main(void)
{
    test_family_declarations();  /* [1] */
    test_complex_macro();        /* [2] */
    test_imaginary_macros();     /* [3] */
    test_I_macro();              /* [4] */
    test_undef_redefine();       /* [5] */

    printf("C99 7.3.1 <complex.h> Introduction: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「_Complex_I 是常量表达式」：将其用于需要整型常量表达式的场合
 * （如数组维度、case 标签、位域宽度）应编译报错。
 * 期望：gcc -std=c99 报错，如 "size of array has non-integer type" 或
 *       "case label does not reduce to an integer constant"。 */
double complex arr[_Complex_I];   /* 错误：数组维度不是整型常量表达式 */

/* 违反约束「_Complex_I 的类型为 const float _Complex」：将其赋给不兼容的
 * 指针类型应编译报错（类型不匹配）。
 * 期望：gcc -std=c99 报错，如 "incompatible types when initializing type
 *       'int *' using type 'float _Complex'"。 */
int *p = _Complex_I;              /* 错误：类型不兼容 */

/* 违反约束「complex 展开为 _Complex」：将 complex 用作标识符（变量名）会与
 * 宏展开冲突，导致语法错误。
 * 期望：gcc -std=c99 报错，如 "expected identifier" 或类似语法错误。 */
int complex = 5;                  /* 错误：complex 是宏，展开后语法错误 */

/* 违反约束「I 展开为 _Imaginary_I 或 _Complex_I」：将 I 用作标识符会与宏
 * 展开冲突，导致语法错误。
 * 期望：gcc -std=c99 报错，如 "expected identifier" 或类似语法错误。 */
int I = 5;                        /* 错误：I 是宏，展开后语法错误 */

/* 违反约束「imaginary 展开为 _Imaginary」：将 imaginary 用作标识符会与宏
 * 展开冲突（仅当实现支持虚数类型、imaginary 已定义时）。
 * 期望：gcc -std=c99 报错，如 "expected identifier" 或类似语法错误。 */
int imaginary = 5;                /* 错误：imaginary 是宏，展开后语法错误 */

/* 违反约束「_Complex_I 是 const 限定的」：试图修改 _Complex_I 的值应编译报错。
 * 期望：gcc -std=c99 报错，如 "assignment of read-only variable" 或
 *       "lvalue required as left operand of assignment"。 */
void modify_const_ci(void)
{
    _Complex_I = 1.0f;            /* 错误：_Complex_I 是常量表达式，不可赋值 */
}

/* 违反约束「_Imaginary_I 是 const 限定的」（仅当实现支持虚数类型时）：
 * 试图修改 _Imaginary_I 的值应编译报错。
 * 期望：gcc -std=c99 报错，如 "assignment of read-only variable"。 */
void modify_const_ii(void)
{
    _Imaginary_I = 1.0f;          /* 错误：_Imaginary_I 是常量表达式，不可赋值 */
}

#endif /* 负向测试结束 */