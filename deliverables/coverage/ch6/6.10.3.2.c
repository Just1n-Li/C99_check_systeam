/* 验证 C99 6.10.3.2 The # operator */
/* 预期行为：正向测试运行通过，负向测试编译报错 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] 基本功能：# 参数 -> "参数拼写" */
#define STR(x) #x

/* [2] 空实参对应的字符串字面量是 "" */
#define EMPTY(x) #x

void test_positive(void) {
    /* [2] 基本功能：将参数转换为字符串字面量 */
    const char *s1 = STR(hello);
    assert(strcmp(s1, "hello") == 0);

    /* [2] 空白处理：参数预处理记号之间的空白变为单个空格，首尾空白删除 */
    const char *s2 = STR(  hello   world  );
    assert(strcmp(s2, "hello world") == 0);

    /* [2] 字符串字面量转义：在 " 字符前插入 \ */
    const char *s3 = STR("hello");
    assert(strcmp(s3, "\"hello\"") == 0);

    /* [2] 字符常量转义：在 ' 字符前插入 \ */
    const char *s4 = STR('a');
    assert(strcmp(s4, "'a'") == 0);

    /* [2] 字符串字面量中的 \ 前插入 \ */
    const char *s5 = STR("\n");
    assert(strcmp(s5, "\"\\n\"") == 0);

    /* [2] 空实参对应的字符串字面量是 "" */
    const char *s6 = EMPTY();
    assert(strcmp(s6, "") == 0);

    printf("Positive tests passed.\n");
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束「# 后面必须跟着参数」：# 后面跟着非参数标识符 */
#define BAD1(x) #y
const char *b1 = BAD1(1);

/* [1] 违反约束「# 后面必须跟着参数」：# 后面跟着数字字面量 */
#define BAD2(x) #123
const char *b2 = BAD2(1);

/* [1] 违反约束「# 后面必须跟着参数」：# 后面没有记号 */
#define BAD3(x) #
const char *b3 = BAD3(1);

/* [1] 违反约束「# 后面必须跟着参数」：# 后面跟着运算符 */
#define BAD4(x) #+
const char *b4 = BAD4(1);
#endif

int main(void) {
    test_positive();
    return 0;
}