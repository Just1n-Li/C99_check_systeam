/*
 * 测试 C99 7.12.9.5 —— lrint / llrint 系列函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <math.h>
 *   [2] 按当前舍入方向舍入到最近整数；超出返回类型范围时结果未指定，
 *       可能发生 domain error 或 range error（UB/未指定，不作为负向测试）
 *   [3] 返回舍入后的整数值
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <fenv.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，若原型缺失或类型不符则编译失败 */
static long int        (*p_lrint)(double)            = lrint;
static long int        (*p_lrintf)(float)            = lrintf;
static long int        (*p_lrintl)(long double)      = lrintl;
static long long int   (*p_llrint)(double)           = llrint;
static long long int   (*p_llrintf)(float)           = llrintf;
static long long int   (*p_llrintl)(long double)     = llrintl;

int main(void)
{
    /* [1] 头文件 <math.h> 已包含，原型可见（上面已通过函数指针赋值验证） */
    (void)p_lrint; (void)p_lrintf; (void)p_lrintl;
    (void)p_llrint; (void)p_llrintf; (void)p_llrintl;

    /* [2][3] 默认舍入方向（FE_TONEAREST）下，舍入到最近整数 */
    assert(lrint(2.4)  == 2L);
    assert(lrint(2.5)  == 2L);   /* 最近偶数 */
    assert(lrint(3.5)  == 4L);   /* 最近偶数 */
    assert(lrint(-2.5) == -2L);
    assert(lrint(2.6)  == 3L);
    assert(lrint(-2.6) == -3L);
    assert(lrint(0.0)  == 0L);

    /* [2][3] float / long double 版本 */
    assert(lrintf(2.5f)   == 2L);
    assert(lrintf(3.5f)   == 4L);
    assert(lrintl(2.5L)   == 2L);
    assert(lrintl(3.5L)   == 4L);

    /* [2][3] llrint 系列 */
    assert(llrint(2.5)    == 2LL);
    assert(llrint(3.5)    == 4LL);
    assert(llrint(-2.5)   == -2LL);
    assert(llrintf(2.5f)  == 2LL);
    assert(llrintl(3.5L)  == 4LL);

    /* [2] 舍入方向影响结果：切换到 FE_DOWNWARD 后 2.9 -> 2 */
    {
        int saved = fegetround();
        if (fesetround(FE_DOWNWARD) == 0) {
            assert(lrint(2.9)  == 2L);
            assert(lrint(-2.1) == -3L);
            assert(llrint(2.9) == 2LL);
        }
        if (fesetround(FE_UPWARD) == 0) {
            assert(lrint(2.1)  == 3L);
            assert(lrint(-2.9) == -2L);
            assert(llrint(2.1) == 3LL);
        }
        if (fesetround(FE_TOWARDZERO) == 0) {
            assert(lrint(2.9)  == 2L);
            assert(lrint(-2.9) == -2L);
            assert(llrint(-2.9) == -2LL);
        }
        fesetround(saved);
    }

    /* [2][3] 大值仍在 long long 范围内，结果确定 */
    assert(llrint(1e15) == 1000000000000000LL);
    assert(llrint(-1e15) == -1000000000000000LL);

    /* [3] 返回值类型为 long int / long long int */
    {
        long int a = lrint(1.0);
        long long int b = llrint(1.0);
        assert(a == 1L);
        assert(b == 1LL);
    }

    printf("C99 7.12.9.5 lrint/llrint: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「lrint 的参数类型为 double」：传入结构体类型，gcc -std=c99 应报错
 * （实参与形参类型不兼容，且无隐式转换） */
struct S { int x; };
struct S s;
lrint(s);            /* error: incompatible type for argument 1 of 'lrint' */

/* 违反约束「llrint 的参数类型为 double」：传入指针类型，应报错 */
int *ip;
llrint(ip);          /* error: incompatible type for argument 1 of 'llrint' */

/* 违反约束「lrintf 的参数类型为 float」：传入结构体，应报错 */
lrintf(s);           /* error: incompatible type for argument 1 of 'lrintf' */

/* 违反约束「lrintl 的参数类型为 long double」：传入指针，应报错 */
lrintl(ip);          /* error: incompatible type for argument 1 of 'lrintl' */

/* 违反约束「llrintf 的参数类型为 float」：传入结构体，应报错 */
llrintf(s);          /* error: incompatible type for argument 1 of 'llrintf' */

/* 违反约束「llrintl 的参数类型为 long double」：传入指针，应报错 */
llrintl(ip);         /* error: incompatible type for argument 1 of 'llrintl' */

/* 违反约束「函数调用实参个数必须与原型一致」：多传一个参数，应报错 */
lrint(1.0, 2.0);     /* error: too many arguments to function 'lrint' */

/* 违反约束「函数调用实参个数必须与原型一致」：少传参数，应报错 */
llrint();            /* error: too few arguments to function 'llrint' */

/* 违反约束「函数返回值不是左值，不能赋值」：对 lrint 的返回值赋值，应报错 */
lrint(1.0) = 5;      /* error: lvalue required as left operand of assignment */

/* 违反约束「函数返回值不是左值，不能取地址」：对 llrint 的返回值取地址，应报错 */
&llrint(1.0);        /* error: lvalue required as unary '&' operand */

#endif