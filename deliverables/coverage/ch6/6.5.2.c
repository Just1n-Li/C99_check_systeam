/*
 * 验证 C99 条款 6.5.2 Postfix operators (后缀运算符)
 * 预期行为：
 * 正向测试：能编译并运行通过，断言成功。
 * 负向测试：违反约束的代码片段在 #if 0 块中，若取消注释应编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

struct S {
    int x;
    double y;
};

/* 用于测试函数返回结构体（非左值）及成员访问 */
struct S get_s(void) {
    struct S s = {42, 3.14};
    return s;
}

/* 无原型函数，测试默认实参提升 */
int np_func() {
    va_list ap;
    va_start(ap, 0); /* C99 允许无原型函数使用 va_start，但不检查参数名 */
    int i = va_arg(ap, int);       /* char 提升为 int */
    double d = va_arg(ap, double); /* float 提升为 double */
    va_end(ap);
    return i + (int)d;
}

/* 省略号函数，测试省略号后停止转换 */
int ellipsis_func(int fixed, ...) {
    va_list ap;
    va_start(ap, fixed);
    int i = va_arg(ap, int);
    va_end(ap);
    return fixed + i;
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    int arr[3] = {10, 20, 30};
    int *p = arr;

    /* [1] 语法：postfix-expression [ expression ] */
    /* [6.5.2.1] 数组下标：E1[E2] 等价于 (*((E1)+(E2))) */
    assert(arr[1] == 20);
    assert(p[2] == 30);
    assert(2[arr] == 30); /* 指针和整数可交换位置 */

    /* [1] 语法：postfix-expression ( argument-expression-list ) */
    /* [6.5.2.2] 函数调用 */
    assert(ellipsis_func(10, 20) == 30);

    /* [6.5.2.2] 无原型函数调用的默认实参提升：char→int, float→double */
    char c = 'A';
    float f = 2.0f;
    assert(np_func(c, f) == 67); /* 65 + 2 = 67 */

    /* [1] 语法：postfix-expression . identifier */
    /* [1] 语法：postfix-expression -> identifier */
    /* [6.5.2.3] 结构体和联合体成员访问 */
    struct S s = {1, 2.0};
    struct S *sp = &s;
    assert(s.x == 1);
    assert(sp->y == 2.0);

    /* [6.5.2.3] 函数返回结构体，访问其成员（结果为非左值，但可读取） */
    assert(get_s().x == 42);

    /* [1] 语法：postfix-expression ++ / postfix-expression -- */
    /* [6.5.2.4] 后缀自增/自减：结果是值（非左值），且为自增前的旧值 */
    int i = 5;
    int old = i++;
    assert(old == 5);
    assert(i == 6);
    old = i--;
    assert(old == 6);
    assert(i == 5);

    /* [1] 语法：( type-name ) { initializer-list } */
    /* [6.5.2.5] 复合字面量：创建一个未命名的对象 */
    int *cl = (int[]){1, 2, 3};
    assert(cl[0] == 1 && cl[2] == 3);
    struct S *cs = (struct S){10, 20.0};
    assert(cs->x == 10);

    /* const/volatile 限定类型在成员访问上的传播 */
    const struct S cs2 = {100, 200.0};
    const struct S *csp = &cs2;
    assert(csp->x == 100); /* csp->x 的类型是 const int */

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [6.5.2.1 约束] 数组下标操作数必须一个是指针一个是整数，不能两个都是指针 */
void test_subscript_constraint(void) {
    int *p1, *p2;
    p1[p2]; /* 违反约束：两个操作数都是指针，gcc -std=c99 应报错 */
}

/* [6.5.2.2 约束] 函数调用的表达式必须是指向函数的指针 */
void test_call_constraint(void) {
    int x;
    x(); /* 违反约束：x 不是函数指针，gcc -std=c99 应报错 */
}

/* [6.5.2.3 约束] 点操作符第一个操作数必须是结构体/联合体类型 */
void test_dot_constraint(void) {
    int x;
    x.y; /* 违反约束：x 不是结构体，gcc -std=c99 应报错 */
}

/* [6.5.2.3 约束] 箭头操作符第一个操作数必须是指向结构体/联合体的指针 */
void test_arrow_constraint(void) {
    struct S s;
    s->x; /* 违反约束：s 不是指针，gcc -std=c99 应报错 */
}

/* [6.5.2.4 约束] 后缀自增/自减操作数必须是可修改的左值（算术或指针类型） */
void test_inc_constraint(void) {
    const int x = 10;
    x++; /* 违反约束：const 不可修改，gcc -std=c99 应报错 */
}

/* [6.5.2.4 约束] 后缀自增操作数不能是结构体（非算术非指针） */
void test_inc_struct_constraint(void) {
    struct S s = {1, 2.0};
    s++; /* 违反约束：结构体不能自增，gcc -std=c99 应报错 */
}

/* [6.5.2.3 语义] 函数返回结构体的成员是非左值，对它赋值应报错 */
void test_non_lvalue_assign_func(void) {
    get_s().x = 10; /* 违反约束：赋值给非左值，gcc -std=c99 应报错 */
}

/* [6.5.4 语义] 强制转换结果是非左值，对它赋值应报错 */
void test_non_lvalue_assign_cast(void) {
    int x = 5;
    (int)x = 10; /* 违反约束：赋值给非左值，gcc -std=c99 应报错 */
}

/* [6.5.15 语义] 条件表达式结果是非左值，对它赋值应报错 */
void test_non_lvalue_assign_conditional(void) {
    int a = 1, b = 2;
    (1 ? a : b) = 3; /* 违反约束：赋值给非左值，gcc -std=c99 应报错 */
}

/* [6.5.17 语义] 逗号表达式结果是非左值，对它赋值应报错 */
void test_non_lvalue_assign_comma(void) {
    int a = 1;
    (a, 2) = 3; /* 违反约束：赋值给非左值，gcc -std=c99 应报错 */
}

/* [6.5.2.3 语义] const 限定类型在成员访问上传播，对 const 成员赋值应报错 */
void test_const_member_assign(void) {
    const struct S cs = {1, 2.0};
    cs.x = 10; /* 违反约束：cs.x 是 const int 不可修改，gcc -std=c99 应报错 */
}

#endif