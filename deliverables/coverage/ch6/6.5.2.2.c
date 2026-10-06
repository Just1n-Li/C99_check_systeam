/*
 * 验证 C99 6.5.2.2 Function calls
 * 正向测试：应能编译并运行通过（所有 assert 通过）
 * 负向测试：违反约束，应编译报错
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] 后缀表达式 + 括号列表构成函数调用；空列表表示无参数 */
static int no_args(void) { return 42; }
static int one_arg(int x) { return x * 2; }

/* [4] 参数可为任意对象类型；参数被赋值为对应实参的值 */
static long long sum_ll(long long a, long long b) { return a + b; }
static double add_d(double a, double b) { return a + b; }

/* [5] 函数返回对象类型 -> 调用表达式同类型；返回 void -> 调用为 void 类型 */
static int ret_int(void) { return 7; }
static void ret_void(int *p) { *p = 99; }

/* [6] 无原型：默认实参提升 char->int, float->double */
/* 用旧式声明模拟无原型函数（C99 仍允许） */
static int take_int_old(i) int i; { return i + 1; }
static double take_dbl_old(d) double d; { return d * 2.0; }

/* [7] 有原型：实参按赋值隐式转换到参数类型；省略号后停止转换 */
static int take_char(char c) { return (int)c; }
static int take_double(double d) { return (int)d; }
static int vararg_first(int first, ...) { return first; }

/* [7] const/volatile 限定：参数取非限定版本 */
static int take_const_int(const int x) { return x; }
static int take_volatile_int(volatile int x) { return x; }
static int take_const_ptr(const int *p) { return *p; }

/* [10] 序列点：实参求值完成后再调用函数 */
static int side_effect_counter = 0;
static int inc_counter(void) { side_effect_counter++; return side_effect_counter; }
static int use_two(int a, int b) { (void)a; (void)b; return 0; }

/* [11] 递归调用：直接与间接 */
static int factorial(int n) { return n <= 1 ? 1 : n * factorial(n - 1); }
static int even(int n);
static int odd(int n);
static int even(int n) { return n == 0 ? 1 : odd(n - 1); }
static int odd(int n) { return n == 0 ? 0 : even(n - 1); }

/* [12] EXAMPLE: (*pf[f1()])(f2(), f3()+f4()) 各函数可任意顺序调用 */
static int call_order[4];
static int idx = 0;
static int f1(void) { call_order[idx++] = 1; return 0; }
static int f2(void) { call_order[idx++] = 2; return 10; }
static int f3(void) { call_order[idx++] = 3; return 20; }
static int f4(void) { call_order[idx++] = 4; return 30; }
static int target_fn(int a, int b) { return a + b; }
typedef int (*pf_t)(int, int);

/* [5] 函数返回结构体：调用结果为非左值（用于后续负向测试） */
typedef struct { int x; } S_t;
static S_t get_struct(void) { S_t s; s.x = 5; return s; }

/* [1] 通过函数指针调用 */
static int (*get_pf(void))(int) { return one_arg; }

