/*
 * 测试 C99 7.13 <setjmp.h> —— Nonlocal jumps
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束/条款，编译器应报错；
 *             由于被 #if 0 屏蔽，整个文件仍可正常编译运行。
 *
 * 覆盖段落：
 *   [1] <setjmp.h> 定义宏 setjmp，声明一个函数 longjmp 和一个类型 jmp_buf，
 *       用于绕过正常的函数调用/返回规则。
 *   [2] jmp_buf 是数组类型，用于保存恢复调用环境所需信息；
 *       环境不包含浮点状态标志、打开文件等抽象机其它组件。
 *   [3] setjmp 是宏还是外部链接标识符未指定；
 *       抑制宏定义以访问实际函数、或程序定义名为 setjmp 的外部标识符，
 *       行为未定义（UB，不作为负向测试）。
 */

#include <setjmp.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] jmp_buf 是数组类型：sizeof 可用，可声明对象、数组、指针 */
static jmp_buf env_global;              /* 全局 jmp_buf 对象 */
static jmp_buf env_array[2];            /* jmp_buf 数组 */

/* [1] 用于测试 longjmp 跨函数跳转的辅助函数 */
static void do_longjmp(jmp_buf env, int val)
{
    longjmp(env, val);                  /* [1] 绕过正常返回，跳回 setjmp */
    /* 不会执行到这里 */
    assert(0 && "longjmp returned");
}

/* [1] 递归调用场景：验证环境足以恢复到正确的块与调用 */
static int recurse_and_jump(jmp_buf env, int depth)
{
    if (depth == 0) {
        longjmp(env, 42);               /* 从深层递归跳回 */
    }
    return recurse_and_jump(env, depth - 1) + 1;
}

int main(void)
{
    /* ---------- [1] setjmp/longjmp 基本语义 ---------- */
    {
        jmp_buf env;
        int r = setjmp(env);            /* [1] setjmp 宏调用 */
        if (r == 0) {
            /* 首次返回 0 */
            assert(r == 0);
            longjmp(env, 7);            /* [1] 非局部跳转，返回值为 7 */
            assert(0 && "unreachable");
        } else {
            /* longjmp 返回后，setjmp 返回非零值 7 */
            assert(r == 7);
        }
    }

    /* ---------- [1] longjmp 的返回值被 setjmp 返回 ---------- */
    {
        jmp_buf env;
        int r = setjmp(env);
        if (r == 0) {
            longjmp(env, 123);
        }
        assert(r == 123);
    }

    /* ---------- [1] 跨函数跳转：绕过正常函数返回 ---------- */
    {
        jmp_buf env;
        int r = setjmp(env);
        if (r == 0) {
            do_longjmp(env, 55);        /* 在另一函数中 longjmp */
            assert(0 && "unreachable");
        }
        assert(r == 55);
    }

    /* ---------- [2] 环境足以恢复到正确的块与调用（递归场景） ---------- */
    {
        jmp_buf env;
        int r = setjmp(env);
        if (r == 0) {
            recurse_and_jump(env, 5);   /* 深层递归中 longjmp */
            assert(0 && "unreachable");
        }
        assert(r == 42);                /* 正确恢复到本块 */
    }

    /* ---------- [2] jmp_buf 是数组类型 ---------- */
    {
        /* 数组类型：sizeof 合法，且大小 > 0 */
        assert(sizeof(jmp_buf) > 0);
        /* jmp_buf 对象可作数组元素 */
        assert(sizeof(env_array) == 2 * sizeof(jmp_buf));
        /* 全局 jmp_buf 可用 */
        int r = setjmp(env_global);
        if (r == 0) {
            longjmp(env_global, 9);
        }
        assert(r == 9);
    }

    /* ---------- [2] 环境不包含浮点状态标志（仅验证跳转后浮点运算仍正常） ---------- */
    {
        jmp_buf env;
        volatile double d = 1.5;
        int r = setjmp(env);
        if (r == 0) {
            d = d * 2.0;                /* 浮点运算 */
            longjmp(env, 3);
        }
        assert(r == 3);
        /* 条款说明环境不含浮点状态标志，此处仅验证跳转本身不破坏浮点运算 */
        assert(d == 3.0);
    }

    /* ---------- [2] 环境不包含打开文件状态（仅验证跳转后文件操作仍可用） ---------- */
    {
        jmp_buf env;
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int r = setjmp(env);
        if (r == 0) {
            fputs("hello", fp);
            longjmp(env, 4);
        }
        assert(r == 4);
        /* 文件对象本身仍有效（条款说明环境不含文件状态，此处仅验证跳转不破坏它） */
        fflush(fp);
        fclose(fp);
    }

    /* ---------- [3] setjmp 可作为宏使用（正常调用形式） ---------- */
    {
        jmp_buf env;
        /* 无论 setjmp 是宏还是函数，以下调用形式都应合法 */
        int r = setjmp(env);
        if (r == 0) {
            longjmp(env, 1);
        }
        assert(r == 1);
    }

    printf("C99 7.13 <setjmp.h> 正向测试全部通过\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 说明：C99 7.13 本身未给出显式的 "Constraints" 段落，
 * 但以下片段违反条款所依赖的约束/语义，编译器应报错。
 * 这些片段被 #if 0 屏蔽，不影响上面的正向测试。
 */

/* --- 违反约束「setjmp 宏/函数需要 jmp_buf 类型实参」 --- */
/* setjmp 的参数必须是 jmp_buf 类型；传入 int 应报错 */
void bad_setjmp_arg(void)
{
    int x = 0;
    setjmp(x);          /* 期望报错：参数类型不匹配（int 不是 jmp_buf） */
}

/* --- 违反约束「longjmp 的第一个参数必须是 jmp_buf」 --- */
void bad_longjmp_arg(void)
{
    int x = 0;
    longjmp(x, 1);      /* 期望报错：第一个参数类型不匹配 */
}

/* --- 违反约束「longjmp 需要两个实参」 --- */
void bad_longjmp_arity(void)
{
    jmp_buf env;
    longjmp(env);       /* 期望报错：实参个数不足 */
}

/* --- 违反约束「jmp_buf 是数组类型，不能直接赋值」 --- */
void bad_jmpbuf_assign(void)
{
    jmp_buf a, b;
    a = b;              /* 期望报错：数组类型不可赋值 */
}

/* --- 违反约束「jmp_buf 是数组类型，不能作为函数返回值」 --- */
jmp_buf bad_return_jmpbuf(void)   /* 期望报错：函数不能返回数组类型 */
{
    jmp_buf env;
    return env;
}

/* --- 违反约束「setjmp 的返回值不能用于需要常量表达式的场合」 --- */
void bad_setjmp_const(void)
{
    jmp_buf env;
    int arr[setjmp(env)];   /* 期望报错：数组长度不是整数常量表达式 */
}

#endif /* 负向测试结束 */