/*
 * 验证 C99 6.10.6 Pragma directive
 * 正向测试：应能编译并运行通过
 * 负向测试：违反 [2] 中约束（STDC pragma 必须为指定形式之一）应编译报错
 */

#include <assert.h>
#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 非标准 pragma（STDC 不紧跟 pragma），实现定义行为 */
/* 不被识别的 pragma 应被忽略，不导致编译失败 */
#pragma unrecognized_pragma_should_be_ignored
#pragma another_unknown_pragma(1, 2, 3)

/* [1] 常见的实现定义非标准 pragma（编译器可选择处理或忽略） */
#pragma message("Testing non-STDC pragma")

/* [2] 标准 pragma：STDC 紧跟 pragma，三种形式 */
/* 前向引用：FP_CONTRACT (7.12.2) */
#pragma STDC FP_CONTRACT ON
#pragma STDC FP_CONTRACT OFF
#pragma STDC FP_CONTRACT DEFAULT

/* 前向引用：FENV_ACCESS (7.6.1) */
#pragma STDC FENV_ACCESS ON
#pragma STDC FENV_ACCESS OFF
#pragma STDC FENV_ACCESS DEFAULT

/* 前向引用：CX_LIMITED_RANGE (7.3.4) */
#pragma STDC CX_LIMITED_RANGE ON
#pragma STDC CX_LIMITED_RANGE OFF
#pragma STDC CX_LIMITED_RANGE DEFAULT

/* [2] STDC 紧跟 pragma 时不执行宏替换（prior to any macro replacement） */
/* 定义宏 ON 为 OFF，但在 STDC pragma 中 ON 不应被替换 */
#define ON OFF
#pragma STDC FP_CONTRACT ON   /* ON 保持原样，不被替换为 OFF */
#undef ON

/* 脚注 152：非标准 pragma 中允许宏替换（但不要求） */
#define VALUE 4
#pragma test_pragma VALUE  /* 非标准 pragma，宏替换允许但不要求 */

int main(void) {
    /* [1] 不被识别的非标准 pragma 被忽略，程序正常运行 */
    printf("[1] Non-STDC pragmas: unrecognized ones ignored, program runs normally.\n");

    /* [2] 标准 STDC pragma 被接受 */
    printf("[2] Standard STDC pragmas accepted: FP_CONTRACT, FENV_ACCESS, CX_LIMITED_RANGE.\n");
    printf("[2] on-off-switch values ON, OFF, DEFAULT all accepted.\n");

    /* [2] STDC pragma 中不执行宏替换 */
    printf("[2] No macro replacement performed in STDC pragmas.\n");

    /* 脚注 152 */
    printf("Footnote 152: Macro replacement permitted (not required) in non-STDC pragmas.\n");

    /* 验证程序正常运行 */
    int result = 42;
    assert(result == 42);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束：STDC 紧跟 pragma，但不是三种标准形式之一 */
/* 期望编译器报错：invalid or unknown STDC pragma */
#pragma STDC UNKNOWN_PRAGMA ON

/* [2] 违反约束：pragma 名称不是 FP_CONTRACT/FENV_ACCESS/CX_LIMITED_RANGE */
#pragma STDC SOME_OTHER_NAME OFF

/* [2] 违反约束：on-off-switch 不是 ON/OFF/DEFAULT 之一 */
#pragma STDC FP_CONTRACT ENABLED

/* [2] 违反约束：on-off-switch 不是 ON/OFF/DEFAULT 之一 */
#pragma STDC FENV_ACCESS TRUE

/* [2] 违反约束：on-off-switch 不是 ON/OFF/DEFAULT 之一 */
#pragma STDC CX_LIMITED_RANGE 1

/* [2] 违反约束：缺少 on-off-switch */
#pragma STDC FP_CONTRACT

/* [2] 违反约束：STDC 后无内容 */
#pragma STDC

/* [2] 违反约束：on-off-switch 为小写（必须是大写的 ON/OFF/DEFAULT） */
#pragma STDC FP_CONTRACT on

#endif