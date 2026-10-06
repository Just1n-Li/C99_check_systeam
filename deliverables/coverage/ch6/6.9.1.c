/*
 * 验证 C99 6.9.1 Function definitions
 * 预期行为：正向测试编译并运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：declaration-specifiers declarator compound-statement */
/* [13] EXAMPLE 1: 原型形式，extern 是存储类说明符，int 是类型说明符 */
extern int max_proto(int a, int b) {
    return a > b ? a : b;
}

/* [13] EXAMPLE 1: 标识符列表形式，int a, b; 是声明列表 */
extern int max_idlist(a, b)
    int a, b;
{
    return a > b ? a : b;
}

/* [7] 语义：参数类型列表作为原型，强制转换。测试 char->int, float->double 的默认实参提升 */
int sum_proto(char c, float f) {
    /* 参数 c 被提升为 int 后又转回 char，f 被提升为 double 后又转回 float */
    return (int)c + (int)f;
}

/* [8] 语义：变参函数必须以省略号结尾的参数类型列表定义 */
int sum_var(int n, ...) {
    va_list args;
    va_start(args, n);
    int total = 0;
    for (int i = 0; i < n; ++i) {
        total += va_arg(args, int);
    }
    va_end(args);
    return total;
}

/* [9] 语义：参数是左值，具有自动存储期，可在嵌套块中重新声明 */
int test_lvalue(int p) {
    p = 10; /* 参数是左值，可修改 */
    {
        int p = 20; /* 嵌套块中重新声明，合法 */
        assert(p == 20);
    }
    return p;
}

/* [10] 语义：变长数组参数大小表达式求值，实参转换（数组转指针） */
int test_vla(int n, int vla[n]) {
    return vla[0] + vla[n-1];
}

/* [14] EXAMPLE 2: 传递函数给函数 */
int f(void) { return 42; }
void g(int (*funcp)(void)) {
    assert((*funcp)() == 42);
}
void g2(int func(void)) {
    assert(func() == 42);
}

int main(void) {
    /* [11] 语义：参数赋值后执行函数体 */
    /* [13] 测试 EXAMPLE 1 */
    assert(max_proto(3, 5) == 5);
    assert(max_idlist(10, 2) == 10);

    /* [7] 测试原型转换：char 提升，float 提升 */
    char c = 'A';
    float fl = 1.5f;
    assert(sum_proto(c, fl) == (int)'A' + 1);

    /* [8] 测试变参 */
    assert(sum_var(3, 10, 20, 30) == 60);

    /* [9] 测试参数左值 */
    assert(test_lvalue(5) == 10);

    /* [10] 测试 VLA 和实参转换 */
    int arr[] = {1, 2, 3, 4, 5};
    assert(test_vla(5, arr) == 6);

    /* [14] 测试 EXAMPLE 2 */
    g(f);
    g2(f);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「标识符必须具有函数类型」：typedef 名不能直接用于函数定义 (Footnote 141) */
typedef int F(void);
F f_def { /* ... */ } /* WRONG: syntax/constraint error */

/* [3] 违反约束「返回类型不能是数组类型」 */
int arr_func(void)[5] { /* ... */ }

/* [4] 违反约束「存储类说明符只能是 extern 或 static」 */
auto int auto_func(void) { return 0; }
register int reg_func(void) { return 0; }

/* [5] 违反约束「参数类型列表中参数必须包含标识符（除单个 void）」 */
int no_id(int) { return 0; }

/* [5] 违反约束「参数类型列表后不能跟声明列表」 */
int with_decl_list(int a)
int a;
{ return 0; }

/* [6] 违反约束「标识符列表形式：声明列表中的声明没有声明符」 */
int idlist_no_declarator(a, b)
int;
{ return 0; }

/* [6] 违反约束「标识符列表形式：声明了不在标识符列表中的标识符」 */
int idlist_extra_id(a)
int a, c;
{ return 0; }

/* [6] 违反约束「标识符列表形式：标识符列表中的标识符未声明」 */
int idlist_undeclared(a, b)
int a;
{ return 0; }

/* [6] 违反约束「typedef 名不能重新声明为参数」 */
typedef int T;
int idlist_typedef(T)
int T;
{ return 0; }

/* [6] 违反约束「声明列表中不能有除 register 外的存储类说明符」 */
int idlist_static(a)
static int a;
{ return 0; }

/* [6] 违反约束「声明列表中不能有初始化」 */
int idlist_init(a)
int a = 1;
{ return 0; }

#endif