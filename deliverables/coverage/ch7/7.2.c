/*
 * 测试 C99 7.2 <assert.h> Diagnostics
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 生效时断言成立不触发；
 *             NDEBUG 定义时 assert 展开为 ((void)0)，表达式不求值）。
 *   负向测试：违反约束的片段应导致编译报错（见 #if 0 块内注释）。
 *
 * 覆盖段落：
 *   [1] <assert.h> 定义 assert 宏，引用 NDEBUG（<assert.h> 自身不定义 NDEBUG）；
 *       NDEBUG 已定义时 assert 定义为 #define assert(ignore) ((void)0)；
 *       每次包含 <assert.h> 时按 NDEBUG 当前状态重新定义 assert。
 *   [2] assert 必须实现为宏，而非实际函数；抑制宏定义以访问实际函数时行为未定义。
 */

#include <stdio.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 未定义 NDEBUG 时，assert 是生效的宏：断言为真时无副作用地通过 */
static int assert_active_test(void)
{
    int x = 5;
    assert(x == 5);          /* 断言成立，程序继续 */
    assert(x > 0);
    return x;
}

/* [1] assert 是宏：可用 #ifdef 检测其是否为宏名（宏可用 #ifdef 检测） */
#ifdef assert
#  define ASSERT_IS_MACRO 1
#else
#  define ASSERT_IS_MACRO 0
#endif

/* [1] 每次包含 <assert.h> 时按 NDEBUG 当前状态重新定义 assert。
 *     这里演示：先定义 NDEBUG，再重新包含 <assert.h>，assert 应变为 ((void)0)，
 *     即其参数表达式不被求值（无副作用）。 */
static int assert_ndebug_reinclude_test(void)
{
    int side_effect = 0;

    /* 在包含 <assert.h> 之前定义 NDEBUG */
#ifndef NDEBUG
#  define NDEBUG 1
#endif
    /* 重新包含 <assert.h>：按当前 NDEBUG 状态重新定义 assert */
#include <assert.h>

    /* 此时 assert 展开为 ((void)0)，参数表达式不应被求值 */
    assert(side_effect++);   /* 若被求值，side_effect 会变成 1 */

    /* 清理：撤销 NDEBUG，恢复 assert 为生效状态，供后续测试使用 */
#undef NDEBUG
#include <assert.h>

    return side_effect;      /* 期望为 0，证明参数未被求值 */
}

/* [1] NDEBUG 不由 <assert.h> 定义：在未自行定义 NDEBUG 时，包含 <assert.h>
 *     不应使 NDEBUG 变为已定义。此测试在文件顶部已包含 <assert.h> 之后进行。 */
static int ndebug_not_defined_by_header_test(void)
{
#ifdef NDEBUG
    /* 若走到这里说明 <assert.h> 定义了 NDEBUG，违反 [1] */
    return 1;
#else
    return 0;   /* 期望：NDEBUG 未被 <assert.h> 定义 */
#endif
}

/* [2] assert 必须实现为宏：用 #ifdef assert 验证其为宏名 */
static int assert_is_macro_test(void)
{
    return ASSERT_IS_MACRO;   /* 期望为 1 */
}

int main(void)
{
    /* [1] 生效状态下断言成立，正常通过 */
    assert(assert_active_test() == 5);

    /* [1] NDEBUG 未由 <assert.h> 定义 */
    assert(ndebug_not_defined_by_header_test() == 0);

    /* [2] assert 是宏 */
    assert(assert_is_macro_test() == 1);

    /* [1] 重新包含 <assert.h> 后按 NDEBUG 状态重定义：参数不被求值 */
    assert(assert_ndebug_reinclude_test() == 0);

    /* [1] 恢复生效状态后，assert 再次正常工作 */
    assert(1 == 1);

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 说明：7.2 本身主要是描述性条款（宏的定义方式与重定义规则），
 * 其“约束”体现在 assert 必须作为宏实现、以及 NDEBUG 的处理方式上。
 * 下面给出若干“若实现违反条款则无法通过”的负向片段，用于验证实现是否合规。
 * 注意：这些片段放在 #if 0 中，保证本文件整体仍可编译运行。 */

/* 违反 [2]「assert 必须实现为宏，而非实际函数」：
 * 若某实现把 assert 做成函数而非宏，则下面用 #ifdef assert 检测宏名的代码
 * 会走 #else 分支并触发 #error，从而在编译期报错。
 * 期望：合规实现下 assert 是宏，#error 不触发；不合规实现下编译报错。 */
#ifdef assert
    /* 合规：assert 是宏，正常 */
#else
#  error "assert must be implemented as a macro, not a function (C99 7.2 [2])"
#endif

/* 违反 [1]「NDEBUG 不由 <assert.h> 定义」：
 * 若某实现在包含 <assert.h> 时自行定义了 NDEBUG，则下面会触发 #error。
 * 期望：合规实现下 NDEBUG 未被 <assert.h> 定义，#error 不触发。 */
#include <assert.h>
#ifdef NDEBUG
#  error "<assert.h> must not define NDEBUG (C99 7.2 [1])"
#endif

/* 违反 [1]「NDEBUG 已定义时 assert 定义为 ((void)0)」：
 * 若某实现在 NDEBUG 已定义时仍对 assert 参数求值，则运行期 side_effect 会改变。
 * 这里用编译期无法直接检测，故以注释形式记录期望的编译/运行行为：
 * 期望：定义 NDEBUG 后重新包含 <assert.h>，assert(expr) 不求值 expr。 */

#endif /* 负向测试结束 */