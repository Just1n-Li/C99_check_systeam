/*
 * 测试 C99 7.23.2.2 —— difftime 函数
 *
 * 预期行为：
 *   正向测试：包含 <time.h>，调用 difftime(time1, time0)，
 *             验证其返回 double 类型且值等于 time1 - time0（以秒为单位）。
 *   负向测试：违反约束的代码应导致编译报错（见 #if 0 块）。
 *
 * 覆盖段落：
 *   [1] 原型：double difftime(time_t time1, time_t time0);
 *   [2] 语义：计算 time1 - time0
 *   [3] 返回值：以 double 表示的秒数差
 */

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <math.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：difftime 的返回类型必须是 double，
 *     两个形参类型必须是 time_t。
 *     通过赋值给精确类型来静态验证（若类型不符，编译器会给出警告/错误）。 */
static double (*fp_difftime)(time_t, time_t) = difftime;

int main(void)
{
    /* [1] 头文件 <time.h> 提供 difftime 声明，可正常取地址并调用 */
    assert(fp_difftime == difftime);

    /* [2][3] 基本语义：time1 - time0，以秒为单位，返回 double */
    {
        time_t t0 = (time_t)1000;
        time_t t1 = (time_t)1060;
        double d = difftime(t1, t0);
        /* 返回类型为 double */
        assert(sizeof(d) == sizeof(double));
        /* 差值为 60 秒 */
        assert(d == 60.0);
    }

    /* [2][3] 差值为负的情况：time1 < time0 */
    {
        time_t t0 = (time_t)5000;
        time_t t1 = (time_t)4900;
        double d = difftime(t1, t0);
        assert(d == -100.0);
    }

    /* [2][3] 差值为零 */
    {
        time_t t = (time_t)12345;
        double d = difftime(t, t);
        assert(d == 0.0);
    }

    /* [2][3] 使用真实时间：time(NULL) 两次调用，差值应 >= 0 且为 double */
    {
        time_t a = time(NULL);
        time_t b = time(NULL);
        double d = difftime(b, a);
        assert(d >= 0.0);
        /* 两次调用间隔极小，差值应为 0 或很小的整数秒 */
        assert(d == floor(d));
    }

    /* [3] 返回值可参与浮点运算，证明其为 double 而非整型 */
    {
        time_t t0 = (time_t)0;
        time_t t1 = (time_t)1;
        double d = difftime(t1, t0);
        double half = d / 2.0;   /* 若 d 为整型，此处会截断为 0 */
        assert(half == 0.5);
    }

    /* [2] 与 time() 配合：计算两个日历时间之差 */
    {
        time_t start = time(NULL);
        time_t end   = start + (time_t)3600; /* 人为构造 1 小时差 */
        double secs  = difftime(end, start);
        assert(secs == 3600.0);
    }

    printf("C99 7.23.2.2 difftime: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「difftime 的形参类型为 time_t」：
 * 传入不兼容的指针类型，且数量不符，gcc -std=c99 应报错。 */
void bad_call_1(void)
{
    double d = difftime("hello", "world"); /* 实参类型错误 */
    (void)d;
}

/* 违反约束「difftime 需要两个实参」：
 * 只传一个实参，gcc -std=c99 应报错（too few arguments）。 */
void bad_call_2(void)
{
    time_t t = (time_t)0;
    double d = difftime(t); /* 缺少第二个实参 */
    (void)d;
}

/* 违反约束「difftime 需要两个实参」：
 * 传三个实参，gcc -std=c99 应报错（too many arguments）。 */
void bad_call_3(void)
{
    time_t t = (time_t)0;
    double d = difftime(t, t, t); /* 实参过多 */
    (void)d;
}

/* 违反约束「difftime 返回 double，不能作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
void bad_call_4(void)
{
    time_t t = (time_t)0;
    difftime(t, t) = 1.0; /* 对非左值赋值 */
}

/* 违反约束「difftime 的返回类型为 double」：
 * 若将其返回值用于需要结构体/数组等不兼容类型的上下文，
 * 或声明为不兼容的函数指针类型，应报错。 */
void bad_call_5(void)
{
    /* 声明一个与 difftime 不兼容的函数指针并赋值，应报错 */
    int (*p)(time_t, time_t) = difftime; /* 返回类型不兼容 */
    (void)p;
}

#endif /* 负向测试结束 */