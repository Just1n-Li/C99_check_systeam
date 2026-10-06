/*
 * 验证 C99 条款 6.4.3: Universal character names
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [3] 在标识符中使用通用字符名 */
    int \u4e2d = 1; /* \u4e2d 对应 '中' */
    int \U00004e2d = 2; /* \U00004e2d 也对应 '中' */
    assert(\u4e2d == 1);
    assert(\U00004e2d == 2);

    /* [2] 例外字符 0024 ($), 0040 (@), 0060 (') 允许使用 */
    assert('\u0024' == '$');
    assert('\u0040' == '@');
    assert('\u0060' == '`');

    /* [3] 在字符常量中使用通用字符名 */
    /* [4] \u 和 \U 的等价性：\unnnn 等价于 \U0000nnnn */
    assert('\u00E9' == '\U000000E9');

    /* [3] 在字符串字面量中使用通用字符名 */
    const char *s1 = "\u00E9";
    const char *s2 = "\U000000E9";
    assert(strcmp(s1, s2) == 0);

    printf("6.4.3 正向测试通过\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [2] 违反约束：指定小于 00A0 的字符（非例外字符），例如 \u0041 (A)，gcc -std=c99 应报错 */
char c1 = '\u0041';

/* [2] 违反约束：指定 D800 到 DFFF 范围内的字符，例如 \uD800，gcc -std=c99 应报错 */
char c2 = '\uD800';

/* [1] 语法错误：\u 后不足 4 位十六进制数字，gcc -std=c99 应报错 */
char c3 = '\u12';

/* [1] 语法错误：\U 后不足 8 位十六进制数字，gcc -std=c99 应报错 */
char c4 = '\U0012';
#endif