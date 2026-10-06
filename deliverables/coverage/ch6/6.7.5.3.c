/* ============================================================================
 * C99 6.7.5.3  Function declarators (including prototypes)
 *
 * 正向测试：以下代码应能编译并运行通过（验证 Semantics [5]-[19]）。
 * 负向测试：以下代码违反 Constraints [1]-[4]，gcc -std=c99 -pedantic-errors
 *           应报错；统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 * ============================================================================
 */

#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ---------------------------------------------------------------------------
 * 正向测试
 * ------------------------------------------------------------------------- */

/* [5] 函数返回类型：D(parameter-type-list) 使 ident 类型为 "function returning T" */
static int add2(int a, int b) { return a + b; }

/* [6] 参数类型列表指定参数类型，并可声明参数标识符 */
static int mul3(int x, int y, int z) { return x * y * z; }

/* [7] "array of type" 参数调整为 "qualified pointer to type"；
 *     数组派生中的限定符保留到指针上。
 *     这里参数写成 int a[10]，实际类型为 int *。 */
static size_t arr_len(int a[10]) { (void)a; return 10; }

/* [7] 带 static 的数组参数：调用者须提供至少 N 个元素 */
static int sum_static(int a[static 3]) { return a[0] + a[1] + a[2]; }

/* [7] 带 const 限定的数组参数：调整为 const int * */
static int first_const(const int a[5]) { return a[0]; }

/* [8] "function returning type" 参数调整为 "pointer to function returning type" */
static int apply(int f(int), int v) { return f(v); }   /* f 实际为 int (*)(int) */
static int inc(int x) { return x + 1; }

/* [9] 省略号终止：逗号之后不再提供参数数目/类型信息 */
static int sum_varargs(int n, ...) {
    /* 这里不实际读取可变参数，仅验证声明合法 */
    return n;
}

/* [10] 唯一无名参数 void 表示函数无参数 */
static int no_params(void) { return 42; }

/* [11] 参数声明中标识符若可解释为 typedef 名或参数名，取 typedef 名 */
typedef int T;
static int use_typedef_name(int T) {   /* 此处 T 是 typedef 名 int，不是参数名 */
    T v = 7;                            /* 合法：T 是类型 */
    return v;
}

/* [12] 非定义的函数声明中，参数可为不完整类型，可用 [*] 指定 VLA 类型 */
static int vla_proto(int n, int a[*]);  /* 声明，非定义 */
static int vla_proto(int n, int a[n]) { /* 定义 */
    int s = 0, i;
    for (i = 0; i < n; i++) s += a[i];
    return s;
}

/* [13] 参数声明中的存储类说明符（非定义时）被忽略 */
static int ignore_reg(int x) { return x; }

/* [14] 空标识符列表：非定义时表示不提供参数信息 */
static int empty_list();               /* 声明：无参数信息 */
static int empty_list(int x) { return x; }  /* 定义 */

/* [15] 兼容性：参数类型列表与空标识符列表的兼容（默认实参提升） */
static int compat_proto(int);          /* 参数类型列表 */
static int compat_empty();             /* 空标识符列表，非定义 */
static int compat_proto(int x) { return x; }

/* [16] EXAMPLE 1: int f(void), *fip(), (*pfi)(); */
static int f16(void) { return 1; }
static int *fip16(void) { static int v = 2; return &v; }
static int (*pfi16)();                 /* 指向无参数规格函数的指针 */

/* [18] EXAMPLE 2: int (*apfi[3])(int *x, int *y); */
static int pair_add(int *x, int *y) { return *x + *y; }
static int (*apfi[3])(int *x, int *y) = { pair_add, pair_add, pair_add };

/* [19] EXAMPLE 3: int (*fpfi(int (*)(long), int))(int, ...); */
static int long_fn(long v) { return (int)v; }
static int var_fn(int a, ...) { return a; }
static int (*fpfi(int (*g)(long), int n))(int, ...) {
    (void)g; (void)n;
    return var_fn;
}

