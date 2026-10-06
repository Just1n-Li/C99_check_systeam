/*
 * 验证 C99 条款 6.10.4 Line control
 * 预期行为：
 * - 正向测试：能编译并运行通过，assert 验证 __LINE__ 和 __FILE__ 的值。
 * - 负向测试：编译报错（违反 Constraints）。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] 当前行号是读取或引入的换行符数量加 1。
 * 通过验证相邻两行的 __LINE__ 差值为 1，体现行号随换行符递增的语义。
 */
static void test_semantics_2(void) {
    int line_a = __LINE__;
    int line_b = __LINE__;
    assert(line_b - line_a == 1);
}

/* [3] #line digit-sequence 设置假定行号。
 * digit-sequence 解释为十进制整数，不能为 0，不能大于 2147483647。
 */
#line 100
static void test_semantics_3(void) {
    int current_line = __LINE__; /* #line 100 的下一行是 101 */
    assert(current_line == 101);
}

/* [4] #line digit-sequence "s-char-sequence" 设置假定行号和假定文件名。
 * 改变 presumed name of the source file。
 */
#line 200 "custom_test_file.c"
static void test_semantics_4(void) {
    int current_line = __LINE__; /* #line 200 的下一行是 201 */
    assert(current_line == 201);
    /* __FILE__ 应包含 "custom_test_file.c" */
    assert(strstr(__FILE__, "custom_test_file.c") != NULL);
}

/* [5] #line pp-tokens 允许宏展开。
 * 展开后应匹配前两种形式。
 */
#define LINE_NUM 300
#define FILE_NAME "macro_test_file.c"
#line LINE_NUM FILE_NAME
static void test_semantics_5(void) {
    int current_line = __LINE__; /* #line 300 的下一行是 301 */
    assert(current_line == 301);
    assert(strstr(__FILE__, "macro_test_file.c") != NULL);
}

/* 恢复行号和文件名，避免影响后续代码 */
#line 1 "main.c"

int main(void) {
    test_semantics_2();
    test_semantics_3();
    test_semantics_4();
    test_semantics_5();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [1] 违反约束「字符串字面量必须是字符字符串字面量」：
 * 不能使用宽字符串字面量 L"..."。
 * 期望编译器报错，例如 "invalid string literal in #line directive"
 */
#line 100 L"wide_file.c"

/* [3] 违反约束「digit-sequence 不能为 0」：
 * 期望编译器报错，例如 "line number out of range" 或 "invalid line number"
 */
#line 0

/* [3] 违反约束「digit-sequence 不能大于 2147483647」：
 * 2147483648 超过了 2147483647。
 * 期望编译器报错，例如 "line number out of range"
 */
#line 2147483648

/* [5] 违反约束「宏展开后应匹配前两种形式」：
 * 展开后为字符串，缺少数字序列。
 * 期望编译器报错，例如 "invalid #line directive"
 */
#define BAD_MACRO_1 "only_string.c"
#line BAD_MACRO_1

/* [5] 违反约束「宏展开后应匹配前两种形式」：
 * 展开后为多个数字，不符合形式。
 * 期望编译器报错，例如 "invalid #line directive"
 */
#define BAD_MACRO_2 100 200
#line BAD_MACRO_2

#endif