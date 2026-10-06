/*
 * 验证 C99 6.7.1 Storage-class specifiers
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：测试所有合法的存储类说明符 */
typedef int MyInt;       /* typedef */
extern int ext_var;      /* extern */
static int sta_var = 1;  /* static */

int ext_var = 10;        /* extern 变量的定义 */

/* [3] 语义：typedef 仅为语法便利被称为存储类说明符，用于定义类型别名 */
MyInt typedef_var = 5;

/* [4] 语义：register 建议快速访问，实现可将其视为 auto，但可正常访问其值 */
int test_register(void) {
    register int reg_var = 42;
    return reg_var;
}

/* [6] 语义：聚合体声明为 static，其属性（静态存储期）递归传递给成员，因此成员默认初始化为 0 */
static struct {
    int member_a;
    struct {
        int inner_b;
    } nested;
} agg_var;

int main(void) {
    /* [1] & [3] 验证 typedef 和其他说明符声明的变量 */
    assert(typedef_var == 5);
    assert(ext_var == 10);
    assert(sta_var == 1);

    /* [4] 验证 register 变量可正常读取 */
    assert(test_register() == 42);

    /* [6] 验证 static 聚合体成员被零初始化（静态存储期属性传递给成员） */
    assert(agg_var.member_a == 0);
    assert(agg_var.nested.inner_b == 0);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「最多只能有一个存储类说明符」：static 和 extern 不能同时使用 */
static extern int x1;

/* [2] 违反约束「最多只能有一个存储类说明符」：auto 和 register 不能同时使用 */
auto register int x2;

/* [5] 违反约束「块作用域函数声明除 extern 外不得有显式存储类说明符」：使用了 static */
void block_scope_func_test1(void) {
    static void f(void);
}

/* [5] 违反约束「块作用域函数声明除 extern 外不得有显式存储类说明符」：使用了 auto */
void block_scope_func_test2(void) {
    auto void g(void);
}

/* 脚注103 / 6.5.3.2 违反约束「不能对 register 变量取地址」：显式使用 & 运算符 */
void test_register_address(void) {
    register int r = 10;
    int *p = &r; /* gcc -std=c99 应报错：address of register variable requested */
}

#endif