/* [17] 文件作用域：f16/fip16 具有文件作用域与外部链接（此处 static 仅内部链接，
 *      但作用域规则一致）；pfi16 为文件作用域对象。 */

int main(void) {
    /* [5][6] */
    assert(add2(3, 4) == 7);
    assert(mul3(2, 3, 4) == 24);

    /* [7] 数组参数调整 */
    int a10[10] = {0};
    assert(arr_len(a10) == 10);
    int a3[3] = {1, 2, 3};
    assert(sum_static(a3) == 6);
    const int ca[5] = {9, 0, 0, 0, 0};
    assert(first_const(ca) == 9);

    /* [8] 函数参数调整为函数指针 */
    assert(apply(inc, 41) == 42);

    /* [9] 省略号 */
    assert(sum_varargs(3, 1, 2, 3) == 3);

    /* [10] void 参数 */
    assert(no_params() == 42);

    /* [11] typedef 名优先 */
    assert(use_typedef_name(0) == 7);

    /* [12] VLA 参数 */
    int v[4] = {1, 2, 3, 4};
    assert(vla_proto(4, v) == 10);

    /* [13] register 被忽略 */
    assert(ignore_reg(5) == 5);

    /* [14] 空标识符列表 */
    assert(empty_list(8) == 8);

    /* [15] 兼容性：通过原型调用 */
    assert(compat_proto(11) == 11);
    assert(compat_empty(12) == 12);   /* 空列表声明，调用时默认实参提升 */

    /* [16] EXAMPLE 1 */
    assert(f16() == 1);
    assert(*fip16() == 2);
    pfi16 = f16;                      /* 指向无参数规格函数 */
    assert(pfi16() == 1);

    /* [18] EXAMPLE 2 */
    int x = 3, y = 4;
    assert(apfi[0](&x, &y) == 7);
    assert(apfi[1](&x, &y) == 7);
    assert(apfi[2](&x, &y) == 7);

    /* [19] EXAMPLE 3 */
    int (*ret)(int, ...) = fpfi(long_fn, 5);
    assert(ret(99, 1, 2, 3) == 99);

    printf("C99 6.7.5.3 positive tests passed.\n");
    return 0;
}

/* ============================================================================
 * 负向测试：以下代码违反 C99 6.7.5.3 的 Constraints，应编译报错。
 * 统一放在 #if 0 中，保证本文件仍能正常编译运行。
 * ============================================================================
 */
#if 0

/* [1] 违反约束「函数声明符不得指定函数类型或数组类型作为返回类型」：
 *     返回类型为函数类型，gcc -std=c99 应报错。 */
int f_ret_func(void)(void);            /* error: function returning function */

/* [1] 返回类型为数组类型，应报错。 */
int f_ret_array(void)[10];             /* error: function returning array */

/* [2] 违反约束「参数声明中唯一允许的存储类说明符是 register」：
 *     参数使用 static 存储类，应报错。 */
int f_param_static(static int x);      /* error: storage class static in parameter */

/* [2] 参数使用 extern 存储类，应报错。 */
int f_param_extern(extern int x);      /* error: storage class extern in parameter */

/* [3] 违反约束「非函数定义的函数声明符中的标识符列表必须为空」：
 *     非定义的声明中出现非空标识符列表，应报错。 */
int f_nonempty_idlist(a, b);           /* error: identifier list not empty in non-definition */

/* [4] 违反约束「作为函数定义一部分的参数类型列表中的参数，调整后不得为不完整类型」：
 *     定义中参数为不完整结构体类型，应报错。 */
struct Incomplete;
int f_incomplete_param(struct Incomplete s) { return 0; }  /* error: incomplete parameter type */

/* [4] 定义中参数为不完整数组类型（元素类型不完整），应报错。 */
int f_incomplete_array(struct Incomplete a[5]) { return 0; } /* error: incomplete element type */

#endif