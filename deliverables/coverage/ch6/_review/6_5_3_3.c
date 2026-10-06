/*
 * 验证 C99 条款 6.5.3.3 Unary arithmetic operators
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    char c = 5;
    short s = 10;
    int i = 20;
    float f = 1.5f;
    double d = 2.5;

    /* [2] unary + : 算术类型，整数提升 */
    assert(+c == 5);
    assert(+s == 10);
    assert(+i == 20);
    assert(+f == 1.5f);
    assert(+d == 2.5);

    /* 验证类型提升：char 和 short 提升为 int */
    assert(sizeof(+c) == sizeof(int));
    assert(sizeof(+s) == sizeof(int));
    assert(sizeof(+i) == sizeof(int));
    assert(sizeof(+f) == sizeof(float));
    assert(sizeof(+d) == sizeof(double));

    /* [3] unary - : 算术类型，整数提升 */
    assert(-c == -5);
    assert(-s == -10);
    assert(-i == -20);
    assert(-f == -1.5f);
    assert(-d == -2.5);

    assert(sizeof(-c) == sizeof(int));
    assert(sizeof(-s) == sizeof(int));

    /* [4] ~ : 整型，按位取反，整数提升 */
    unsigned char uc = 5;
    unsigned int ui = 10;
    int i2 = 10;

    /* 验证提升后的类型 */
    assert(sizeof(~uc) == sizeof(int)); /* unsigned char 提升为 int */
    
    /* 验证按位取反 */
    assert(~i2 == ~10);
    
    /* 验证无符号类型：~E 等价于最大值减去 E */
    /* 对于 unsigned int, ~ui == UINT_MAX - ui */
    assert(~ui == (UINT_MAX - ui));
    
    /* 对于 unsigned char 提升为 int，不是 unsigned int，所以 ~uc == ~(int)uc */
    assert(~uc == ~(int)uc);

    /* [5] ! : 标量类型，结果为 0 或 1，类型为 int */
    int zero = 0;
    int non_zero = 5;
    float fzero = 0.0f;
    float fnon_zero = 1.5f;
    int *ptr_null = NULL;
    int *ptr_valid = &i;

    assert(!zero == 1);
    assert(!non_zero == 0);
    assert(!fzero == 1);
    assert(!fnon_zero == 0);
    assert(!ptr_null == 1);
    assert(!ptr_valid == 0);

    /* 验证结果类型为 int */
    assert(sizeof(!zero) == sizeof(int));
    
    /* 验证 !E 等价于 (0==E) */
    assert(!zero == (0==zero));
    assert(!non_zero == (0==non_zero));
    assert(!fzero == (0==fzero));
    assert(!ptr_null == (0==ptr_null));

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    struct S { int x; } s_obj;
    int arr[10];
    int *ptr;

    /* [1] 违反约束：unary + 的操作数必须为算术类型，结构体不是算术类型 */
    +s_obj;

    /* [1] 违反约束：unary - 的操作数必须为算术类型，指针不是算术类型 */
    -ptr;

    /* [1] 违反约束：~ 的操作数必须为整型，浮点型不是整型 */
    ~1.5;

    /* [1] 违反约束：~ 的操作数必须为整型，结构体不是整型 */
    ~s_obj;

    /* [1] 违反约束：~ 的操作数必须为整型，指针不是整型 */
    ~ptr;

    /* [1] 违反约束：! 的操作数必须为标量类型，结构体不是标量类型 */
    !s_obj;

    /* [1] 违反约束：! 的操作数必须为标量类型，数组不是标量类型 */
    !arr;
#endif

    return 0;
}