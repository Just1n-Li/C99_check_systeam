/*
 * 验证 C99 6.5.2.3 Structure and union members
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [6] EXAMPLE 1: f().x is a valid postfix expression but is not an lvalue */
struct S1 { int x; double y; };
struct S1 get_s1(void) {
    struct S1 s = { 10, 2.5 };
    return s;
}

/* [7] EXAMPLE 2: qualified types propagation */
struct s { int i; const int ci; };

/* [5][8] EXAMPLE 3: Common initial sequence */
struct t1 { int m; };
struct t2 { int m; };

int f(struct t1 *p1, struct t2 *p2) {
    if (p1->m < 0) p2->m = -p2->m;
    return p1->m;
}

int g(void) {
    union {
        struct t1 s1;
        struct t2 s2;
    } u;
    u.s1.m = 5;
    /* [5] The complete type of the union is visible here, so inspecting common initial part is allowed */
    return f(&u.s1, &u.s2);
}

int main(void) {
    /* [3] Semantics of . operator: result is an lvalue if the first expression is an lvalue */
    struct S1 a = {1, 1.5};
    a.x = 5; 
    assert(a.x == 5);

    /* [4] Semantics of -> operator: result is an lvalue */
    struct S1 *pa = &a;
    pa->y = 3.14;
    assert(pa->y == 3.14);

    /* [6] EXAMPLE 1: f().x is valid but not an lvalue (can read, cannot assign) */
    int val = get_s1().x;
    assert(val == 10);

    /* [7] EXAMPLE 2: qualified types */
    struct s s_obj = {0, 0};
    const struct s cs_obj = {0, 0};
    volatile struct s vs_obj = {0, 0};

    s_obj.i = 10;   /* int */
    /* s_obj.ci is const int, cannot assign */
    assert(s_obj.i == 10);

    /* cs_obj.i is const int, cannot assign */
    /* cs_obj.ci is const int, cannot assign */

    /* vs_obj.i is volatile int, assignment is allowed */
    vs_obj.i = 20;
    assert(vs_obj.i == 20);
    /* vs_obj.ci is volatile const int, cannot assign */

    /* [5][8] EXAMPLE 3: Common initial sequence */
    union {
        struct { int alltypes; } n;
        struct { int type; int intnode; } ni;
        struct { int type; double doublenode; } nf;
    } u;

    u.nf.type = 1;
    u.nf.doublenode = 3.14;
    /* [5] permitted to inspect the common initial part */
    assert(u.n.alltypes == 1);
    if (u.n.alltypes == 1) {
        assert(sin(u.nf.doublenode) > 0.0);
    }

    /* [8] valid fragment context test */
    assert(g() == 5);

    printf("All positive tests passed!\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束「. 的第一个操作数必须是结构体或联合体类型」：int 不能有成员 */
int int_var = 0;
int_var.x;

/* [1] 违反约束「. 的第二个操作数必须命名该类型的成员」：z 不是 S1 的成员 */
struct S1 valid_struct;
valid_struct.z;

/* [2] 违反约束「-> 的第一个操作数必须是指向结构体或联合体的指针」：int 不是指针 */
int int_var2 = 0;
int_var2->x;

/* [2] 违反约束「-> 的第一个操作数必须是指向结构体或联合体的指针」：结构体变量不是指针 */
struct S1 valid_struct2;
valid_struct2->x;

/* [2] 违反约束「-> 的第二个操作数必须命名所指向类型的成员」：z 不是 S1 的成员 */
struct S1 *ptr = &valid_struct2;
ptr->z;

/* [6] 违反约束「非左值不能赋值」：f().x 不是左值，不能对其赋值 */
get_s1().x = 10;

/* [7] 违反约束「对 const 限定的成员赋值」：cs_obj.i 具有 const int 类型，不可修改 */
cs_obj.i = 10;

/* [7] 违反约束「对 const 限定的成员赋值」：s_obj.ci 具有 const int 类型，不可修改 */
s_obj.ci = 10;

/* [7] 违反约束「对 volatile const 限定的成员赋值」：vs_obj.ci 具有 volatile const int 类型，不可修改 */
vs_obj.ci = 10;

#endif