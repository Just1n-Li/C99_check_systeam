/*
 * 测试目标：C99 6.6 Constant expressions（常量表达式）
 *
 * 预期行为：
 *   - 正向测试：以下使用常量表达式的代码应能编译并运行通过（assert 全部成立）。
 *   - 负向测试：违反 6.6 约束（[3] 禁止赋值/自增/自减/函数调用/逗号运算符；
 *     [4] 结果须在类型可表示范围内；[6] 整数常量表达式的操作数限制；
 *     [8] 算术常量表达式的操作数限制）的代码应被编译器拒绝（编译报错）。
 *     负向片段统一放在 #if 0 ... #endif 中，保证本文件仍可正常编译运行。
 *
 * 覆盖段落：[1][2][3][4][5][6][7][8][9][10][11] 及 Footnote 100。
 */

#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] constant-expression: conditional-expression
 * [2] 常量表达式可在翻译期求值，可用于任何需要常量的地方。
 * [6] 整数常量表达式：仅含整数常量、枚举常量、字符常量、sizeof 结果、
 *     以及作为强制转换直接操作数的浮点常量；转换只能算术→整数。
 * [99] 用于数组大小、case 标签、位域宽度、枚举值。 */

enum Color { RED = 1, GREEN = 2, BLUE = RED + GREEN };   /* [6] 枚举常量 */

/* [6] 数组大小必须是整数常量表达式 */
static int arr_size_test[3 + 4 * 2];                     /* 值 11 */
static int arr_sizeof_test[sizeof(int) + 1];             /* sizeof 结果 */

/* [6] 浮点常量作为强制转换的直接操作数 */
static int cast_float_operand = (int)3.9;                /* 合法：3 */

/* [6] 转换只能算术→整数（此处 double→int） */
static int arith_to_int = (int)(2.0 + 3.0);

/* [7] 初始化器中的常量表达式：算术常量表达式 */
static int init_arith = 10 * 2 + 5;

/* [7] 空指针常量 */
static int *init_null_ptr = 0;

/* [7] 地址常量：静态存储期对象的地址 */
static int global_obj = 42;
static int *init_addr_const = &global_obj;

/* [7] 地址常量 ± 整数常量表达式 */
static int global_arr[10];
static int *init_addr_plus = &global_arr[0] + 5;         /* 地址常量 + 整数常量表达式 */
static int *init_addr_minus = &global_arr[9] - 3;

/* [9] 地址常量可由数组/函数类型隐式产生 */
static int *init_array_decay = global_arr;               /* 数组类型隐式转指针 */
static int (*init_func_ptr)(void) = NULL;                /* 函数指针（此处用空指针常量） */

/* [9] 地址常量创建中可使用 [] . -> & * 及指针转换，但不得访问对象值 */
struct Point { int x; int y; };
static struct Point gp = { 1, 2 };
static int *init_member_addr = &gp.y;                    /* 使用 & 和 . 创建地址常量 */

/* [5] 翻译期浮点求值的精度/范围至少与执行期一样大 */
static double fp_const = 1.0 / 3.0;

/* [11] 常量表达式求值语义与非恒定表达式相同 */
static int same_semantics = (2 + 3) * 4;                 /* 20 */

/* [100] 短路求值：2 || 1/0 是合法整数常量表达式，值为 1 */
static int short_circuit_const = 2 || 1 / 0;

/* [3] 赋值/自增/自减/函数调用/逗号运算符在“未被求值的子表达式”中允许 */
static int unevaluated_ok = sizeof(int) ? 1 : (0, 0);    /* 逗号在未求值分支 */

/* [6] 整数常量表达式用于 case 标签 */
static int switch_test(int v)
{
    switch (v) {
    case 1 + 2:            /* 整数常量表达式 */
        return 100;
    case (int)3.0:         /* 浮点常量作为强制转换直接操作数 */
        return 200;
    default:
        return 0;
    }
}

/* [6] 整数常量表达式用于位域宽度 */
struct BitField {
    unsigned int a : 3;              /* 整数常量表达式 */
    unsigned int b : (1 + 2);        /* 整数常量表达式 */
};

/* [6] sizeof 的结果是整数常量 */
static int sizeof_is_const = sizeof(char);