int main(void)
{
    /* [3] 基本函数调用 */
    assert(no_args() == 42);
    assert(one_arg(21) == 42);

    /* [4] 参数为任意对象类型，参数赋值为实参值 */
    assert(sum_ll(1000000000LL, 2000000000LL) == 3000000000LL);
    assert(add_d(1.5, 2.5) == 4.0);

    /* [5] 返回对象类型 -> 调用表达式同类型；返回 void */
    int r = ret_int();
    assert(r == 7);
    int v = 0;
    ret_void(&v);
    assert(v == 99);

    /* [6] 无原型默认实参提升：char 提升为 int, float 提升为 double */
    char c = 5;
    assert(take_int_old(c) == 6);       /* char -> int 提升 */
    float f = 3.0f;
    assert(take_dbl_old(f) == 6.0);     /* float -> double 提升 */

    /* [7] 有原型：实参隐式转换到参数类型 */
    assert(take_char(321) == 65);       /* int 321 -> char 65 ('A') */
    assert(take_double(3) == 3.0);      /* int 3 -> double 3.0 */

    /* [7] 省略号后停止转换：默认提升作用于尾随参数 */
    assert(vararg_first(100, (char)1, (float)2.0f) == 100);

    /* [7] const/volatile 限定：参数取非限定版本 */
    assert(take_const_int(5) == 5);
    assert(take_volatile_int(6) == 6);
    int val = 7;
    assert(take_const_ptr(&val) == 7);

    /* [10] 序列点：实参求值在调用前完成 */
    side_effect_counter = 0;
    use_two(inc_counter(), inc_counter());
    assert(side_effect_counter == 2);   /* 两次 inc 都在调用前完成 */

    /* [11] 直接递归 */
    assert(factorial(5) == 120);
    /* [11] 间接递归 */
    assert(even(10) == 1);
    assert(odd(10) == 0);

    /* [12] EXAMPLE: (*pf[f1()])(f2(), f3()+f4()) */
    idx = 0;
    pf_t pf_arr[1];
    pf_arr[0] = target_fn;
    int result = (*pf_arr[f1()])(f2(), f3() + f4());
    assert(result == 60);              /* 10 + (20+30) */
    /* 所有副作用在调用前完成 */
    assert(idx == 4);

    /* [1] 通过函数指针调用（指针指向返回对象类型的函数） */
    int (*pf)(int) = get_pf();
    assert(pf(21) == 42);

    /* [5] 函数返回结构体：访问成员（合法，读取非左值成员） */
    assert(get_struct().x == 5);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [1] 违反约束：调用表达式类型为「返回数组的函数」——不允许 */
/* C99 不允许函数返回数组类型，但可通过 typedef 制造此类声明 */
typedef int Arr5[5];
/* 以下声明一个「返回数组的函数」是约束违反本身，这里测试调用端 */
extern Arr5 bad_func_returning_array(void);  /* 声明本身在多数编译器报错 */
/* 若编译器接受声明，则调用 bad_func_returning_array() 违反 [1] */

/* [1] 违反约束：对返回数组类型的函数指针解引用调用 */
/* （同上，作为约束测试占位） */

/* [2] 违反约束：有原型时参数个数不匹配——参数过多 */
extern int two_params(int a, int b);
two_params(1, 2, 3);  /* 3 个参数 vs 2 个参数，应报错 */

/* [2] 违反约束：有原型时参数个数不匹配——参数过少 */
two_params(1);       /* 1 个参数 vs 2 个参数，应报错 */

/* [2] 违反约束：实参类型不能赋值给参数类型——指针类型不兼容 */
extern int take_int_ptr(int *p);
take_int_ptr((double*)0);  /* double* 不能赋值给 int*，应报错 */

/* [2] 违反约束：实参类型不能赋值给参数类型——结构体不匹配 */
struct A { int x; };
struct B { int y; };
extern int take_struct_a(struct A a);
take_struct_a((struct B){1});  /* struct B 不能赋值给 struct A，应报错 */

/* [5] 违反约束（语义约束）：对函数调用结果（非左值）赋值应报错 */
extern struct S { int x; } get_s(void);
get_s().x = 10;  /* 函数返回结构体，成员为非左值，赋值应报错 */

/* [5] 违反约束（语义约束）：对 void 函数调用结果赋值 */
extern void void_func(void);
int v = void_func();  /* void 不能初始化 int，应报错 */

/* [1] 违反约束：调用表达式不是函数指针——对整数调用 */
int not_a_function = 0;
not_a_function();  /* int 不是函数指针，应报错 */

/* [1] 违反约束：调用表达式不是函数指针——对结构体调用 */
struct Dummy { int a; } d;
d();  /* 结构体不是函数指针，应报错 */

#endif