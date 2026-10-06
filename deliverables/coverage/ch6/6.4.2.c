/*
 * 验证 C99 条款 6.4.2 Identifiers
 * 预期行为：
 * 正向测试：能编译并运行通过，验证标识符的语义（合法字符、大小写敏感、通用字符名等）。
 * 负向测试：编译报错，验证标识符的约束（不能以数字开头、不能仅由通用字符名组成、不能包含非法字符）。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [4] 语义：标识符是非数字字符（包括下划线 _）和数字的序列 */
int valid_identifier_1 = 1;
int _valid_identifier_2 = 2;
int identifier123 = 3;

/* [5] 语义：小写字母和大写字母是不同的 */
int case_test = 10;
int CASE_TEST = 20;

/* [4] 语义：包含通用字符名的标识符（非全部由通用字符名组成） */
/* 使用通用字符名 \u00E9 (é) 作为标识符的一部分 */
int var\u00E9 = 30;

int main(void) {
    /* [4] 验证合法标识符 */
    assert(valid_identifier_1 == 1);
    assert(_valid_identifier_2 == 2);
    assert(identifier123 == 3);
    printf("[4] Valid identifiers test passed.\n");

    /* [5] 验证大小写敏感 */
    assert(case_test != CASE_TEST);
    assert(case_test == 10);
    assert(CASE_TEST == 20);
    printf("[5] Case sensitivity test passed.\n");

    /* [4] 验证包含通用字符名的标识符 */
    assert(var\u00E9 == 30);
    printf("[4] Identifier with universal character name test passed.\n");

    /* [6][7] 语义：保留标识符（以 _ 加大写字母或另一个 _ 开头，或以 _ 开头用于文件作用域）。
     * 注意：使用保留标识符属于未定义行为(UB)，而非约束违反，编译器不一定要报错。
     * 这里在块作用域内使用以 _ 开头加小写字母的标识符，通常不会导致编译失败。
     */
    int _local_var = 40;
    assert(_local_var == 40);
    printf("[6][7] Reserved identifiers note acknowledged.\n");

    printf("All positive tests passed!\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* [1] 约束：标识符不能以数字开头。
     * 期望报错：例如 "expected identifier or '(' before numeric constant"
     */
    int 1invalid_identifier;

    /* [2] 约束：标识符不能仅由通用字符名组成。
     * 期望报错：例如 "an identifier cannot consist solely of universal-character-name"
     */
    int \u00E9;

    /* [3] 约束：标识符不能包含语法中未指定的字符（如 '$'）。
     * 期望报错：例如 "stray '$' in program" 或 "expected ';', '$' token"
     */
    int invalid$identifier;

#endif

    return 0;
}