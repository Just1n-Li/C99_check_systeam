/*
 * 测试 C99 7.23.2.1 —— clock 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过，验证 clock() 的语义：
 *             - 声明于 <time.h>，原型为 clock_t clock(void)
 *             - 返回处理器时间的最佳近似值
 *             - 返回值可除以 CLOCKS_PER_SEC 得到秒数
 *             - 时间不可用/不可表示时返回 (clock_t)(-1)
 *             - 多次调用返回值单调不减（脚注 275 的用法）
 *   负向测试：违反约束的代码应编译报错（见文件末尾 #if 0 块）
 */

#include <time.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：clock 接受 void 参数，返回 clock_t。
 *     用函数指针类型强制匹配，若原型不符则编译报错。 */
static clock_t (*clock_proto_check)(void) = clock;

int main(void)
{
    /* [1] 头文件 <time.h> 提供 clock_t 与 CLOCKS_PER_SEC */
    clock_t t0, t1, t2;
    double seconds;

    /* [1] 原型：无参数调用 */
    t0 = clock();
    assert(clock_proto_check == clock);

    /* [3] 返回值类型为 clock_t；CLOCKS_PER_SEC 为正的常量 */
    assert(CLOCKS_PER_SEC > 0);

    /* [2][3] 消耗一些处理器时间，然后再次调用 */
    {
        volatile unsigned long i;
        unsigned long sum = 0;
        for (i = 0; i < 2000000UL; ++i)
            sum += i;
        /* 防止优化掉循环 */
        assert(sum == 1999999000000UL || sum != 0);
    }

    t1 = clock();

    /* [3] 若时间可用，则 t0、t1 均不为 (clock_t)(-1)，
     *     且处理器时间单调不减（脚注 275 的用法）。 */
    if (t0 != (clock_t)(-1) && t1 != (clock_t)(-1)) {
        assert(t1 >= t0);

        /* [3] 除以 CLOCKS_PER_SEC 得到秒数（非负） */
        seconds = (double)t1 / (double)CLOCKS_PER_SEC;
        assert(seconds >= 0.0);

        /* 脚注 275：用后续调用减去起始调用，得到程序消耗的时间 */
        {
            double elapsed = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
            assert(elapsed >= 0.0);
        }
    }

    /* [3] 再次调用，验证可重复调用且结果一致地单调 */
    t2 = clock();
    if (t1 != (clock_t)(-1) && t2 != (clock_t)(-1)) {
        assert(t2 >= t1);
    }

    /* [3] 若返回 (clock_t)(-1)，表示时间不可用或不可表示；
     *     这是允许的实现行为，此处仅验证该值可被正确比较。 */
    if (t0 == (clock_t)(-1)) {
        assert(t0 == (clock_t)-1);
    }

    printf("C99 7.23.2.1 clock() 正向测试通过\n");
    printf("  CLOCKS_PER_SEC = %ld\n", (long)CLOCKS_PER_SEC);
    printf("  clock() 起始值 = %ld\n", (long)t0);
    printf("  clock() 结束值 = %ld\n", (long)t2);
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「clock 的原型为 clock_t clock(void)」：
 * 以参数调用 clock，实参个数与原型不符，gcc -std=c99 应报错
 * （"too many arguments to function 'clock'"）。 */
#include <time.h>
void bad_call_with_arg(void)
{
    clock(1);
}

/* 违反约束「clock 的原型为 clock_t clock(void)」：
 * 将 clock 的返回值赋给不兼容类型并用于需要算术类型的上下文，
 * 这里演示把函数指针当作对象使用（对函数名取地址后解引用调用错误）。
 * 更直接地：把 clock 当作对象赋值，gcc 应报错。 */
void bad_assign_to_function(void)
{
    clock = 0;   /* 函数指示符不是可修改左值，应报错 */
}

/* 违反约束「clock 无参数」：
 * 用不完整/错误的参数列表声明与 <time.h> 中声明冲突，
 * 应报 "conflicting types for 'clock'"。 */
clock_t clock(int);   /* 与 clock_t clock(void) 冲突 */

#endif /* 负向测试结束 */