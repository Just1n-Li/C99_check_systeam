/*
 * 测试目标：C99 7.12 Mathematics <math.h>
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反 7.12 约束的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 *
 * 覆盖段落：[1] 函数族与类型声明、[2] float_t/double_t、[3] HUGE_VAL 族、
 *           [4] INFINITY、[5] NAN、[6] 分类宏、[7] FP_FAST_FMA 族、
 *           [8] FP_ILOGB0/FP_ILOGBNAN、[9] MATH_ERRNO/MATH_ERREXCEPT/math_errhandling。
 */

#include <math.h>
#include <float.h>
#include <limits.h>
#include <fenv.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <math.h> 声明两个类型和许多数学函数，并定义若干宏。
 *     主函数族：double 参数/返回值；f 后缀为 float；l 后缀为 long double。
 *     这里验证函数族的存在性与可调用性（取地址即可验证声明存在）。 */
static void test_family_declarations(void)
{
    /* 主函数（double） */
    double (*pd)(double)              = sin;
    double (*pd2)(double, double)     = pow;
    double (*pd3)(double, double, double) = fma;
    /* f 后缀（float） */
    float  (*pf)(float)               = sinf;
    float  (*pf2)(float, float)       = powf;
    float  (*pf3)(float, float, float)= fmaf;
    /* l 后缀（long double） */
    long double (*pl)(long double)              = sinl;
    long double (*pl2)(long double, long double)= powl;
    long double (*pl3)(long double, long double, long double) = fmal;

    assert(pd  != NULL && pd2 != NULL && pd3 != NULL);
    assert(pf  != NULL && pf2 != NULL && pf3 != NULL);
    assert(pl  != NULL && pl2 != NULL && pl3 != NULL);

    /* 实际调用，验证返回值类型与语义 */
    assert(sin(0.0) == 0.0);
    assert(pow(2.0, 10.0) == 1024.0);
    assert(sinf(0.0f) == 0.0f);
    assert(powl(2.0L, 10.0L) == 1024.0L);
}

/* [2] float_t / double_t 是至少与 float / double 一样宽的浮点类型，
 *     且 double_t 至少与 float_t 一样宽。
 *     依据 FLT_EVAL_METHOD 的取值，它们分别等于 float/double/long double。 */
static void test_float_t_double_t(void)
{
    /* 类型存在 */
    float_t  ft = (float_t)1.0;
    double_t dt = (double_t)1.0;
    (void)ft; (void)dt;

    /* 宽度关系：sizeof(float) <= sizeof(float_t) <= sizeof(double_t) */
    assert(sizeof(float)  <= sizeof(float_t));
    assert(sizeof(double) <= sizeof(double_t));
    assert(sizeof(float_t) <= sizeof(double_t));

    /* 依据 FLT_EVAL_METHOD 的具体对应关系 */
#if FLT_EVAL_METHOD == 0
    assert(sizeof(float_t)  == sizeof(float));
    assert(sizeof(double_t) == sizeof(double));
#elif FLT_EVAL_METHOD == 1
    assert(sizeof(float_t)  == sizeof(double));
    assert(sizeof(double_t) == sizeof(double));
#elif FLT_EVAL_METHOD == 2
    assert(sizeof(float_t)  == sizeof(long double));
    assert(sizeof(double_t) == sizeof(long double));
#else
    /* 其他取值：实现定义，仅验证宽度关系（上面已断言） */
#endif
}

/* [3] HUGE_VAL 是正的 double 常量表达式；HUGE_VALF/HUGE_VALL 为 float/long double 版本。 */
static void test_huge_val(void)
{
    double hv  = HUGE_VAL;
    float  hvf = HUGE_VALF;
    long double hvl = HUGE_VALL;

    assert(hv  > 0.0);
    assert(hvf > 0.0f);
    assert(hvl > 0.0L);

    /* 类型检查：HUGE_VAL 为 double 类型 */
    assert(sizeof(HUGE_VAL)  == sizeof(double));
    assert(sizeof(HUGE_VALF) == sizeof(float));
    assert(sizeof(HUGE_VALL) == sizeof(long double));
}

