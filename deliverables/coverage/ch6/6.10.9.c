/*
 * 验证 C99 条款 6.10.9 Pragma operator
 * 预期行为：正向测试编译并运行通过，负向测试编译报错。
 */

#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 基本形式：_Pragma ( string-literal )，验证删除首尾双引号并作为 pragma 执行 */
_Pragma("STDC FP_CONTRACT ON")

/* [1] 删除 L 前缀（如果存在）：宽字符串字面量应被同样处理 */
_Pragma(L"STDC FP_CONTRACT OFF")

/* [1] 替换转义序列 \" 为双引号，替换 \\ 为单反斜杠 */
/* 使用非标准 pragma 仅验证 destringize 过程不报错，编译器通常会忽略未知 pragma */
_Pragma("listing on \"..\\listing.dir\"")

/* [2] EXAMPLE: 宏替换形式，验证 _Pragma 结果来自宏替换时同样处理 */
#define LISTING(x) PRAGMA(listing on #x)
#define PRAGMA(x) _Pragma(#x)
LISTING ( ..\listing.dir )

int main(void) {
    /* 如果程序能编译并运行到这里，说明 _Pragma 操作符被正确处理 */
    printf("Pragma operator tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束「_Pragma 操作数必须是字符串字面量」：整数不能作为操作数，应报错 */
_Pragma(123)

/* 违反约束「_Pragma 操作数必须是字符串字面量」：标识符不能作为操作数，应报错 */
_Pragma(hello)
#endif