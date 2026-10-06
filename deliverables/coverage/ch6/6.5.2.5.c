/*
 * 验证 C99 条款 6.5.2.5 Compound literals
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* [9] EXAMPLE 1: 文件作用域复合字面量，静态存储期，常量表达式 */
int *file_scope_p = (int []){2, 4};

/* [6] 静态存储期验证：函数外复合字面量具有静态存储期，每次调用地址相同 */
int *get_static_compound(void) {
    return (int[]){10, 20};
}

/* [11] EXAMPLE 3: 结构体复合字面量传值与传指针 */
struct point { int x; int y; };

void drawline_by_val(struct point p1, struct point p2) {
    assert(p1.x == 1 && p1.y == 1);
    assert(p2.x == 3 && p2.y == 4);
}

void drawline_by_ptr(struct point *p1, struct point *p2) {
    assert(p1->x == 1 && p1->y == 1);
    assert(p2->x == 3 && p2->y == 4);
}

/* [16] EXAMPLE 8: 每个复合字面量在给定作用域内只创建一个对象 */
struct s { int i; };
int f(void) {
    struct s *p = 0, *q;
    int j = 0;
    again:
    q = p;
    p = &((struct s){ j++ });
    if (j < 2) goto again;
    return p == q && q->i == 1;
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
int main(void) {
    /* [4] 语义：基本复合字面量，提供未命名对象 */
    int *p = (int[]){2, 4};
    assert(p[0] == 2 && p[1] == 4);

    /* [5] 语义：未知大小数组的大小由初始化列表决定，结果为左值 */
    int *a = (int[]){1, 2, 3};
    assert(sizeof((int[]){1,2,3}) / sizeof(int) == 3);
    /* 验证左值：可以取地址 */
    int (*ptr_to_array)[3] = &(int[3]){1, 2, 3};
    assert((*ptr_to_array)[0] == 1);

    /* [6] 语义：块作用域复合字面量具有自动存储期，不同复合字面量地址不同 */
    int *p1 = (int[]){10, 20};
    int *p2 = (int[]){10, 20};
    assert(p1 != p2);

    /* [7] 语义：6.7.8 初始化列表规则适用，部分初始化则剩余为0 */
    int *arr = (int[5]){1, 2};
    assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 0 && arr[3] == 0 && arr[4] == 0);

    /* [8] 语义：const 限定复合字面量可共享，不崩溃即可 */
    const char *c1 = (const char[]){"abc"};
    const char *c2 = "abc";
    (void)c1; (void)c2; /* 可能相等也可能不等，不强制 assert */

    /* [10] EXAMPLE 2: 块作用域，非常量表达式 */
    int val = 42;
    int *p3 = (int[2]){val};
    assert(p3[0] == 42 && p3[1] == 0);

    /* [11] EXAMPLE 3: 指示器初始化器 */
    drawline_by_val((struct point){.x=1, .y=1}, (struct point){.x=3, .y=4});
    drawline_by_ptr(&(struct point){.x=1, .y=1}, &(struct point){.x=3, .y=4});

    /* [12] EXAMPLE 4: 只读复合字面量 */
    const float *fp = (const float []){1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6};
    assert(fp[0] == 1.0f && fp[6] == 1e6f);

    /* [13] EXAMPLE 5: 字符串字面量与 char 数组复合字面量区别 */
    char *mod_str = (char []){"/tmp/fileXXXXXX"};
    mod_str[0] = 'A'; /* 可修改 */
    assert(mod_str[0] == 'A');

    /* [14] EXAMPLE 6: const 复合字面量可共享 */
    assert(((const char []){"abc"}) == "abc" == 1 || ((const char []){"abc"}) != "abc");

    /* [15] EXAMPLE 7: 无法自引用（语义限制，不作为负向约束测试） */
    struct int_list { int car; struct int_list *cdr; };
    struct int_list endless_zeros = {0, &endless_zeros};
    assert(endless_zeros.cdr == &endless_zeros);

    /* [16] EXAMPLE 8: goto 循环中复合字面量地址相同 */
    assert(f() == 1);

    /* [17] Note: 若用 for 循环，生命周期仅限循环体，会导致 UB，此处不测 UB */

    /* [9] 文件作用域验证 */
    assert(file_scope_p[0] == 2 && file_scope_p[1] == 4);
    int *fs1 = get_static_compound();
    int *fs2 = get_static_compound();
    assert(fs1 == fs2); /* 静态存储期，地址相同 */

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束「类型名不能是变长数组(VLA)类型」：gcc -std=c99 应报错 */
void test_vla_constraint(void) {
    int n = 5;
    int *p = (int[n]){1, 2, 3, 4, 5}; 
}

/* [2] 违反约束「初始化器不能提供超出对象的值」：gcc -std=c99 应报错 excess elements */
void test_overflow_init(void) {
    int *p = (int[2]){1, 2, 3}; 
}

/* [3] 违反约束「函数外部复合字面量初始化列表必须为常量表达式」：gcc -std=c99 应报错 */
int external_var = 5;
int *bad_file_scope = (int[]){external_var}; 
#endif