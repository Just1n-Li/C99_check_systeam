/*
 * 测试 C99 7.13.2.1 —— longjmp 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 覆盖段落：
 *   [1] 原型声明
 *   [2] 恢复最近一次 setjmp 保存的环境；无对应调用/函数已终止/变长类型作用域已离开 => UB
 *   [3] 除自动存储期、非 volatile、在 setjmp 与 longjmp 之间被修改的局部对象外，
 *       所有可访问对象保留 longjmp 调用时刻的值
 *   [4] longjmp 完成后如同 setjmp 刚返回 val；val==0 时 setjmp 返回 1
 *   [5] EXAMPLE：VLA 内存可能被浪费（仅演示，不产生可断言结果）
 */

#include <setjmp.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 原型：void longjmp(jmp_buf env, int val); */
static jmp_buf g_env;

/* 用于 [3] 测试：全局对象在 longjmp 后应保留 longjmp 调用时刻的值 */
static int g_global = 0;

/* 用于 [3] 测试：volatile 局部对象在 longjmp 后应保留 longjmp 调用时刻的值 */
static void test_volatile_preserved(void)
{
    jmp_buf env;
    volatile int v = 10;
    int r = setjmp(env);
    if (r == 0) {
        v = 42;                 /* 在 setjmp 与 longjmp 之间修改 volatile 局部 */
        longjmp(env, 7);
        assert(0 && "unreachable");
    }
    /* [3] volatile 限定的自动对象保留 longjmp 调用时刻的值 */
    assert(r == 7);
    assert(v == 42);
}

/* [3] 非 volatile 自动对象在 setjmp/longjmp 之间被修改 => 值不确定（UB 范畴，不在此断言） */

/* [4] val == 0 时 setjmp 返回 1 */
static void test_val_zero_returns_one(void)
{
    jmp_buf env;
    int r = setjmp(env);
    if (r == 0) {
        longjmp(env, 0);        /* [4] longjmp 不能使 setjmp 返回 0 */
        assert(0 && "unreachable");
    }
    assert(r == 1);             /* [4] val 为 0 时 setjmp 返回 1 */
}

/* [4] val 非 0 时 setjmp 返回该值 */
static void test_val_nonzero(void)
{
    jmp_buf env;
    int r = setjmp(env);
    if (r == 0) {
        longjmp(env, 12345);
        assert(0 && "unreachable");
    }
    assert(r == 12345);
}

/* [2] 跨函数 longjmp：恢复最近一次 setjmp 保存的环境 */
static void inner_cross(void)
{
    longjmp(g_env, 99);         /* [2] 恢复到 f_cross 中的 setjmp */
}

static void test_cross_function(void)
{
    int r = setjmp(g_env);
    if (r == 0) {
        inner_cross();
        assert(0 && "unreachable");
    }
    assert(r == 99);
}

/* [3] 全局对象在 longjmp 后保留 longjmp 调用时刻的值 */
static void set_global_then_jump(void)
{
    g_global = 777;             /* 在 setjmp 与 longjmp 之间修改全局对象 */
    longjmp(g_env, 5);
}

static void test_global_preserved(void)
{
    int r = setjmp(g_env);
    if (r == 0) {
        set_global_then_jump();
        assert(0 && "unreachable");
    }
    assert(r == 5);
    /* [3] 全局对象（非自动存储期）保留 longjmp 调用时刻的值 */
    assert(g_global == 777);
}

/* [3] 通过指针访问的自动对象（非本函数局部）保留 longjmp 调用时刻的值 */
static void test_pointer_to_outer_auto(void)
{
    jmp_buf env;
    int outer = 1;
    int *p = &outer;
    int r = setjmp(env);
    if (r == 0) {
        *p = 555;               /* 修改的是调用者函数的自动对象，非 setjmp 所在函数的局部 */
        longjmp(env, 3);
        assert(0 && "unreachable");
    }
    assert(r == 3);
    /* [3] outer 不是包含 setjmp 调用的函数的局部对象，故保留其值 */
    assert(outer == 555);
}

/* [5] EXAMPLE：VLA 与 longjmp —— 演示内存可能被浪费（不产生可断言结果） */
static jmp_buf vla_env;
static int vla_n = 6;

static void vla_h(int n)
{
    int b[n];                   /* [5] b 可能保持已分配 */
    (void)b;
    longjmp(vla_env, 2);        /* [5] 可能导致内存浪费 */
}

static void vla_g(int n)
{
    int a[n];                   /* [5] a 可能保持已分配 */
    (void)a;
    vla_h(n);
}

static void test_vla_example(void)
{
    int r = setjmp(vla_env);
    if (r == 0) {
        vla_g(vla_n);
        assert(0 && "unreachable");
    }
    /* [5] 仅验证控制流返回，VLA 内存是否浪费不可移植地断言 */
    assert(r == 2);
}

int main(void)
{
    test_val_zero_returns_one();    /* [4] */
    test_val_nonzero();             /* [4] */
    test_cross_function();          /* [2] */
    test_global_preserved();        /* [3] */
    test_volatile_preserved();      /* [3] */
    test_pointer_to_outer_auto();   /* [3] */
    test_vla_example();             /* [5] */

    printf("C99 7.13.2.1 longjmp: all positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「longjmp 的第一个参数类型必须为 jmp_buf」：
 * 传入 int 而非 jmp_buf，gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg_type(void)
{
    int not_env = 0;
    longjmp(not_env, 1);        /* 期望报错：参数类型不兼容 */
}

/* 违反约束「longjmp 的第二个参数类型必须为 int」：
 * 传入 struct 而非 int，gcc -std=c99 应报 incompatible type 错误。 */
struct NotInt { int x; };
void bad_arg2_type(void)
{
    jmp_buf env;
    struct NotInt s = { 0 };
    longjmp(env, s);            /* 期望报错：参数类型不兼容 */
}

/* 违反约束「longjmp 参数个数必须为 2」：
 * 只传一个参数，gcc -std=c99 应报 too few arguments 错误。 */
void bad_arg_count(void)
{
    jmp_buf env;
    longjmp(env);               /* 期望报错：参数太少 */
}

/* 违反约束「longjmp 参数个数必须为 2」：
 * 传三个参数，gcc -std=c99 应报 too many arguments 错误。 */
void bad_arg_count2(void)
{
    jmp_buf env;
    longjmp(env, 1, 2);         /* 期望报错：参数太多 */
}

/* 违反约束「longjmp 返回类型为 void，不能用于需要值的上下文」：
 * 将 longjmp 的返回值赋给 int，gcc -std=c99 应报 void value not ignored 错误。 */
void bad_void_value(void)
{
    jmp_buf env;
    int x = longjmp(env, 1);    /* 期望报错：void 值不能使用 */
    (void)x;
}

/* 违反约束「调用 longjmp 前必须包含 <setjmp.h> 声明」：
 * 若未包含头文件，隐式声明与 C99 冲突（C99 取消隐式函数声明），
 * gcc -std=c99 应报 implicit declaration 错误。此处仅示意。 */
/* longjmp(env, 1); */          /* 期望报错：隐式声明 */

#endif /* 负向测试结束 */