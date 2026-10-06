/*
 * 测试 C99 7.6.2.3 —— feraiseexcept 函数
 *
 * 预期行为：
 *   正向测试：包含 <fenv.h>，调用 feraiseexcept，验证：
 *     [1] 原型 int feraiseexcept(int excepts);
 *     [2] 能引发参数所表示的受支持浮点异常；参数为 0 时返回 0；
 *         成功引发所有指定异常时返回 0；否则返回非零。
 *     [3] 返回值语义。
 *   负向测试：违反约束的代码应编译报错（放在 #if 0 中）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra -lm
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 若编译器未定义 FE_ALL_EXCEPT 等宏，则跳过运行期检查，仅做编译期检查 */
#if defined(FE_ALL_EXCEPT) && defined(FE_DIVBYZERO) && defined(FE_INVALID) \
    && defined(FE_OVERFLOW) && defined(FE_UNDERFLOW) && defined(FE_INEXACT)

#define HAVE_FENV_MACROS 1
#else
#define HAVE_FENV_MACROS 0
#endif

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与 int (*)(int) 兼容 */
static int (*fp_feraiseexcept)(int) = feraiseexcept;

/* [2] 辅助：清除所有异常标志，便于观察 feraiseexcept 的效果 */
static void clear_all_exceptions(void)
{
#if HAVE_FENV_MACROS
    feclearexcept(FE_ALL_EXCEPT);
#endif
}

/* [2] 辅助：读取当前已引发的异常标志集合 */
static int get_raised_exceptions(void)
{
#if HAVE_FENV_MACROS
    return fetestexcept(FE_ALL_EXCEPT);
#else
    return 0;
#endif
}

int main(void)
{
    int r;

    /* [1] 原型可用性：能取地址、能通过函数指针调用 */
    assert(fp_feraiseexcept == feraiseexcept);

    /* [3] 参数为 0 时，必须返回 0 */
    clear_all_exceptions();
    r = feraiseexcept(0);
    assert(r == 0);

    /* [3] 参数为 0 时不应引发任何异常 */
    assert(get_raised_exceptions() == 0);

#if HAVE_FENV_MACROS
    /* [2] 引发 FE_DIVBYZERO：若受支持，应成功（返回 0）且标志被置位 */
    clear_all_exceptions();
    r = feraiseexcept(FE_DIVBYZERO);
    if (r == 0) {
        /* 成功引发：标志应被置位 */
        assert((get_raised_exceptions() & FE_DIVBYZERO) != 0);
    } else {
        /* 不受支持：返回非零，符合 [3] */
        assert(r != 0);
    }

    /* [2] 引发 FE_INVALID */
    clear_all_exceptions();
    r = feraiseexcept(FE_INVALID);
    if (r == 0) {
        assert((get_raised_exceptions() & FE_INVALID) != 0);
    } else {
        assert(r != 0);
    }

    /* [2] 引发 FE_OVERFLOW（实现可能同时引发 FE_INEXACT，属实现定义） */
    clear_all_exceptions();
    r = feraiseexcept(FE_OVERFLOW);
    if (r == 0) {
        assert((get_raised_exceptions() & FE_OVERFLOW) != 0);
        /* 实现定义：可能额外引发 FE_INEXACT，这里不做强制断言 */
    } else {
        assert(r != 0);
    }

    /* [2] 引发 FE_UNDERFLOW（实现可能同时引发 FE_INEXACT，属实现定义） */
    clear_all_exceptions();
    r = feraiseexcept(FE_UNDERFLOW);
    if (r == 0) {
        assert((get_raised_exceptions() & FE_UNDERFLOW) != 0);
    } else {
        assert(r != 0);
    }

    /* [2] 引发 FE_INEXACT */
    clear_all_exceptions();
    r = feraiseexcept(FE_INEXACT);
    if (r == 0) {
        assert((get_raised_exceptions() & FE_INEXACT) != 0);
    } else {
        assert(r != 0);
    }

    /* [2] 同时引发多个异常：FE_DIVBYZERO | FE_INVALID */
    clear_all_exceptions();
    r = feraiseexcept(FE_DIVBYZERO | FE_INVALID);
    if (r == 0) {
        int raised = get_raised_exceptions();
        assert((raised & FE_DIVBYZERO) != 0);
        assert((raised & FE_INVALID) != 0);
    } else {
        assert(r != 0);
    }

    /* [2] 同时引发 FE_OVERFLOW | FE_UNDERFLOW */
    clear_all_exceptions();
    r = feraiseexcept(FE_OVERFLOW | FE_UNDERFLOW);
    if (r == 0) {
        int raised = get_raised_exceptions();
        assert((raised & FE_OVERFLOW) != 0);
        assert((raised & FE_UNDERFLOW) != 0);
    } else {
        assert(r != 0);
    }

    /* [2] 引发全部受支持异常 */
    clear_all_exceptions();
    r = feraiseexcept(FE_ALL_EXCEPT);
    if (r == 0) {
        int raised = get_raised_exceptions();
        /* 至少 FE_ALL_EXCEPT 中所有位都应被置位 */
        assert((raised & FE_ALL_EXCEPT) == FE_ALL_EXCEPT);
    } else {
        assert(r != 0);
    }
#endif /* HAVE_FENV_MACROS */

    /* [3] 返回值语义：0 表示成功或参数为 0；非零表示失败 */
    /* 再次确认参数为 0 返回 0 */
    r = feraiseexcept(0);
    assert(r == 0);

    printf("C99 7.6.2.3 feraiseexcept: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参类型必须与形参兼容」：
 * feraiseexcept 的形参为 int，传入结构体类型应报错。
 * 期望：gcc -std=c99 报 "incompatible type for argument 1" 类错误。
 */
struct S { int x; } s;
feraiseexcept(s);

/* 违反约束「函数调用实参个数必须与原型一致」：
 * 原型为 int feraiseexcept(int)，不传实参应报错。
 * 期望：gcc -std=c99 报 "too few arguments to function" 类错误。
 */
feraiseexcept();

/* 违反约束「函数调用实参个数必须与原型一致」：
 * 原型只接受 1 个实参，传 2 个应报错。
 * 期望：gcc -std=c99 报 "too many arguments to function" 类错误。
 */
feraiseexcept(FE_INVALID, FE_DIVBYZERO);

/* 违反约束「函数返回值不能作为赋值目标（非左值）」：
 * feraiseexcept 返回 int 值，对其赋值应报错。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"。
 */
feraiseexcept(0) = 1;

/* 违反约束「不能对函数返回值取地址（非左值）」：
 * 期望：gcc -std=c99 报 "lvalue required as unary '&' operand"。
 */
int *p = &feraiseexcept(0);

/* 违反约束「函数名不能作为赋值目标（函数指示符非左值）」：
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"。
 */
feraiseexcept = 0;

#endif /* 负向测试结束 */