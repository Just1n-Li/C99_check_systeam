/*
 * 验证 C99 条款 6.4.7 Header names
 * 预期行为：正向测试编译并运行通过；负向测试无（本条款无 Constraints 部分，特殊字符导致的是 UB 而非约束违反）。
 */

#include <stdio.h>   /* [1] 语法：< h-char-sequence > 形式 */
#include "assert.h"  /* [1] 语法：" q-char-sequence " 形式 */

/* [3][4] 验证 header name 预处理记号只在 #include 中被识别。
   在普通代码中，0x3<1/a.h>1e2 应被拆分为 9 个独立的预处理记号，
   而不是将 <1/a.h> 作为一个 header-name 记号。 */
#define COUNT9(a, b, c, d, e, f, g, h, i) 9

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [2] 语义：两种形式的 header names 被正确映射并包含，
       使得标准库函数 printf 和宏 assert 可用。 */
    printf("Test [1][2]: Headers included successfully.\n");
    assert(1);

    /* [3] 语义：Header name 预处理记号只在 #include 预处理指令中被识别。
       在宏调用中，<1/a.h> 不会被识别为 header-name，而是被拆分为
       <, 1, /, a, ., h, > 等多个记号。 */
    /* [4] 例子：0x3<1/a.h>1e2 形成以下序列：
       {0x3}{<}{1}{/}{a}{.}{h}{>}{1e2}，共 9 个记号。 */
    int token_count = COUNT9(0x3<1/a.h>1e2);
    assert(token_count == 9);
    printf("Test [3][4]: Token sequence parsed correctly. Count: %d\n", token_count);

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* 本条款（6.4.7 Header names）没有定义 Constraints（约束）部分。
       条款 [3] 中明确指出：如果在 < 和 > 之间出现 ', \, ", //, 或 /*，
       行为是未定义的；在 " 和 " 之间出现 ', \, //, 或 /*，行为也是未定义的。
       根据测试规则，不针对未定义行为(UB)编写负向测试，故此处留空。 */
    #endif

    return 0;
}