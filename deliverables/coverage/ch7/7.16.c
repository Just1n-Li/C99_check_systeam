/*
 * 测试 C99 7.16 <stdbool.h>：Boolean type and values
 *
 * 预期行为：
 *   正向测试：包含 <stdbool.h> 后，bool 展开为 _Bool，true 为 1，false 为 0，
 *             __bool_true_false_are_defined 为 1；三个宏可用于 #if；
 *             程序可以 #undef 并重新定义 bool/true/false。
 *   负向测试：违反约束的片段应导致编译报错（见文件末尾 #if 0 块）。
 */

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] bool 展开为 _Bool：用 _Generic 或类型检查验证 */
static void test_bool_is__Bool(void)
{
    /* 若 bool 不是 _Bool，则下面这个 _Generic 选择会失败 */
    bool b = 0;
    int which = _Generic(b,
                         _Bool: 1,
                         default: 0);
    assert(which == 1);          /* [2] bool 就是 _Bool */

    /* _Bool 的语义：非零赋值为 1，零为 0 */
    b = 42;
    assert(b == 1);
    b = 0;
    assert(b == 0);
    b = true;
    assert(b == 1);
    b = false;
    assert(b == 0);
}

/* [3] true 展开为整数常量 1，false 展开为整数常量 0 */
static void test_true_false_values(void)
{
    assert(true == 1);           /* [3] true 是 1 */
    assert(false == 0);          /* [3] false 是 0 */

    /* 它们是整数常量，可用于需要常量表达式的地方 */
    int arr[true + 1];           /* 大小 2，编译期常量 */
    assert(sizeof(arr) / sizeof(arr[0]) == 2);

    switch (1) {
    case true:                   /* 常量表达式 */
        break;
    default:
        assert(0);
    }
}

/* [3] __bool_true_false_are_defined 展开为整数常量 1 */
static void test_defined_macro(void)
{
    assert(__bool_true_false_are_defined == 1);   /* [3] */
}

/* [3] 三个宏适合用于 #if 预处理指令 */
#if !true
#error "true should be nonzero in #if"
#endif
#if false
#error "false should be zero in #if"
#endif
#if !__bool_true_false_are_defined
#error "__bool_true_false_are_defined should be nonzero in #if"
#endif

/* [4] 程序可以 undefine 并重新定义 bool、true、false */
#undef bool
#undef true
#undef false

#define bool  int
#define true  100
#define false (-100)

static void test_redefine(void)
{
    bool x = true;               /* 重新定义后 bool 是 int，true 是 100 */
    assert(x == 100);
    x = false;
    assert(x == -100);
}

/* 恢复标准定义，便于后续代码使用 */
#undef bool
#undef true
#undef false
#include <stdbool.h>

static void test_restore(void)
{
    bool b = true;
    assert(b == 1);
    assert(false == 0);
}

int main(void)
{
    test_bool_is__Bool();
    test_true_false_values();
    test_defined_macro();
    test_redefine();
    test_restore();

    printf("C99 7.16 <stdbool.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「bool 展开为 _Bool」：把 bool 当作可赋任意指针的类型使用，
 * 若 bool 正确展开为 _Bool，则 _Bool 不能保存指针，gcc -std=c99 应报错
 * （incompatible types / initialization makes integer from pointer）。 */
#include <stdbool.h>
void f1(void)
{
    bool b = (void *)0;   /* 期望报错：指针赋给 _Bool */
    (void)b;
}

/* 违反约束「true 展开为整数常量 1」：把 true 用作左值赋值，
 * 整数常量不是左值，gcc -std=c99 应报错（lvalue required as left operand）。 */
#include <stdbool.h>
void f2(void)
{
    true = 0;             /* 期望报错：true 不是左值 */
}

/* 违反约束「false 展开为整数常量 0」：对 false 取地址，
 * 整数常量没有地址，gcc -std=c99 应报错（lvalue required as unary '&' operand）。 */
#include <stdbool.h>
void f3(void)
{
    int *p = &false;      /* 期望报错：false 不是左值 */
    (void)p;
}

/* 违反约束「__bool_true_false_are_defined 展开为整数常量 1」：
 * 把它用作左值赋值，gcc -std=c99 应报错（lvalue required）。 */
#include <stdbool.h>
void f4(void)
{
    __bool_true_false_are_defined = 0;   /* 期望报错：不是左值 */
}

/* 违反约束「bool 展开为 _Bool」：在需要结构体类型的地方使用 bool，
 * 若 bool 是 _Bool，则不能定义结构体成员为 bool 后再用 . 访问不存在的成员，
 * 这里用 sizeof(bool) 与结构体比较来触发类型错误（示例：把 bool 当结构体用）。 */
#include <stdbool.h>
void f5(void)
{
    bool b;
    b.member = 1;         /* 期望报错：_Bool 没有成员 */
}

#endif /* 负向测试结束 */