int main(void)
{
    /* [2] 常量表达式可用于任何需要常量的地方 */
    assert(sizeof(arr_size_test) / sizeof(arr_size_test[0]) == 11);
    assert(sizeof(arr_sizeof_test) / sizeof(arr_sizeof_test[0]) == sizeof(int) + 1);

    /* [6] 浮点常量作为强制转换直接操作数 */
    assert(cast_float_operand == 3);
    assert(arith_to_int == 5);

    /* [7] 初始化器中的算术常量表达式 */
    assert(init_arith == 25);

    /* [7] 空指针常量 */
    assert(init_null_ptr == NULL);

    /* [7][9] 地址常量 */
    assert(*init_addr_const == 42);
    assert(init_addr_plus == &global_arr[5]);
    assert(init_addr_minus == &global_arr[6]);
    assert(init_array_decay == &global_arr[0]);
    assert(init_member_addr == &gp.y);
    assert(*init_member_addr == 2);

    /* [5] 翻译期浮点常量 */
    assert(fp_const > 0.333 && fp_const < 0.334);

    /* [11] 求值语义一致 */
    assert(same_semantics == 20);

    /* [100] 短路：2 || 1/0 == 1 */
    assert(short_circuit_const == 1);

    /* [3] 未求值子表达式中的逗号运算符 */
    assert(unevaluated_ok == 1);

    /* [6] case 标签中的整数常量表达式 */
    assert(switch_test(3) == 100);
    assert(switch_test(3) == 200 || switch_test(3) == 100); /* 3 匹配 case 1+2 */
    assert(switch_test(99) == 0);

    /* [6] 位域宽度 */
    {
        struct BitField bf;
        bf.a = 7;   /* 3 位最大值 */
        bf.b = 7;
        assert(bf.a == 7);
        assert(bf.b == 7);
    }

    /* [6] sizeof 结果是整数常量 */
    assert(sizeof_is_const == 1);

    /* [6] 枚举常量 */
    assert(RED == 1 && GREEN == 2 && BLUE == 3);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [3]：常量表达式不得包含赋值运算符。
 * 期望：gcc -std=c99 报错（"initializer element is not constant" 或类似）。 */
static int bad_assign = (1 = 2);

/* 违反约束 [3]：常量表达式不得包含自增运算符。
 * 期望：编译报错。 */
static int bad_incr = (1++);

/* 违反约束 [3]：常量表达式不得包含自减运算符。
 * 期望：编译报错。 */
static int bad_decr = (1--);

/* 违反约束 [3]：常量表达式不得包含函数调用运算符。
 * 期望：编译报错（"initializer element is not constant"）。 */
int f(void);
static int bad_call = f();

/* 违反约束 [3]：常量表达式不得包含逗号运算符（在求值位置）。
 * 期望：编译报错。 */
static int bad_comma = (1, 2);

/* 违反约束 [3]：数组大小必须是常量表达式，不能含函数调用。
 * 期望：编译报错（"variably modified" 或 "not constant"）。 */
int g(void);
static int bad_array_size[g()];

/* 违反约束 [3]：case 标签必须是整数常量表达式，不能含函数调用。
 * 期望：编译报错（"case label does not reduce to an integer constant"）。 */
int h(void);
void bad_case(int v) {
    switch (v) {
    case h():      /* 函数调用 */
        break;
    }
}

/* 违反约束 [3]：位域宽度必须是整数常量表达式，不能含赋值。
 * 期望：编译报错。 */
struct BadBitField {
    unsigned int x : (1 = 2);
};

/* 违反约束 [4]：常量表达式结果须在类型可表示范围内。
 * 期望：编译报错（"integer overflow in expression" 或类似）。 */
static int bad_range = 2147483647 + 1;   /* 假设 int 为 32 位 */

/* 违反约束 [6]：整数常量表达式只能含整数常量、枚举常量、字符常量、
 * sizeof 结果，以及作为强制转换直接操作数的浮点常量。
 * 此处浮点常量 3.14 不是强制转换的直接操作数。
 * 期望：编译报错（"initializer element is not constant"）。 */
static int bad_int_const_expr = 3.14;

/* 违反约束 [6]：整数常量表达式中，强制转换只能算术→整数。
 * 此处将整数常量强制转换为指针类型，不是算术→整数。
 * 期望：编译报错。 */
static int bad_cast_to_ptr = (int)(int *)0;

/* 违反约束 [8]：算术常量表达式只能含整数常量、浮点常量、枚举常量、
 * 字符常量、sizeof 结果；转换只能算术→算术。
 * 此处将指针强制转换为整数，不是算术→算术。
 * 期望：编译报错。 */
static double bad_arith_cast = (double)(int *)0;

/* 违反约束 [8]：算术常量表达式不能含指针操作数。
 * 期望：编译报错。 */
static int bad_arith_ptr = (int)&global_obj;

/* 违反约束 [9]：地址常量的创建中不得访问对象的值。
 * 此处通过 * 解引用访问了对象的值，不是合法的地址常量。
 * 期望：编译报错（"initializer element is not constant"）。 */
static int bad_addr_deref = *&global_obj;

/* 违反约束 [9]：地址常量创建中不得访问对象值（通过 [] 访问）。
 * 期望：编译报错。 */
static int bad_addr_subscript = global_arr[0];

/* 违反约束 [7]：初始化器中的常量表达式必须是算术常量表达式、
 * 空指针常量、地址常量，或地址常量 ± 整数常量表达式。
 * 此处用两个地址常量相加，不是合法形式。
 * 期望：编译报错。 */
static int *bad_addr_add = &global_obj + &global_obj;

/* 违反约束 [6]：整数常量表达式不能含非整数类型的操作数（如指针）。
 * 期望：编译报错。 */
static int bad_int_ptr_operand = (int)(&global_obj - &global_obj);

#endif /* 负向测试结束 */

/*
 * 说明：
 * 1. 正向部分覆盖了 [1]~[11] 各段语义：常量表达式可用于数组大小、case 标签、
 *    位域宽度、枚举值、初始化器；整数/算术/地址常量表达式的操作数限制；
 *    翻译期浮点精度；短路求值（Footnote 100）；未求值子表达式中的逗号运算符。
 * 2. 负向部分针对 [3][4][6][7][8][9] 的约束，故意写出违反规则的代码，
 *    期望 gcc -std=c99 编译报错；这些片段被 #if 0 屏蔽，不影响本文件编译运行。
 * 3. 未将未定义行为（UB）作为负向测试，仅针对编译器必须拒绝的约束违反。
 */