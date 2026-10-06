/*
 * 测试 C99 6.10.5 —— #error 指令
 *
 * 预期行为：
 *   正向测试：不触发 #error 时，程序应能正常编译并运行通过。
 *   负向测试：一旦预处理遇到 #error，实现必须产生一条诊断消息，
 *             该消息中应包含 #error 之后给出的预处理记号序列。
 *
 * 说明：由于 #error 是预处理期指令，无法在同一个翻译单元里既触发它
 *       又让程序正常编译运行，因此负向测试统一放在 #if 0 ... #endif
 *       中（被条件编译屏蔽，不会真正触发），仅作为“应报错”的示例。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] #error 只在被预处理到（即条件为真）时才产生诊断。
 *     下面用条件编译保证 #error 分支不被选中，因此不产生诊断，
 *     程序应正常编译运行。 */
#if 0
#error this branch must not be taken
#endif

/* [1] 再验证一次：条件为假时 #error 被跳过。 */
#ifdef NEVER_DEFINED_MACRO
#error this must never be reached
#endif

/* [1] #error 的 pp-tokens 可以是任意预处理记号序列，
 *     包括字符串、数字、标点等；这里同样放在未选中的分支里。 */
#if 0
#error "unreachable" 123 + - * / ( ) [ ] { } , ; : ? ~ ! @ # $ % ^ & | \ 
#endif

int main(void)
{
    /* 能运行到这里，说明上面的 #error 都没有被触发 */
    printf("C99 6.10.5 positive test: no #error triggered, program runs.\n");

    /* 简单断言，确认程序确实执行 */
    int x = 1 + 1;
    assert(x == 2);

    printf("positive test passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/*
 * [1] 违反约束/语义：当预处理到达 #error 指令时，实现必须产生
 *     一条诊断消息，且该消息应包含 #error 之后的预处理记号序列。
 *
 * 期望：gcc -std=c99 -c 该文件（去掉 #if 0 后）应报错，例如：
 *   error: #error "this is a deliberate error"
 * 或类似包含 "this is a deliberate error" 的诊断信息。
 */
#error "this is a deliberate error"

/*
 * [1] 期望诊断消息包含指定的记号序列（这里是若干记号）。
 * 期望：编译器报错，诊断中包含 these tokens must appear。
 */
#error these tokens must appear

/*
 * [1] 期望诊断消息包含数字与标点记号序列。
 * 期望：编译器报错，诊断中包含 123 + 456。
 */
#error 123 + 456

/*
 * [1] 即使 #error 出现在条件为真的分支中，也必须产生诊断。
 * 期望：编译器报错。
 */
#if 1
#error conditional branch taken, must diagnose
#endif

#endif /* 负向测试结束 */