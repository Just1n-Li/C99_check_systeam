/*
 * 验证 C99 6.7.2.1 条款：Structure and union specifiers
 * 预期行为：
 * - 正向测试：能编译并运行通过，断言成功。
 * - 负向测试：违反约束，编译器应报错。
 */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [5] 结构体顺序存储，联合体重叠存储 */
void test_semantics_5(void) {
    struct S { int a; char b; double c; };
    struct S s;
    assert((char*)&s.a < (char*)&s.b);
    assert((char*)&s.b < (char*)&s.c);

    union U { int a; double b; char c[8]; };
    union U u;
    assert((void*)&u.a == (void*)&u.b);
    assert((void*)&u.b == (void*)&u.c);
}

/* [6] struct 和 union 关键字 */
void test_semantics_6(void) {
    struct S1 { int x; };
    union U1 { int x; };
    struct S1 s;
    union U1 u;
    s.x = 1;
    u.x = 1;
    assert(s.x == 1);
    assert(u.x == 1);
}

/* [7] 类型直到 } 才完整 */
void test_semantics_7(void) {
    struct S { int a; } s;
    /* sizeof 可以在 } 之后使用 */
    assert(sizeof(s) == sizeof(int));
}

/* [8] 成员可以是任何对象类型（除了可变修改类型） */
void test_semantics_8(void) {
    struct Inner { int x; };
    struct S {
        int arr[5];
        struct Inner inner;
        int *ptr;
    };
    struct S s;
    s.arr[0] = 1;
    s.inner.x = 2;
    int v = 3;
    s.ptr = &v;
    assert(s.arr[0] == 1);
    assert(s.inner.x == 2);
    assert(*s.ptr == 3);
}

/* [9] _Bool 位域存0或1比较相等 */
void test_semantics_9(void) {
    struct S { _Bool b : 1; };
    struct S s;
    s.b = 0;
    assert(s.b == 0);
    s.b = 1;
    assert(s.b == 1);
}

/* [11] 无名位域和宽度为0的位域 */
void test_semantics_11(void) {
    struct S {
        int a : 4;
        int : 4; /* 无名位域 */
        int b : 4;
        int : 0; /* 宽度为0，强制下一个位域到下一个存储单元 */
        int c : 4;
    };
    struct S s;
    s.a = 1;
    s.b = 2;
    s.c = 3;
    assert(s.a == 1);
    assert(s.b == 2);
    assert(s.c == 3);
}

/* [13] 地址递增，指针指向首成员 */
void test_semantics_13(void) {
    struct S { int a; int b; };
    struct S s;
    assert((char*)&s.a < (char*)&s.b);
    /* 指向结构体的指针转换后指向首成员 */
    assert((void*)&s == (void*)&s.a);
}

/* [14] 联合体大小，指针指向成员 */
void test_semantics_14(void) {
    union U { int a; double b; };
    union U u;
    assert(sizeof(u) >= sizeof(double));
    assert(sizeof(u) >= sizeof(int));
    assert((void*)&u == (void*)&u.a);
    assert((void*)&u == (void*)&u.b);
}

/* [16] 灵活数组成员 */
void test_semantics_16(void) {
    struct s {
        int n;
        double d[]; /* 灵活数组成员 */
    };
    struct s_no_fam {
        int n;
    };
    /* sizeof 忽略 FAM，但可能有尾部填充 */
    assert(sizeof(struct s) >= sizeof(struct s_no_fam));

    int m = 5;
    struct s *p = malloc(sizeof(struct s) + m * sizeof(double));
    assert(p != NULL);
    p->n = m;
    for (int i = 0; i < m; i++) {
        p->d[i] = i * 2.0;
    }
    assert(p->d[4] == 8.0);
    free(p);
}

/* [17] 示例 */
void test_example_17(void) {
    struct s { int n; double d[]; };
    int m = 3;
    struct s *p = malloc(sizeof (struct s) + sizeof (double) * m);
    p->n = m;
    for (int i = 0; i < m; i++) p->d[i] = i;
    assert(p->n == 3);
    assert(p->d[2] == 2.0);
    free(p);
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束：结构体包含不完整类型成员（非最后成员） */
struct IncompleteType;
struct BadIncomplete {
    struct IncompleteType val;  /* 应报错 */
};

/* [2] 违反约束：结构体包含函数类型成员 */
struct BadFunc {
    int func(void); /* 应报错 */
};

/* [2] 违反约束：结构体包含自身实例 */
struct SelfInstance {
    struct SelfInstance self; /* 应报错 */
};

/* [2] 违反约束：包含灵活数组的结构体作为其他结构体成员 */
struct WithFAM {
    int n;
    int data[];
};
struct BadFAMMember {
    int x;
    struct WithFAM fam; /* 应报错 */
};

/* [2] 违反约束：包含灵活数组的结构体作为数组元素 */
struct BadFAMArray {
    struct WithFAM arr[3]; /* 应报错 */
};

/* [3] 违反约束：位域宽度为负 */
struct NegBitfield {
    int x : -1; /* 应报错 */
};

/* [3] 违反约束：位域宽度超过类型宽度 */
struct HugeBitfield {
    int x : 100; /* 应报错 */
};

/* [3] 违反约束：位域宽度为0但有声明符 */
struct ZeroBitfieldDeclarator {
    int x : 0; /* 应报错 */
};

/* [4] 违反约束：位域类型为非允许类型（如 float） */
struct FloatBitfield {
    float x : 1; /* 应报错 */
};

#endif

int main(void) {
    test_semantics_5();
    test_semantics_6();
    test_semantics_7();
    test_semantics_8();
    test_semantics_9();
    test_semantics_11();
    test_semantics_13();
    test_semantics_14();
    test_semantics_16();
    test_example_17();
    printf("All positive tests passed.\n");
    return 0;
}