/*
 * 验证 C99 条款 6.2.1 (Scopes of identifiers)
 * 预期行为：
 * - 正向测试：能编译并运行通过，assert 验证成功。
 * - 负向测试：违反作用域约束，编译器应报错（undeclared identifier 等）。
 */

#include <stdio.h>
#include <assert.h>

/* [1] 标识符可以表示对象、函数、标签、成员、typedef名等 */
int file_scope_obj = 10; /* [4] 文件作用域，终止于翻译单元末尾 */
typedef int MyTypeDef; /* [1] typedef 名 */

/* [2] 函数原型作用域：参数 proto_param 的作用域终止于函数声明符末尾 */
int proto_func(int proto_param);

/* [7] 结构体标签作用域：标签 Node 在类型说明符出现后立即可见 */
struct Node {
    struct Node *next; /* 标签 Node 可见 */
    int data;
};

/* [7] 枚举常量作用域：枚举常量在枚举符出现后立即可见 */
enum Color {
    RED,
    GREEN = RED + 1, /* RED 已可见 */
    BLUE = GREEN + 1 /* GREEN 已可见 */
};

/* [4] 内层作用域隐藏外层作用域测试准备 */
int shadow_var = 1;

/* [3] 标签具有函数作用域，可在声明前使用 */
int label_test(int cond) {
    if (cond) goto my_label; /* [3] 在声明前使用标签 */
    return 0;
my_label: /* [3] 标签隐式声明，具有函数作用域 */
    return 1;
}

int main(void) {
    /* [1] 同一标识符在不同点表示不同实体：MyTypeDef 作为类型名，这里作为对象名 */
    int MyTypeDef = 5; 
    assert(MyTypeDef == 5);

    /* [2] 块作用域：block_var 终止于关联块末尾 */
    int block_var = 20;
    assert(block_var == 20);

    /* [4] 内层作用域隐藏外层作用域 */
    assert(shadow_var == 1);
    {
        int shadow_var = 2; /* 隐藏外层的 shadow_var */
        assert(shadow_var == 2);
    }
    assert(shadow_var == 1); /* 离开内层后，外层恢复可见 */

    /* [6] 相同作用域：a 和 b 的作用域终止于同一点（main 函数块末尾） */
    int a = 100;
    int b = 200;
    assert(a + b == 300);

    /* [7] 其他标识符作用域始于声明符完成之后 */
    int x = 10;
    int y = x; /* x 的声明符已完成，可见 */
    assert(y == 10);

    /* [3] 标签函数作用域测试 */
    assert(label_test(1) == 1);
    assert(label_test(0) == 0);

    /* [7] 枚举常量作用域测试 */
    assert(RED == 0);
    assert(GREEN == 1);
    assert(BLUE == 2);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 正向测试结束 ========== */


/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反 [4] 块作用域约束：在块外使用块内声明的标识符，gcc -std=c99 应报错 undeclared */
void block_scope_violation(void) {
    {
        int inner_var = 5;
    }
    inner_var = 10; /* 期望报错：'inner_var' undeclared */
}

/* 违反 [2] 函数原型作用域约束：在原型外使用参数名，gcc -std=c99 应报错 undeclared */
int proto_func(int proto_param);
void proto_scope_violation(void) {
    proto_param = 10; /* 期望报错：'proto_param' undeclared */
}

/* 违反 [3] 函数作用域约束：标签不能跨函数使用，gcc -std=c99 应报错 label undeclared */
void cross_function_goto_violation(void) {
    goto cross_label; /* 期望报错：'cross_label' undeclared */
}
void another_function(void) {
cross_label:
    ;
}

/* 违反 [7] 声明符完成前使用标识符约束：sizeof 操作时 arr 声明符未完成，gcc -std=c99 应报错 undeclared */
void declarator_completion_violation(void) {
    int arr[sizeof(arr)]; /* 期望报错：'arr' undeclared (在 sizeof 中不可见) */
}

#endif