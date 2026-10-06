/*
 * 验证 C99 6.10.7 Null directive（空指令）
 * 语义 [1]：形式为 # new-line 的预处理指令没有任何效果。
 *
 * 正向测试：程序应能编译并运行通过，验证 # 空指令不影响编译和运行。
 * 负向测试：本条款仅含 Semantics 段落，无 Constraints 段落，故无负向测试。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 空指令 # 在文件开头，无效果 */
#

/* [1] 空指令 # 在 #include 之后 */
#

/* [1] 空指令 # 在函数定义之间 */
#

/* [1] 空指令 # 紧跟在 #define 之后 */
#define MACRO_FORTY_TWO 42
#

int main(void)
{
    /* [1] 空指令 # 在函数体内语句之前 */
    #

    /* [1] 空指令不影响变量定义与赋值 */
    int x = MACRO_FORTY_TWO;
    #
    assert(x == 42);

    /* [1] 空指令不影响表达式计算 */
    #
    int y = x + 1;
    assert(y == 43);

    /* [1] 空指令不影响控制流 */
    #
    if (x == 42) {
        #
        assert(1);
        #
    } else {
        assert(0); /* 不应到达 */
    }

    /* [1] 多个连续空指令，均无效果 */
    #
    #
    #
    assert(x + y == 85);

    /* [1] 空指令穿插在循环中，无效果 */
    #
    int sum = 0;
    for (int i = 1; i <= 10; i++) {
        #
        sum += i;
        #
    }
    #
    assert(sum == 55);

    /* [1] 空指令穿插在宏使用中，无效果 */
    #
    assert(MACRO_FORTY_TWO == 42);
    #

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
/*
 * 本条款 6.10.7 仅包含 Semantics 段落，不包含 Constraints 段落。
 * 空指令 # 本身没有任何约束条件，因此无负向测试可写。
 *
 * 唯一相关的约束来自 6.10（预处理指令通用约束），而非 6.10.7 本身：
 * 例如 # 后跟非法标记等，但那属于 6.10 的约束，不属于本条款范围。
 */
#if 0
/* 6.10.7 无 Constraints —— 此处留空 */
#endif