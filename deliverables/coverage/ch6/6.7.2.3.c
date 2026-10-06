/*
 * 验证 C99 条款 6.7.2.3 (Tags)
 * 预期行为：
 * - 正向测试：能正常编译并运行通过，断言全部成立。
 * - 负向测试：违反约束条件，应导致编译报错。
 */

#include <assert.h>
#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [4] 相同作用域和相同标签声明同一类型；类型在定义的右大括号前是不完整的，之后是完整的 */
struct S { int a; };
struct S; /* 不重新声明标签，指定同一类型，此时类型已完整 */
void test_scope_and_completeness(void) {
    struct S s1 = {10};
    assert(s1.a == 10);
    assert(sizeof(struct S) == sizeof(int)); /* 类型已完整，可以求大小 */
}

/* [5] 不同作用域或不同标签声明不同的类型；无标签声明不同的类型 */
struct { int a; } anon1;
struct { int a; } anon2; /* 匿名结构体，与 anon1 类型不同 */
void test_distinct_types(void) {
    assert(sizeof(anon1) == sizeof(anon2)); /* 布局可能相同，但类型不同 */
    /* anon1 = anon2; */ /* 若取消注释，由于类型不同将编译报错 */
}

void inner_scope_func(void) {
    struct S { int b; }; /* 不同作用域，声明了不同的类型 struct S */
    struct S inner = {20};
    assert(inner.b == 20);
}

/* [6] struct-or-union identifier { struct-declaration-list } 声明结构/联合类型并定义内容 */
struct U { int x; } u_var;
/* [7] struct-or-union identifier ; 指定结构或联合类型并声明标签 */
struct V;
/* [8] struct-or-union identifier (无可见标签时) 声明不完整类型并声明标签 */
struct W *wp; /* 声明不完整类型 struct W 和标签 W */
/* [9] struct-or-union identifier (有可见标签时) 指定同一类型，不重新声明标签 */
struct X { int a; };
struct X x1;
struct X *xp; /* 指定同一类型 */

void test_tags_and_incomplete(void) {
    u_var.x = 5;
    assert(u_var.x == 5);
    
    struct V *vp = 0; /* struct V 是不完整类型，只能使用指针 */
    assert(vp == 0);
    
    wp = 0; /* struct W 是不完整类型 */
    assert(wp == 0);
    
    x1.a = 42;
    xp = &x1; /* 同类型赋值 */
    assert(xp->a == 42);
}

/* [10] EXAMPLE 1 自引用结构 */
struct tnode {
    int count;
    struct tnode *left, *right; /* 在右大括号前类型不完整，但声明指针不需要完整类型 */
};
void test_example1(void) {
    struct tnode s = {1, 0, 0}, *sp = &s;
    s.right = &s; /* s.right 指向 s 自身 */
    assert(sp->left == 0);
    assert(s.right->count == 1); /* s.right->count 即 s.count */
}

/* [11] EXAMPLE 1 替代方案：使用 typedef 机制 */
typedef struct tnode2 TNODE;
struct tnode2 {
    int count;
    TNODE *left, *right;
};
void test_example1_alt(void) {
    TNODE s = {2, 0, 0}, *sp = &s;
    assert(sp->count == 2);
}

/* [12] EXAMPLE 2 相互引用的结构 */
struct s2; /* 提前声明标签 s2，消除上下文敏感性 */
struct s1 {
    struct s2 *s2p; /* D1: 指向 struct s2 的指针 */
};
struct s2 {
    struct s1 *s1p; /* D2: 指向 struct s1 的指针 */
};
void test_example2(void) {
    struct s1 d1 = {0};
    struct s2 d2 = {0};
    d1.s2p = &d2;
    d2.s1p = &d1;
    assert(d1.s2p->s1p == &d1);
    assert(d2.s1p->s2p == &d2);
}

int main(void) {
    test_scope_and_completeness();
    test_distinct_types();
    inner_scope_func();
    test_tags_and_incomplete();
    test_example1();
    test_example1_alt();
    test_example2();
    
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束 [1]「特定类型的内容只能定义一次」：同一作用域内重复定义 struct A 的内容 */
struct A { int x; };
struct A { int y; };

/* 违反约束 [2]「在使用相同标签声明相同类型的两个声明中，它们必须都使用相同的 struct、union 或 enum 选择」：struct B 和 union B 使用相同标签 */
struct B { int x; };
union B { int y; };

/* 违反约束 [3]「形式为 enum identifier 且不带枚举器列表的类型说明符只能出现在其指定的类型完整之后出现」：在 enum C 完整前使用 enum C 声明变量 */
enum C c_var;
enum C { RED, GREEN, BLUE };

#endif