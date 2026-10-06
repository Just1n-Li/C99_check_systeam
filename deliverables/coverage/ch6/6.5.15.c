/*
 * 验证 C99 6.5.15 条件运算符
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：logical-OR-expression ? expression : conditional-expression */
/* [2] 约束：第一个操作数必须为标量类型 */
/* [4] 语义：第一个操作数求值，之后有序列点；短路求值；结果转换为下述类型 */
static int g_side_effect = 0;
static void f2(void) { g_side_effect = 2; }
static void f3(void) { g_side_effect = 3; }

static struct S { int x; } gs1 = {1}, gs2 = {2};
static struct T { int x; } gt1 = {1};

void test_conditional(void) {
    int a = 1, b = 0;

    /* [1][2] 第一个操作数为标量类型（int） */
    int val = (a != 0) ? 10 : 20;
    assert(val == 10);

    /* [2] 第一个操作数为指针（标量类型） */
    int *p = &a;
    val = (p != NULL) ? 100 : 200;
    assert(val == 100);

    /* [4] 短路求值与序列点测试：条件为真时，第三操作数不求值 */
    g_side_effect = 0;
    (1) ? f2() : f3();
    assert(g_side_effect == 2);

    /* [4] 短路求值：条件为假时，第二操作数不求值 */
    g_side_effect = 0;
    (0) ? f2() : f3();
    assert(g_side_effect == 3);

    /* [3][5] 第二和第三操作数均为算术类型，结果类型由通常算术转换决定 */
    double d = (1) ? 1 : 2.5; /* int 与 double，结果应为 double */
    assert(d == 1.0);
    assert(sizeof(1 ? 1 : 2.5) == sizeof(double));

    /* [3][5] 第二和第三操作数均为相同的结构体或联合类型 */
    struct S s = (1) ? gs1 : gs2;
    assert(s.x == 1);

    /* [3][5] 第二和第三操作数均为 void 类型 */
    (1) ? (void)1 : (void)2;

    /* [3][6] 第二和第三操作数均为指向兼容类型的限定/非限定版本的指针 */
    const int *c_ip = NULL;
    volatile int *v_ip = NULL;
    int *ip = NULL;
    const void *c_vp = NULL;
    void *vp = NULL;
    const char *c_cp = NULL;

    /* [6][8] c_vp 和 c_ip -> const void * */
    const void *r1 = (1) ? c_vp : c_ip;
    /* [6][8] v_ip 和 0 -> volatile int * */
    volatile int *r2 = (1) ? v_ip : 0;
    /* [6][8] c_ip 和 v_ip -> const volatile int * */
    const volatile int *r3 = (1) ? c_ip : v_ip;
    /* [6][8] vp 和 c_cp -> const void * */
    const void *r4 = (1) ? vp : c_cp;
    /* [6][8] ip 和 c_ip -> const int * */
    const int *r5 = (1) ? ip : c_ip;
    /* [6][8] vp 和 ip -> void * */
    void *r6 = (1) ? vp : ip;

    /* 避免未使用变量警告 */
    (void)r1; (void)r2; (void)r3; (void)r4; (void)r5; (void)r6;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「第一个操作数必须为标量类型」：结构体不能作为条件判断，gcc -std=c99 应报错 */
void test_neg_first_operand(void) {
    struct S { int x; } s = {0};
    s ? 1 : 2;
}

/* [3] 违反约束「第二和第三操作数必须匹配」：一个是结构体，一个是算术类型，gcc -std=c99 应报错 */
void test_neg_mismatched_types(void) {
    struct S { int x; } s = {0};
    1 ? s : 1;
}

/* [3] 违反约束「第二和第三操作数必须匹配」：不同的结构体类型，gcc -std=c99 应报错 */
void test_neg_different_structs(void) {
    struct S { int x; } s = {0};
    struct T { int x; } t = {0};
    1 ? s : t;
}

/* [3] 违反约束「指针必须指向兼容类型或void」：int* 与 double* 不兼容，gcc -std=c99 应报错 */
void test_neg_incompatible_pointers(void) {
    int *ip = 0;
    double *dp = 0;
    1 ? ip : dp;
}

/* [95 脚注] 违反约束「条件表达式不产生左值」：对条件表达式结果赋值，gcc -std=c99 应报错 */
void test_neg_not_lvalue(void) {
    int a = 1, b = 2;
    (1 ? a : b) = 3;
}

#endif

int main(void) {
    test_conditional();
    printf("6.5.15 Conditional operator tests passed.\n");
    return 0;
}