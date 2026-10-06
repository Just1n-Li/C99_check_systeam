/*
 * 验证 C99 条款 6.8.4.2: The switch statement
 * 预期行为：
 * - 正向测试：编译并运行通过，assert 验证语义正确。
 * - 负向测试：编译报错（违反 Constraints）。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 约束：控制表达式为整数类型 */
void test_integer_controlling_expr(void) {
    int i = 2;
    int r = 0;
    switch (i) {
        case 1: r = 1; break;
        case 2: r = 2; break;
        default: r = -1; break;
    }
    assert(r == 2);
}

/* [1] 约束：枚举类型也是整数类型 */
void test_enum_controlling_expr(void) {
    enum Color { RED, GREEN, BLUE } c = GREEN;
    int r = 0;
    switch (c) {
        case RED: r = 1; break;
        case GREEN: r = 2; break;
        case BLUE: r = 3; break;
    }
    assert(r == 2);
}

/* [2] 约束：变长修改类型(VMT)作用域内的 switch */
void test_vmt_scope_valid(void) {
    int n = 5;
    {
        int vla[n]; /* VMT */
        switch (n) {
            case 5:
                vla[0] = 42;
                break;
            default:
                break;
        }
        assert(vla[0] == 42);
    }
}

/* [3] 约束：case 标签为整型常量表达式，值不重复，最多一个 default */
void test_case_labels_valid(void) {
    int x = 10;
    int r = 0;
    switch (x) {
        case 5: r = 1; break;
        case 10: r = 2; break;
        case 15: r = 3; break;
        default: r = -1; break;
    }
    assert(r == 2);
}

/* [4] 语义：跳转到匹配的 case、default 或跳过 switch 体 */
void test_jump_semantics(void) {
    int x = 99;
    int r = 0;
    
    /* 测试无匹配且无 default，不执行任何部分 */
    switch (x) {
        case 1: r = 1; break;
        case 2: r = 2; break;
    }
    assert(r == 0);
    
    /* 测试跳转到 default */
    switch (x) {
        case 1: r = 1; break;
        default: r = -1; break;
    }
    assert(r == -1);
}

/* [4] 语义：case/default 标签只在最内层 switch 可访问 */
void test_nested_switch_scope(void) {
    int x = 1, y = 2;
    int r = 0;
    switch (x) {
        case 1:
            switch (y) {
                case 2: r = 12; break;
                default: r = 10; break;
            }
            break;
        /* 这里的 case 2 属于外层 switch */
        case 2: r = 20; break;
    }
    assert(r == 12);
}

/* [5] 语义：整数提升与 case 常量转换 */
void test_integer_promotion(void) {
    char c = 'A';
    int r = 0;
    /* char 被提升为 int，case 常量也转换为 int */
    switch (c) {
        case 'A': r = 1; break;
        case 66: r = 2; break; /* 'B' */
        default: r = -1; break;
    }
    assert(r == 1);
}

/* [7] 示例：fall-through 与跳过初始化（避免 UB 的版本） */
void test_example_fall_through(void) {
    int expr = 0;
    int result = -1;
    switch (expr) { 
        int i = 4; 
        /* f(i); 无法到达 */
        case 0: i = 17; /* falls through into default code */ 
        default: result = i; /* 此时 i 已被赋值为 17 */ 
    }
    assert(result == 17);
}

int main(void) {
    test_integer_controlling_expr();
    test_enum_controlling_expr();
    test_vmt_scope_valid();
    test_case_labels_valid();
    test_jump_semantics();
    test_nested_switch_scope();
    test_integer_promotion();
    test_example_fall_through();
    
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [1] 违反约束：控制表达式必须为整数类型。浮点数不能用于 switch */
void test_switch_float(void) {
    float f = 1.0f;
    switch (f) { /* 期望报错：控制表达式非整数类型 */
        case 1: break;
    }
}

/* [1] 违反约束：控制表达式必须为整数类型。指针不能用于 switch */
void test_switch_pointer(void) {
    int *p = 0;
    switch (p) { /* 期望报错：控制表达式非整数类型 */
        case 0: break;
    }
}

/* [1] 违反约束：控制表达式必须为整数类型。结构体不能用于 switch */
struct S { int x; };
void test_switch_struct(void) {
    struct S s = {0};
    switch (s) { /* 期望报错：控制表达式非整数类型 */
        case 0: break;
    }
}

/* [2] 违反约束：case 标签在变长修改类型(VMT)作用域内，但 switch 关键字不在该作用域内 */
void test_vmt_scope_violation(void) {
    int n = 5;
    switch (n) { /* switch 关键字在 vla 作用域外 */
        case 5:
            {
                int vla[n]; /* VMT */
                case 6: break; /* 期望报错：case 标签在 VMT 作用域内，但 switch 不在 */
            }
            break;
    }
}

/* [3] 违反约束：case 标签必须是整型常量表达式 */
void test_case_not_constant(void) {
    int x = 1;
    int y = 2;
    switch (x) {
        case y: break; /* 期望报错：y 不是常量表达式 */
    }
}

/* [3] 违反约束：同一 switch 中 case 常量值不能重复 */
void test_case_duplicate(void) {
    int x = 1;
    switch (x) {
        case 1: break;
        case 1: break; /* 期望报错：重复的 case 常量值 */
    }
}

/* [3] 违反约束：最多只能有一个 default 标签 */
void test_multiple_default(void) {
    int x = 1;
    switch (x) {
        default: break;
        default: break; /* 期望报错：重复的 default 标签 */
    }
}

#endif