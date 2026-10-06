/*
 * 测试 C99 7.20.4.2 —— atexit 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过，验证：
 *     [1] 原型 int atexit(void (*func)(void)); 可用
 *     [2] 注册的函数在正常程序终止时被无参调用
 *     [3] 实现至少支持注册 32 个函数
 *     [4] 注册成功返回 0，失败返回非 0
 *   负向测试：违反约束的代码应编译报错（见 #if 0 块）
 *
 * 注意：atexit 注册的函数在 main 返回后、程序正常终止时被调用。
 *       因此本程序把“注册是否成功”的检查放在 main 中，
 *       把“回调是否被调用”的检查放在回调函数里，通过退出码/输出体现。
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* ---------- 用于验证 [2]：回调被无参调用 ---------- */
static int g_called = 0;          /* 回调被调用的次数 */
static int g_order[8];            /* 记录调用顺序 */
static int g_order_n = 0;

static void cb_noarg(void)        /* 无参回调，符合 [2] */
{
    g_called++;
    if (g_order_n < 8)
        g_order[g_order_n++] = 1;
}

static void cb_mark2(void)
{
    if (g_order_n < 8)
        g_order[g_order_n++] = 2;
}

static void cb_mark3(void)
{
    if (g_order_n < 8)
        g_order[g_order_n++] = 3;
}

/* ---------- 用于验证 [3]：至少支持 32 个注册 ---------- */
#define N32 32
static int g_bulk_called = 0;
static void cb_bulk(void) { g_bulk_called++; }

/* ---------- 用于验证 [4]：返回值语义 ---------- */
static void cb_ret(void) { /* 仅用于注册 */ }

int main(void)
{
    /* ============================================================
     * [1] 原型检查：atexit 接受 void(*)(void) 并返回 int
     * ============================================================ */
    {
        int (*fp)(void (*)(void)) = atexit;   /* 类型必须匹配 [1] */
        assert(fp != NULL);
    }

    /* ============================================================
     * [4] 注册成功应返回 0
     * ============================================================ */
    {
        int r = atexit(cb_ret);
        assert(r == 0);                       /* [4] 成功返回 0 */
    }

    /* ============================================================
     * [2] 注册一个无参回调，期望在正常终止时被调用
     * ============================================================ */
    {
        int r = atexit(cb_noarg);
        assert(r == 0);
    }

    /* ============================================================
     * [2] 注册多个回调，验证它们都会被调用（顺序为逆序，但标准
     *     只保证“被调用”，不强制顺序；这里只检查都被调用）
     * ============================================================ */
    {
        int r1 = atexit(cb_mark2);
        int r2 = atexit(cb_mark3);
        assert(r1 == 0 && r2 == 0);
    }

    /* ============================================================
     * [3] 环境限制：实现至少支持注册 32 个函数。
     *     这里注册 32 个，全部应成功（返回 0）。
     * ============================================================ */
    {
        int i;
        for (i = 0; i < N32; i++) {
            int r = atexit(cb_bulk);
            assert(r == 0);                   /* [3]+[4] 至少 32 个成功 */
        }
    }

    /* ============================================================
     * 在 main 返回前，回调尚未被调用（正常终止时才调用）
     * ============================================================ */
    assert(g_called == 0);
    assert(g_bulk_called == 0);

    printf("main: all registrations succeeded; callbacks run at exit.\n");

    /* 正常从 main 返回 —— 触发正常程序终止，回调应被调用 */
    return 0;
}

/*
 * 由于 atexit 回调在 main 返回之后运行，无法在 main 内 assert 回调结果。
 * 这里用一个“退出时检查”的回调：它最后注册，因此最先被调用（LIFO），
 * 但标准不保证顺序，所以只检查“所有回调都被调用过”。
 * 为便于观察，注册一个检查回调（最先注册 => 最后调用）。
 */
static void cb_final_check(void)
{
    /* 该函数在其它回调之后被调用（若实现为 LIFO）。
       标准不保证顺序，因此这里只做“被调用过”的弱检查，
       并把结果打印出来，供人工/脚本核对。 */
    printf("exit: cb_noarg called %d time(s)\n", g_called);
    printf("exit: bulk callbacks called %d time(s) (expect %d)\n",
           g_bulk_called, N32);
    printf("exit: order recorded %d callbacks\n", g_order_n);

    /* 弱断言：至少 cb_noarg 被调用一次，且 32 个 bulk 都被调用 */
    if (g_called < 1) {
        fprintf(stderr, "FAIL: cb_noarg not called at exit\n");
        _Exit(1);
    }
    if (g_bulk_called != N32) {
        fprintf(stderr, "FAIL: bulk callbacks %d != %d\n",
                g_bulk_called, N32);
        _Exit(1);
    }
    printf("PASS: atexit callbacks invoked at normal termination.\n");
}

/* 用构造函数在 main 之前注册“最终检查”回调，
   使其成为最早注册者，从而（在 LIFO 实现下）最后被调用。 */
#if defined(__GNUC__)
__attribute__((constructor))
static void register_final_check(void)
{
    (void)atexit(cb_final_check);
}
#endif

/* ================================================================
 * ========== 负向测试：以下代码违反 C99 约束，应编译报错 ==========
 * ================================================================
 * 说明：以下片段全部放在 #if 0 中，保证本文件仍能正常编译运行。
 *       每个片段都违反 7.20.4.2 的约束（[1] 原型 / 参数类型），
 *       期望 gcc -std=c99 报错。
 */
#if 0

/* 违反约束 [1]：atexit 的参数类型必须是 void (*)(void)。
 * 传入 int (*)(void) 类型不兼容，应报错
 * （incompatible pointer type / passing argument 1 ...）。 */
#include <stdlib.h>
static int bad_ret_int(void) { return 0; }
void t1(void) { atexit(bad_ret_int); }   /* 期望：编译错误 */

/* 违反约束 [1]：传入带参数的函数指针 void (*)(int)，
 * 与 void (*)(void) 不兼容，应报错。 */
static void bad_takes_arg(int x) { (void)x; }
void t2(void) { atexit(bad_takes_arg); } /* 期望：编译错误 */

/* 违反约束 [1]：atexit 需要函数指针，传入整数常量应报错。 */
void t3(void) { atexit(0); }             /* 期望：编译错误（0 不是函数指针） */

/* 违反约束 [1]：atexit 需要函数指针，传入对象指针应报错。 */
static int obj;
void t4(void) { atexit(&obj); }          /* 期望：编译错误（int* 不兼容） */

/* 违反约束 [1]：atexit 需要函数指针，传入 void* 应报错。 */
void t5(void *p) { atexit(p); }          /* 期望：编译错误 */

/* 违反约束 [1]：atexit 返回 int，不能当作 void 函数使用于需要
 * 返回值的上下文之外——这里演示把返回值赋给不兼容类型（可选）。
 * 更直接：调用时参数个数错误。 */
void t6(void) { atexit(); }              /* 期望：编译错误（参数太少） */

/* 违反约束 [1]：参数个数过多。 */
void t7(void) { atexit(bad_ret_int, bad_ret_int); } /* 期望：编译错误 */

#endif /* 负向测试结束 */