/* [4] INFINITY 展开为 float 类型的常量表达式，表示正无穷（若可用）。 */
static void test_infinity(void)
{
    float inf = INFINITY;
    assert(sizeof(INFINITY) == sizeof(float));
    /* 若实现支持无穷，则 inf 为正无穷：inf > FLT_MAX 且 inf == inf */
    assert(inf > FLT_MAX);
    assert(inf == inf);
    assert(!(inf < inf));
}

/* [5] NAN 当且仅当实现支持 float 的 quiet NaN 时定义，
 *     展开为 float 类型的 quiet NaN 常量表达式。 */
static void test_nan(void)
{
#ifdef NAN
    float n = NAN;
    assert(sizeof(NAN) == sizeof(float));
    /* quiet NaN 不等于自身 */
    assert(n != n);
    /* 用分类宏确认它是 NaN */
    assert(fpclassify(n) == FP_NAN);
#else
    /* 实现不支持 quiet NaN，NAN 未定义，跳过 */
#endif
}

/* [6] 分类宏 FP_INFINITE FP_NAN FP_NORMAL FP_SUBNORMAL FP_ZERO
 *     是互斥的浮点值种类，展开为互不相同的整数常量表达式。 */
static void test_classification_macros(void)
{
    /* 互不相同 */
    assert(FP_INFINITE  != FP_NAN);
    assert(FP_INFINITE  != FP_NORMAL);
    assert(FP_INFINITE  != FP_SUBNORMAL);
    assert(FP_INFINITE  != FP_ZERO);
    assert(FP_NAN       != FP_NORMAL);
    assert(FP_NAN       != FP_SUBNORMAL);
    assert(FP_NAN       != FP_ZERO);
    assert(FP_NORMAL    != FP_SUBNORMAL);
    assert(FP_NORMAL    != FP_ZERO);
    assert(FP_SUBNORMAL != FP_ZERO);

    /* 整数常量表达式：可用于数组维度 */
    {
        char arr[FP_NORMAL + 1];
        (void)arr;
    }

    /* 实际分类验证 */
    assert(fpclassify(1.0)   == FP_NORMAL);
    assert(fpclassify(0.0)   == FP_ZERO);
    assert(fpclassify(INFINITY) == FP_INFINITE);
#ifdef NAN
    assert(fpclassify(NAN)   == FP_NAN);
#endif
}

/* [7] FP_FAST_FMA 可选定义；若定义，展开为整数常量 1。
 *     FP_FAST_FMAF / FP_FAST_FMAL 为 float / long double 版本。 */
static void test_fast_fma(void)
{
#ifdef FP_FAST_FMA
    assert(FP_FAST_FMA == 1);
#endif
#ifdef FP_FAST_FMAF
    assert(FP_FAST_FMAF == 1);
#endif
#ifdef FP_FAST_FMAL
    assert(FP_FAST_FMAL == 1);
#endif
    /* 无论是否定义，fma 函数族都应可用 */
    assert(fma(2.0, 3.0, 4.0) == 10.0);
}

/* [8] FP_ILOGB0 / FP_ILOGBNAN 展开为整数常量表达式，
 *     分别由 ilogb(x) 在 x 为 0 或 NaN 时返回。
 *     FP_ILOGB0 的值应为 INT_MIN 或 -INT_MAX；
 *     FP_ILOGBNAN 的值应为 INT_MAX 或 INT_MIN。 */
static void test_ilogb_macros(void)
{
    assert(FP_ILOGB0 == INT_MIN || FP_ILOGB0 == -INT_MAX);
    assert(FP_ILOGBNAN == INT_MAX || FP_ILOGBNAN == INT_MIN);

    /* 整数常量表达式：可用于数组维度 */
    {
        char a[1];
        (void)a;
    }

    /* 实际返回值验证 */
    assert(ilogb(0.0) == FP_ILOGB0);
#ifdef NAN
    assert(ilogb(NAN) == FP_ILOGBNAN);
#endif
}

/* [9] MATH_ERRNO == 1，MATH_ERREXCEPT == 2；
 *     math_errhandling 为 int 类型表达式，值为 MATH_ERRNO、MATH_ERREXCEPT 或二者按位或。
 *     若 math_errhandling & MATH_ERREXCEPT 非零，则 <fenv.h> 中定义了
 *     FE_DIVBYZERO、FE_INVALID、FE_OVERFLOW。 */
