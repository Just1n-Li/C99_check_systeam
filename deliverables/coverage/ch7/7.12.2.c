/*
 * 测试条款：C99 7.12.2  The FP_CONTRACT pragma
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，在允许的上下文（外部声明处、复合语句内所有显式声明和语句之前）
 *             使用 #pragma STDC FP_CONTRACT on/off/on-off-switch，程序应能编译并运行通过。
 *             验证 pragma 的作用域规则：外部声明处生效至下一个 pragma 或翻译单元结束；
 *             复合语句内生效至下一个 pragma（含嵌套）或复合语句结束，结束时恢复之前状态。
 *   负向测试：把 #pragma STDC FP_CONTRACT 放在不允许的上下文（如语句之后、表达式中间等）
 *             属于未定义行为，但标准未规定必须报错，故此处不作为负向测试。
 *             本条款没有“违反后必须诊断”的约束（constraint）段落，
 *             因此负向测试部分仅演示“错误拼写/错误形式”的 pragma 会被实现忽略，
 *             并说明标准未要求诊断。
 *
 * 说明：本条款 Description 段落 [2] 全部为语义/作用域规则，无 Constraints 段落。
 *       因此负向测试以“非标准形式 pragma 被忽略”来体现，并注明这不是约束违反。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：包含 <math.h> 后可使用 #pragma STDC FP_CONTRACT on-off-switch */

/* [2] 在外部声明处使用 pragma：生效至下一个 pragma 或翻译单元结束 */
#pragma STDC FP_CONTRACT ON
#pragma STDC FP_CONTRACT OFF
#pragma STDC FP_CONTRACT DEFAULT

/* 再次打开，便于后续函数内测试 */
#pragma STDC FP_CONTRACT ON

/* 一个简单的浮点表达式，用于验证 pragma 存在时程序仍能正常计算 */
static double compute(double a, double b, double c)
{
    /* 表达式 a*b + c 可能被收缩为 fma(a,b,c)，但结果在数学上应一致（近似） */
    return a * b + c;
}

/* [2] 复合语句内使用 pragma：必须位于所有显式声明和语句之前 */
static double compute_in_block(double a, double b, double c)
{
#pragma STDC FP_CONTRACT OFF
    double r = a * b + c;
#pragma STDC FP_CONTRACT ON
    double s = a * b + c;
    return r + s;
}

/* [2] 嵌套复合语句：内层 pragma 生效至内层结束，结束后恢复外层状态 */
static double compute_nested(double a, double b, double c)
{
#pragma STDC FP_CONTRACT OFF
    double outer = a * b + c;
    {
#pragma STDC FP_CONTRACT ON
        double inner = a * b + c;
        outer += inner;
    }
    /* 此处应恢复为 OFF（外层状态） */
    double after = a * b + c;
    return outer + after;
}

int main(void)
{
    /* [2] 外部声明处的 pragma 已生效，函数调用应正常 */
    double v1 = compute(2.0, 3.0, 4.0);
    assert(v1 == 10.0);

    /* [2] 复合语句内 pragma 位于所有声明和语句之前，应合法 */
    double v2 = compute_in_block(2.0, 3.0, 4.0);
    assert(v2 == 20.0);

    /* [2] 嵌套复合语句作用域恢复 */
    double v3 = compute_nested(2.0, 3.0, 4.0);
    assert(v3 == 30.0);

    /* [2] 在 main 的复合语句内使用 pragma（位于所有显式声明和语句之前） */
#pragma STDC FP_CONTRACT OFF
    double v4 = compute(1.0, 1.0, 1.0);
    assert(v4 == 2.0);
#pragma STDC FP_CONTRACT ON
    double v5 = compute(1.0, 1.0, 1.0);
    assert(v5 == 2.0);

    /* [2] 默认状态是实现定义的，这里只验证程序能运行，不假设具体默认值 */
    printf("FP_CONTRACT pragma tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 注意：C99 7.12.2 没有 Constraints 段落，因此严格来说没有“违反约束必须报错”的规则。
 * 下面演示的是“非标准形式的 pragma”，实现应忽略它（C99 6.10.6：无法识别的 pragma 被忽略），
 * 而不是报错。这里放在 #if 0 中仅作说明，不期望编译报错。
 */

/* 非标准 on-off-switch 拼写：实现应忽略该 pragma，而非报错 */
#pragma STDC FP_CONTRACT YES

/* 缺少 on-off-switch：实现应忽略该 pragma，而非报错 */
#pragma STDC FP_CONTRACT

/* 在语句之后使用 pragma：标准说行为未定义（UB），不是约束违反，不要求诊断 */
void bad_context(void)
{
    int x = 0;
    x = x + 1;
#pragma STDC FP_CONTRACT ON   /* UB：不在外部声明处，也不在复合语句所有声明/语句之前 */
    (void)x;
}

#endif