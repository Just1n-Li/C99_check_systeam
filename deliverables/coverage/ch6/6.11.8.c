/*
 * 验证 C99 6.11.8 Pragma directives
 * [1] Pragmas whose first preprocessing token is STDC are reserved for future standardization.
 *
 * 预期行为：
 * 正向测试：使用标准定义的 STDC pragma，能正常编译并运行通过。
 * 负向测试：本条款属于未来保留方向，无强制约束，故无负向测试。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
/* [1] 验证标准定义的 STDC pragma 可以正常使用，体现 STDC 命名空间被标准保留并使用 */
#pragma STDC FP_CONTRACT ON
#pragma STDC FP_CONTRACT OFF
#pragma STDC FENV_ACCESS ON
#pragma STDC FENV_ACCESS OFF
#pragma STDC CX_LIMITED_RANGE ON
#pragma STDC CX_LIMITED_RANGE OFF

int main(void) {
    int x = 42;
    assert(x == 42);
    printf("Standard STDC pragmas accepted and program runs successfully.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 6.11.8 条款规定以 STDC 开头的 pragma 保留给未来标准化。
 * 该条款属于 "Future language directions"，并未定义强制约束。
 * 使用未知的 STDC pragma 属于未定义行为(UB)，而非约束违反。
 * 根据测试要求，UB 不作为负向测试，故此处无约束负向测试。
 * 以下代码仅为演示 UB，编译器可能仅发出警告而非报错：
 */
#pragma STDC UNKNOWN_FUTURE_PRAGMA ON
#endif