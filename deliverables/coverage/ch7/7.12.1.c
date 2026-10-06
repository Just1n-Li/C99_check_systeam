/*
 * 测试目标：C99 7.12.1 Treatment of error conditions
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 说明：7.12.1 主要规定 <math.h> 函数在定义域错误(domain error)、
 *       值域错误(range error)、上溢(overflow)、下溢(underflow)时的行为，
 *       以及 math_errhandling / errno / MATH_ERRNO / MATH_ERREXCEPT 的语义。
 *       本测试通过检查宏存在性、errno 设置、HUGE_VAL 系列返回值等可观察行为，
 *       验证实现是否符合条款。注意：条款允许实现定义的行为（如 domain error
 *       的返回值、下溢时 errno 是否置 ERANGE），因此对这些部分只做“合法范围”
 *       检查，不做精确值断言。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <errno.h>
#include <float.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 每个 <math.h> 函数对所有可表示输入都有定义的行为，
     *     且如同单一操作执行，不产生外部可见的异常条件。
     *     这里验证：math_errhandling 宏存在，且其值由 MATH_ERRNO / MATH_ERREXCEPT 组成。 */
    {
        int me = math_errhandling;
        /* math_errhandling 必须是 MATH_ERRNO 与 MATH_ERREXCEPT 的按位组合（可能为 0）。 */
        assert((me & ~(MATH_ERRNO | MATH_ERREXCEPT)) == 0);
        /* 至少应定义这两个宏。 */
        assert(MATH_ERRNO == 1 || MATH_ERRNO == 2 || MATH_ERRNO != 0);
        assert(MATH_ERREXCEPT == 1 || MATH_ERREXCEPT == 2 || MATH_ERREXCEPT != 0);
    }

    /* [1] 正常输入：函数应正常返回，不产生 domain/range error。
     *     验证 sqrt(4.0) == 2.0，且 errno 不被置为 EDOM/ERANGE。 */
    {
        errno = 0;
        double r = sqrt(4.0);
        assert(r == 2.0);
        assert(errno != EDOM);
        assert(errno != ERANGE);
    }

    /* [2] domain error：输入参数超出数学函数定义域。
     *     sqrt(-1.0) 是典型 domain error。
     *     若 math_errhandling & MATH_ERRNO 非零，则 errno 应被置为 EDOM。
     *     返回值是实现定义的，这里只检查“若置了 errno 则必须是 EDOM”。 */
    {
        errno = 0;
        double r = sqrt(-1.0);
        (void)r; /* 返回值实现定义，不做精确断言 */
        if (math_errhandling & MATH_ERRNO) {
            assert(errno == EDOM);
        }
    }

    /* [2] 另一个 domain error：log(-1.0)。 */
    {
        errno = 0;
        double r = log(-1.0);
        (void)r;
        if (math_errhandling & MATH_ERRNO) {
            assert(errno == EDOM);
        }
    }

    /* [2] asin 的 domain 是 [-1,1]，asin(2.0) 是 domain error。 */
    {
        errno = 0;
        double r = asin(2.0);
        (void)r;
        if (math_errhandling & MATH_ERRNO) {
            assert(errno == EDOM);
        }
    }

    /* [3][4] range error / overflow：数学结果因量级过大无法表示。
     *     exp(1000.0) 会溢出，返回 HUGE_VAL（同号），
     *     若 math_errhandling & MATH_ERRNO 非零，errno 应置 ERANGE。 */
    {
        errno = 0;
        double r = exp(1000.0);
        assert(r == HUGE_VAL);          /* [4] 返回 HUGE_VAL，符号与正确值相同（正） */
        if (math_errhandling & MATH_ERRNO) {
            assert(errno == ERANGE);    /* [4] errno 置 ERANGE */
        }
    }

    /* [4] 精确无穷：log(0.0) 的数学结果是精确的 -infinity（来自有限参数）。
     *     返回 HUGE_VAL 且符号为负（即 -HUGE_VAL）。
     *     若 MATH_ERRNO 非零，errno 置 ERANGE。 */
    {
        errno = 0;
        double r = log(0.0);
        assert(r == -HUGE_VAL);         /* [4] 符号与正确值相同（负） */
        if (math_errhandling & MATH_ERRNO) {
            assert(errno == ERANGE);
        }
    }

    /* [4] 浮点溢出：HUGE_VALF 用于 float 返回类型。
     *     expf(1000.0f) 溢出，返回 HUGE_VALF。 */
    {
        errno = 0;
        float rf = expf(1000.0f);
        assert(rf == HUGE_VALF);
        if (math_errhandling & MATH_ERRNO) {
            assert(errno == ERANGE);
        }
    }

    /* [4] HUGE_VALL 用于 long double 返回类型。 */
    {
        errno = 0;
        long double rl = expl(1000.0L);
        assert(rl == HUGE_VALL);
        if (math_errhandling & MATH_ERRNO) {
            assert(errno == ERANGE);
        }
    }

    /* [5] underflow：数学结果量级太小无法表示。
     *     exp(-1000.0) 下溢。返回值是实现定义的，其量级不大于
     *     该类型最小规格化正数（DBL_MIN）。
     *     若 MATH_ERRNO 非零，errno 是否置 ERANGE 是实现定义的，不做断言。 */
    {
        errno = 0;
        double r = exp(-1000.0);
        /* 返回值量级 <= DBL_MIN（最小规格化正数）。 */
        assert(fabs(r) <= DBL_MIN);
        /* errno 是否置 ERANGE 是实现定义的，这里不断言。 */
    }

    /* [5] float 下溢：expf(-1000.0f)，返回值量级 <= FLT_MIN。 */
    {
        errno = 0;
        float rf = expf(-1000.0f);
        assert(fabsf(rf) <= FLT_MIN);
    }

    /* [5] long double 下溢：expl(-1000.0L)，返回值量级 <= LDBL_MIN。 */
    {
        errno = 0;
        long double rl = expl(-1000.0L);
        assert(fabsl(rl) <= LDBL_MIN);
    }

    /* [2] 若实现支持 MATH_ERREXCEPT，则 domain error 应引发 "invalid" 浮点异常。
     *     这里用 fenv 检查（若实现支持）。 */
    {
        if (math_errhandling & MATH_ERREXCEPT) {
            feclearexcept(FE_ALL_EXCEPT);
            volatile double r = sqrt(-1.0);
            (void)r;
            int raised = fetestexcept(FE_INVALID);
            assert(raised & FE_INVALID);
        }
    }

    /* [4] 若实现支持 MATH_ERREXCEPT，则精确无穷（log(0.0)）应引发
     *     "divide-by-zero" 浮点异常。 */
    {
        if (math_errhandling & MATH_ERREXCEPT) {
            feclearexcept(FE_ALL_EXCEPT);
            volatile double r = log(0.0);
            (void)r;
            int raised = fetestexcept(FE_DIVBYZERO);
            assert(raised & FE_DIVBYZERO);
        }
    }

    /* [4] 若实现支持 MATH_ERREXCEPT，则一般溢出（exp(1000.0)）应引发
     *     "overflow" 浮点异常。 */
    {
        if (math_errhandling & MATH_ERREXCEPT) {
            feclearexcept(FE_ALL_EXCEPT);
            volatile double r = exp(1000.0);
            (void)r;
            int raised = fetestexcept(FE_OVERFLOW);
            assert(raised & FE_OVERFLOW);
        }
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 说明：7.12.1 本身主要是“行为规定”，并未直接给出大量语法约束。
 * 下面列出与 <math.h> 使用相关的、违反 C99 约束的片段，
 * 期望编译器在 -std=c99 下报错。
 */

/* 违反约束「<math.h> 中函数参数必须为算术类型（或可转换类型）」：
 * 向 sqrt 传入结构体，参数类型不匹配，应编译报错。 */
struct S { int x; };
struct S s;
double d = sqrt(s);   /* 错误：sqrt 的参数不能是结构体类型 */

/* 违反约束「函数调用参数个数必须匹配原型」：
 * sqrt 原型接受 1 个参数，这里传 2 个，应编译报错。 */
double d2 = sqrt(4.0, 5.0);   /* 错误：参数个数过多 */

/* 违反约束「函数调用参数个数必须匹配原型」：
 * pow 原型接受 2 个参数，这里传 1 个，应编译报错。 */
double d3 = pow(2.0);   /* 错误：参数个数过少 */

/* 违反约束「赋值目标必须是可修改左值」：
 * 对函数返回值赋值，应编译报错。 */
sqrt(4.0) = 2.0;   /* 错误：函数调用结果不是左值 */

/* 违反约束「math_errhandling 等宏不可被赋值」：
 * 宏展开后不是可修改左值，赋值应编译报错。 */
math_errhandling = 0;   /* 错误：宏不是可修改左值 */

/* 违反约束「HUGE_VAL 等宏不可被赋值」：
 * 宏展开后不是可修改左值，赋值应编译报错。 */
HUGE_VAL = 1.0;   /* 错误：宏不是可修改左值 */

#endif