/*
 * 验证 C99 条款 6.9 External definitions
 * 预期行为：
 * - 正向测试：能编译并运行通过，assert 不触发。
 * - 负向测试：违反约束，应编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* [1] translation-unit: external-declaration */
/* [4] 外部声明出现在函数外，具有文件作用域，且为定义（导致存储空间被分配） */
int global_obj = 42; /* 对象定义 */
int global_func(void) { return 42; } /* 函数定义 */

/* [3] 内部链接标识符的定义与使用 */
static int internal_obj = 10; /* 内部链接对象定义 */
static int internal_func(void) { return 5; } /* 内部链接函数定义 */

/* [5] 脚注 140: 声明具有外部链接但未在表达式中使用，不需要外部定义 */
extern int unused_ext;

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
int main(void) {
    /* [4] 验证外部定义分配了存储空间并可被访问 */
    assert(global_obj == 42);
    assert(global_func() == 42);

    /* [3] 验证内部链接标识符被使用时，恰好有一个外部定义 */
    assert(internal_obj == 10);
    assert(internal_func() == 5);

    /* [5] 验证外部链接标识符被使用时，整个程序中恰好有一个外部定义 */
    /* global_obj 和 global_func 已在此处使用，它们在文件顶部有唯一定义 */

    printf("6.9 All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [2] 违反约束：auto 不能出现在外部声明的说明符中 */
auto int bad_auto = 1;

/* [2] 违反约束：register 不能出现在外部声明的说明符中 */
register int bad_register = 2;

/* [3] 违反约束：每个内部链接标识符不得有超过一个外部定义 */
static int dup_internal = 1;
static int dup_internal = 2;

/* [3] 违反约束：内部链接标识符在表达式中使用（sizeof 整数常量除外），必须恰好有一个外部定义 */
static int no_def_internal(void);
int use_no_def = no_def_internal(); /* 使用了但无定义，应报错 */

/* [5] 违反约束：外部链接标识符不得有超过一个外部定义 */
int dup_ext = 1;
int dup_ext = 2;
#endif