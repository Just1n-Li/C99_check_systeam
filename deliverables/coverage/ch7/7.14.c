/*
 * 测试目标：C99 7.14 Signal handling <signal.h>
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反 7.14 约束的代码片段应被编译器拒绝（编译报错），
 *             这些片段统一放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] <signal.h> 声明一个类型、两个函数、若干宏
 *   [2] sig_atomic_t 类型（可能带 volatile 限定）的整数类型
 *   [3] SIG_DFL / SIG_ERR / SIG_IGN 常量表达式，类型与 signal 第二参数
 *       及返回值兼容，且不等于任何可声明函数的地址；
 *       SIGABRT/SIGFPE/SIGILL/SIGINT/SIGSEGV/SIGTERM 为 int 型正整数常量表达式
 *   [4] 信号号均为正；实现可额外定义信号；默认处理实现定义
 */

#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* 用于验证 signal 返回值与 handler 类型兼容的处理器 */
static void my_handler(int sig)
{
    (void)sig;
}

/* 用于验证 SIG_DFL/SIG_IGN/SIG_ERR 与函数地址不相等 */
static void some_function(int sig)
{
    (void)sig;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] <signal.h> 声明了 signal 与 raise 两个函数 */
    {
        /* 取函数地址，验证声明存在且可调用 */
        void (*psig)(int) = signal;
        int  (*praise)(int) = raise;
        assert(psig != NULL);
        assert(praise != NULL);
    }

    /* [2] sig_atomic_t 是整数类型（可带 volatile 限定） */
    {
        sig_atomic_t s = 0;
        s = 42;
        assert(s == 42);

        /* 允许 volatile 限定形式 */
        volatile sig_atomic_t vs = 7;
        assert(vs == 7);

        /* 必须是整数类型：可用算术运算验证 */
        sig_atomic_t a = 3, b = 4;
        assert(a + b == 7);
    }

    /* [3] SIG_DFL / SIG_ERR / SIG_IGN 是常量表达式，
     *     类型与 signal 第二参数及返回值兼容，
     *     且不等于任何可声明函数的地址 */
    {
        /* 类型兼容：可赋给 signal 的返回类型 / 第二参数类型 */
        void (*h1)(int) = SIG_DFL;
        void (*h2)(int) = SIG_IGN;
        void (*h3)(int) = SIG_ERR;
        (void)h1; (void)h2; (void)h3;

        /* 三者互不相等 */
        assert(SIG_DFL != SIG_IGN);
        assert(SIG_DFL != SIG_ERR);
        assert(SIG_IGN != SIG_ERR);

        /* 不等于任何可声明函数的地址 */
        assert(SIG_DFL != some_function);
        assert(SIG_IGN != some_function);
        assert(SIG_ERR != some_function);
    }

    /* [3] 六个信号宏是 int 型正整数常量表达式，且互不相同 */
    {
        /* 类型为 int：用 sizeof 验证 */
        assert(sizeof(SIGABRT) == sizeof(int));
        assert(sizeof(SIGFPE)  == sizeof(int));
        assert(sizeof(SIGILL)  == sizeof(int));
        assert(sizeof(SIGINT)  == sizeof(int));
        assert(sizeof(SIGSEGV) == sizeof(int));
        assert(sizeof(SIGTERM) == sizeof(int));

        /* 均为正 */
        assert(SIGABRT > 0);
        assert(SIGFPE  > 0);
        assert(SIGILL  > 0);
        assert(SIGINT  > 0);
        assert(SIGSEGV > 0);
        assert(SIGTERM > 0);

        /* 互不相同 */
        assert(SIGABRT != SIGFPE);
        assert(SIGABRT != SIGILL);
        assert(SIGABRT != SIGINT);
        assert(SIGABRT != SIGSEGV);
        assert(SIGABRT != SIGTERM);
        assert(SIGFPE  != SIGILL);
        assert(SIGFPE  != SIGINT);
        assert(SIGFPE  != SIGSEGV);
        assert(SIGFPE  != SIGTERM);
        assert(SIGILL  != SIGINT);
        assert(SIGILL  != SIGSEGV);
        assert(SIGILL  != SIGTERM);
        assert(SIGINT  != SIGSEGV);
        assert(SIGINT  != SIGTERM);
        assert(SIGSEGV != SIGTERM);
    }

    /* [3] 常量表达式：可用于静态初始化与数组维度 */
    {
        static const int arr_dim = SIGINT;   /* 常量表达式 */
        int arr[SIGINT > 0 ? 1 : -1];        /* 常量表达式作维度 */
        (void)arr_dim;
        (void)arr;
    }

    /* [1][4] signal 函数可安装处理器；raise 可显式产生信号 */
    {
        void (*old)(int) = signal(SIGINT, my_handler);
        assert(old != SIG_ERR);

        int r = raise(SIGINT);
        assert(r == 0);

        /* 恢复默认处理 */
        old = signal(SIGINT, SIG_DFL);
        assert(old != SIG_ERR);
    }

    /* [4] 信号号均为正（再次以变量形式验证） */
    {
        int nums[6];
        nums[0] = SIGABRT;
        nums[1] = SIGFPE;
        nums[2] = SIGILL;
        nums[3] = SIGINT;
        nums[4] = SIGSEGV;
        nums[5] = SIGTERM;
        for (int i = 0; i < 6; i++) {
            assert(nums[i] > 0);
        }
    }

    printf("C99 7.14 signal.h: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「SIG_DFL/SIG_IGN/SIG_ERR 类型必须与 signal 第二参数及返回值兼容」：
 * 将其赋给不兼容的函数指针类型，gcc -std=c99 应报错（incompatible pointer type）。 */
void (*bad1)(double) = SIG_DFL;

/* 违反约束「signal 第二参数类型为 void(*)(int)」：
 * 传入不兼容的函数指针，gcc -std=c99 应报错。 */
void wrong_handler(double d) { (void)d; }
void use_signal_bad(void) {
    signal(SIGINT, wrong_handler);   /* 参数类型不兼容 */
}

/* 违反约束「signal 返回类型为 void(*)(int)」：
 * 赋给不兼容类型，gcc -std=c99 应报错。 */
void use_signal_bad2(void) {
    double (*p)(int) = signal(SIGINT, SIG_DFL);  /* 返回类型不兼容 */
    (void)p;
}

/* 违反约束「raise 参数为 int」：
 * 传入结构体，gcc -std=c99 应报错。 */
struct S { int x; };
void use_raise_bad(void) {
    struct S s;
    raise(s);   /* 参数类型不兼容 */
}

/* 违反约束「sig_atomic_t 是整数类型」：
 * 不能把结构体赋给 sig_atomic_t，gcc -std=c99 应报错。 */
void use_sig_atomic_bad(void) {
    struct S s;
    sig_atomic_t v = s;   /* 类型不兼容 */
    (void)v;
}

/* 违反约束「signal 第二参数为 void(*)(int)」：
 * 传入整数常量，gcc -std=c99 应报错。 */
void use_signal_bad3(void) {
    signal(SIGINT, 12345);   /* 参数类型不兼容 */
}

#endif