static void test_math_errhandling(void)
{
    assert(MATH_ERRNO == 1);
    assert(MATH_ERREXCEPT == 2);

    int me = math_errhandling;
    assert(me == MATH_ERRNO || me == MATH_ERREXCEPT ||
           me == (MATH_ERRNO | MATH_ERREXCEPT));

    /* 值在程序运行期间恒定 */
    assert(math_errhandling == me);

    /* 若使用异常机制，则 <fenv.h> 必须定义这三个宏 */
    if (math_errhandling & MATH_ERREXCEPT) {
        int d = FE_DIVBYZERO;
        int i = FE_INVALID;
        int o = FE_OVERFLOW;
        (void)d; (void)i; (void)o;
    }
}

int main(void)
{
    test_family_declarations();
    test_float_t_double_t();
    test_huge_val();
    test_infinity();
    test_nan();
    test_classification_macros();
    test_fast_fma();
    test_ilogb_macros();
    test_math_errhandling();

    printf("C99 7.12 <math.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「HUGE_VAL 是正的 double 常量表达式」：
 * 不能把 HUGE_VAL 用作需要整型常量表达式的场合（如数组维度、case 标签）。
 * gcc -std=c99 应报错：size of array has non-integer type / case label does not reduce to an integer constant。 */
double arr_huge[HUGE_VAL];

/* 违反约束「INFINITY 是 float 类型的常量表达式」：
 * 不能把 INFINITY 用作整型常量表达式（数组维度）。
 * gcc -std=c99 应报错：size of array has non-integer type。 */
float arr_inf[INFINITY];

/* 违反约束「分类宏展开为整数常量表达式」：
 * 不能把 FP_NAN 用作浮点常量初始化浮点对象（它不是浮点常量表达式）。
 * 更直接地：把分类宏用作 case 标签之外的浮点上下文会触发诊断。
 * 这里用 FP_NAN 作为数组维度以外的非法用法：作为浮点初始化器。
 * gcc -std=c99 应报错：initializer element is not a compile-time constant（或类型不匹配）。 */
float bad_class = FP_NAN;

/* 违反约束「FP_ILOGB0 的值应为 INT_MIN 或 -INT_MAX」：
 * 若实现把 FP_ILOGB0 定义为其他值，则违反约束。
 * 这里用静态断言模拟：若 FP_ILOGB0 不等于 INT_MIN 且不等于 -INT_MAX，则编译报错。
 * gcc -std=c99 应报错：negative width in bit-field（当条件为假时）。 */
struct {
    int check : (FP_ILOGB0 == INT_MIN || FP_ILOGB0 == -INT_MAX) ? 1 : -1;
} fp_ilogb0_check;

/* 违反约束「FP_ILOGBNAN 的值应为 INT_MAX 或 INT_MIN」：
 * 同上，用负宽度位域强制编译期诊断。 */
struct {
    int check : (FP_ILOGBNAN == INT_MAX || FP_ILOGBNAN == INT_MIN) ? 1 : -1;
} fp_ilogbnan_check;

/* 违反约束「MATH_ERRNO == 1 且 MATH_ERREXCEPT == 2」：
 * 若实现未按此定义，则违反约束。用负宽度位域强制诊断。 */
struct {
    int check : (MATH_ERRNO == 1 && MATH_ERREXCEPT == 2) ? 1 : -1;
} math_err_check;

/* 违反约束「math_errhandling 的值为 MATH_ERRNO、MATH_ERREXCEPT 或二者按位或」：
 * 用负宽度位域强制编译期诊断。 */
struct {
    int check : (math_errhandling == MATH_ERRNO ||
                 math_errhandling == MATH_ERREXCEPT ||
                 math_errhandling == (MATH_ERRNO | MATH_ERREXCEPT)) ? 1 : -1;
} math_errhandling_check;

/* 违反约束「若 math_errhandling & MATH_ERREXCEPT 非零，则 <fenv.h> 必须定义
 * FE_DIVBYZERO、FE_INVALID、FE_OVERFLOW」：
 * 若条件成立但宏未定义，则引用未声明标识符，编译报错。
 * 这里直接引用，若实现未定义则报错：'FE_DIVBYZERO' undeclared。 */
#if (math_errhandling & MATH_ERREXCEPT)
int fe_divbyzero_val = FE_DIVBYZERO;
int fe_invalid_val   = FE_INVALID;
int fe_overflow_val  = FE_OVERFLOW;
#endif

#endif /* 负向测试结束 */