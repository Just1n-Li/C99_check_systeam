/*
 * 验证 C99 条款 6.4.2.1 (标识符 General)
 * 预期行为：
 * 正向测试：能编译并运行通过，assert 验证语义。
 * 负向测试：违反约束，编译器应报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：标识符由非数字字符（含下划线、大小写字母）和数字组成 */
int _start_with_underscore = 1;
int start_with_letter = 2;
int contain_digits_123 = 3;

/* [2] 语义：大小写字母是不同的 */
int caseTest = 10;
int CASETEST = 20;

/* [2] 语义：标识符最大长度没有特定限制（测试一个较长的标识符，至少超过 C90 的 31 字符限制） */
int aVeryLongIdentifierThatExceedsTheOldMinimumLimitOfThirtyOneCharacters = 100;

/* [3] 语义：通用字符名 (UCN) 可用于标识符，且首字符不是数字 */
int var\u00FC = 5; /* \u00FC 是 'ü'，属于合法的字母范围 */

int main(void) {
    /* [1] 验证基本标识符语法 */
    assert(_start_with_underscore == 1);
    assert(start_with_letter == 2);
    assert(contain_digits_123 == 3);

    /* [2] 验证大小写敏感 */
    assert(caseTest != CASETEST);
    assert(caseTest == 10);
    assert(CASETEST == 20);

    /* [2] 验证长标识符 */
    assert(aVeryLongIdentifierThatExceedsTheOldMinimumLimitOfThirtyOneCharacters == 100);

    /* [3] 验证通用字符名作为标识符 */
    assert(var\u00FC == 5);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反语法约束：标识符不能以数字开头 */
int 1invalid;

/* [3] 违反约束：标识符首字符不能是表示数字的通用字符名 (\u0030 是 '0') */
int \u0030invalid;

/* [3] 违反约束：通用字符名必须落在 annex D 指定的范围内 (\u0001 是控制字符，不在允许范围内) */
int invalid\u0001;

/* [4] 违反语义/约束：预处理 token 如果能转换为关键字，则转换为关键字。关键字不能作为标识符 */
int int;
int for;
int while;

/* [6] 注意：如果两个标识符仅在非有效字符上不同，行为是未定义(UB)，不是约束违规，故不作为负向测试 */
#endif