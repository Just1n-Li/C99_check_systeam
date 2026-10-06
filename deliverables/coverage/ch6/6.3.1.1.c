/*
 * 测试 C99 6.3.1.1 —— Boolean, characters, and integers
 * 验证内容：
 *   [1] 整数转换等级（integer conversion rank）的规则
 *   [2] 整数提升（integer promotions）：rank <= int/unsigned int 的类型在表达式中提升为 int 或 unsigned int
 *   [3] 整数提升保值（含符号），plain char 的符号性由实现定义
 *
 * 预期行为：
 *   正向测试：编译通过，运行断言全部成立。
 *   负向测试：位于 #if 0 中，单独取出编译时 gcc -std=c99 应报错（违反约束）。
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 整数转换等级：用 _Generic 无法直接比较 rank，改用类型宽度/符号
 *     的语义行为间接验证。这里用 sizeof 与类型兼容性验证基本关系。
 *     注意：rank 是抽象概念，标准未提供直接查询手段，故用可观察行为验证。
 */

/* [1] 验证：signed char < short < int < long < long long 的精度递增 */
static void test_rank_precision(void)
{
    /* [1] "rank of a signed integer type shall be greater than the rank of
     *      any signed integer type with less precision"
     * 精度递增意味着 sizeof 非递减，且值域包含关系成立。 */
    assert(sizeof(signed char) <= sizeof(short));
    assert(sizeof(short) <= sizeof(int));
    assert(sizeof(int) <= sizeof(long));
    assert(sizeof(long) <= sizeof(long long));

    /* [1] "rank of long long int > long int > int > short int > signed char"
     * 用值域包含关系验证：较小 rank 类型的值域被较大 rank 类型包含。 */
    assert(SCHAR_MIN >= LLONG_MIN && SCHAR_MAX <= LLONG_MAX);
    assert(SHRT_MIN  >= LLONG_MIN && SHRT_MAX  <= LLONG_MAX);
    assert(INT_MIN   >= LLONG_MIN && INT_MAX   <= LLONG_MAX);
    assert(LONG_MIN  >= LLONG_MIN && LONG_MAX  <= LLONG_MAX);
}

/* [1] 验证：无符号类型与其对应有符号类型 rank 相等
 *     可观察行为：unsigned int 与 int 宽度相同。 */
static void test_rank_unsigned_equals_signed(void)
{
    /* [1] "rank of any unsigned integer type shall equal the rank of the
     *      corresponding signed integer type" */
    assert(sizeof(unsigned char) == sizeof(signed char));
    assert(sizeof(unsigned short) == sizeof(short));
    assert(sizeof(unsigned int) == sizeof(int));
    assert(sizeof(unsigned long) == sizeof(long));
    assert(sizeof(unsigned long long) == sizeof(long long));
}

/* [1] 验证：char 的 rank 等于 signed char 和 unsigned char */
static void test_rank_char(void)
{
    /* [1] "rank of char shall equal the rank of signed char and unsigned char"
     * 可观察：sizeof 相同。 */
    assert(sizeof(char) == sizeof(signed char));
    assert(sizeof(char) == sizeof(unsigned char));
}

/* [1] 验证：_Bool 的 rank 小于所有其他标准整数类型
 *     可观察：sizeof(_Bool) <= sizeof(char) 且 _Bool 值域最小。 */
static void test_rank_bool(void)
{
    /* [1] "rank of _Bool shall be less than the rank of all other standard
     *      integer types" */
    assert(sizeof(_Bool) <= sizeof(char));
    assert(sizeof(_Bool) <= sizeof(short));
    assert(sizeof(_Bool) <= sizeof(int));
    assert(sizeof(_Bool) <= sizeof(long));
    assert(sizeof(_Bool) <= sizeof(long long));
}

/* [1] 验证：枚举类型的 rank 等于其兼容整数类型的 rank
 *     枚举常量在表达式中参与整数提升，其行为与 int 一致。 */
enum Color { RED = 1, GREEN = 2, BLUE = 3 };

static void test_rank_enum(void)
{
    /* [1] "rank of any enumerated type shall equal the rank of the
     *      compatible integer type" */
    enum Color c = GREEN;
    /* 枚举值参与算术运算，提升为 int */
    int x = c + 1;
    assert(x == 3);
    /* 枚举类型与 int 兼容，sizeof 通常相同 */
    assert(sizeof(enum Color) == sizeof(int));
}

/* [1] 传递性：T1 > T2 且 T2 > T3 则 T1 > T3
 *     用值域包含关系验证 long long > int > short。 */
static void test_rank_transitivity(void)
{
    /* [1] "if T1 has greater rank than T2 and T2 has greater rank than T3,
     *      then T1 has greater rank than T3" */
    /* long long > int > short  =>  long long > short */
    assert(SHRT_MIN >= LLONG_MIN && SHRT_MAX <= LLONG_MAX);
    /* long long > long > int  =>  long long > int */
    assert(INT_MIN >= LLONG_MIN && INT_MAX <= LLONG_MAX);
}

/* [2] 整数提升：rank <= int 的类型在表达式中提升为 int（若 int 能表示其所有值）
 *     用 _Generic 验证提升后的类型。 */
static void test_integer_promotion_type(void)
{
    /* [2] "If an int can represent all values of the original type, the value
     *      is converted to an int" */
    signed char sc = 1;
    short sh = 1;
    unsigned char uc = 1;
    unsigned short us = 1;
    _Bool b = 1;
    char c = 1;

    /* 在表达式中，这些类型提升为 int（假设 int 能表示其所有值） */
    assert(_Generic(sc + 0, int: 1, default: 0));
    assert(_Generic(sh + 0, int: 1, default: 0));
    assert(_Generic(uc + 0, int: 1, default: 0));
    assert(_Generic(us + 0, int: 1, default: 0));
    assert(_Generic(b  + 0, int: 1, default: 0));
    assert(_Generic(c  + 0, int: 1, default: 0));

    /* [2] "All other types are unchanged by the integer promotions"
     * int 及以上类型不变 */
    int i = 1;
    long l = 1;
    long long ll = 1;
    assert(_Generic(i  + 0, int: 1, default: 0));
    assert(_Generic(l  + 0, long: 1, default: 0));
    assert(_Generic(ll + 0, long long: 1, default: 0));
}

/* [2] 整数提升：若 int 不能表示原类型所有值，则提升为 unsigned int
 *     典型场景：unsigned int 与 int 同宽时，unsigned short 提升为 int
 *     （因为 int 能表示 unsigned short 的所有值）。
 *     这里验证 unsigned int 本身不提升（rank 等于 int）。 */
static void test_promotion_unsigned_int(void)
{
    /* [2] unsigned int 的 rank 等于 int，不在提升范围内（rank <= int 才提升，
     *     但 unsigned int 的 rank 等于 int，标准说 "less than or equal to
     *     the rank of int and unsigned int"，故 unsigned int 也属于可提升范围，
     *     但 int 无法表示 unsigned int 所有值，故提升为 unsigned int（即不变）。 */
    unsigned int ui = 1;
    assert(_Generic(ui + 0, unsigned int: 1, default: 0));
}

/* [2] 位域（bit-field）的整数提升
 *     类型为 _Bool, int, signed int, unsigned int 的位域参与提升。 */
struct BitFields {
    _Bool    bf_bool : 1;
    int      bf_int  : 4;
    signed int bf_sint : 4;
    unsigned int bf_uint : 4;
};

static void test_promotion_bitfield(void)
{
    struct BitFields s;
    s.bf_bool = 1;
    s.bf_int  = 3;
    s.bf_sint = -3;
    s.bf_uint = 5;

    /* [2] 位域在表达式中提升为 int（若 int 能表示其所有值） */
    assert(_Generic(s.bf_bool + 0, int: 1, default: 0));
    assert(_Generic(s.bf_int  + 0, int: 1, default: 0));
    assert(_Generic(s.bf_sint + 0, int: 1, default: 0));
    /* unsigned int 位域：若 int 不能表示其所有值则提升为 unsigned int，
     * 但 4 位 unsigned int 的值域 0..15 能被 int 表示，故提升为 int。 */
    assert(_Generic(s.bf_uint + 0, int: 1, default: 0));

    assert(s.bf_int + 0 == 3);
    assert(s.bf_sint + 0 == -3);
    assert(s.bf_uint + 0 == 5);
}

/* [3] 整数提升保值（含符号）
 *     负的 signed char 提升后仍为负值。 */
static void test_promotion_preserves_sign(void)
{
    /* [3] "The integer promotions preserve value including sign" */
    signed char sc = -1;
    short sh = -100;
    int promoted_sc = sc;   /* 提升为 int，值仍为 -1 */
    int promoted_sh = sh;   /* 提升为 int，值仍为 -100 */
    assert(promoted_sc == -1);
    assert(promoted_sh == -100);

    /* 在表达式中验证 */
    assert(sc + 0 == -1);
    assert(sh + 0 == -100);
    assert((int)sc == -1);
    assert((int)sh == -100);
}

/* [3] plain char 的符号性由实现定义
 *     这里只验证 char 要么是 signed 要么是 unsigned，不假设具体哪种。 */
static void test_plain_char_signedness(void)
{
    /* [3] "whether a ''plain'' char is treated as signed is
     *      implementation-defined" */
    char c = (char)-1;
    /* 无论 char 是有符号还是无符号，以下断言都成立：
     * 若 char 有符号，c == -1；若无符号，c == 255（或 UCHAR_MAX）。 */
    assert(c == -1 || (unsigned char)c == UCHAR_MAX);
}

/* [2] 整数提升在移位运算符操作数上的应用
 *     （脚注 48：整数提升应用于移位运算符的两个操作数） */
static void test_promotion_shift_operands(void)
{
    /* [2] 移位运算符的操作数经历整数提升 */
    unsigned char uc = 1;
    signed char sc = 1;
    /* uc 提升为 int，移位结果为 int */
    assert(_Generic(uc << 1, int: 1, default: 0));
    assert(_Generic(sc << 1, int: 1, default: 0));
    assert((uc << 3) == 8);
    assert((sc << 3) == 8);
}

/* [2] 整数提升在一元 +, -, ~ 运算符上的应用
 *     （脚注 48） */
static void test_promotion_unary_operands(void)
{
    unsigned char uc = 1;
    signed char sc = 1;
    /* 一元 + 的结果类型为提升后的类型 */
    assert(_Generic(+uc, int: 1, default: 0));
    assert(_Generic(-uc, int: 1, default: 0));
    assert(_Generic(~uc, int: 1, default: 0));
    assert(_Generic(+sc, int: 1, default: 0));
    assert(_Generic(-sc, int: 1, default: 0));
    assert(_Generic(~sc, int: 1, default: 0));

    assert(+uc == 1);
    assert(-uc == -1);
    assert(~uc == ~1);
}

/* [2] 整数提升在通常算术转换中的应用
 *     （脚注 48：作为通常算术转换的一部分） */
static void test_promotion_usual_arithmetic(void)
{
    unsigned char uc = 200;
    signed char sc = 100;
    /* uc 提升为 int，sc 提升为 int，相加结果为 int */
    assert(_Generic(uc + sc, int: 1, default: 0));
    assert(uc + sc == 300);

    short sh = 1000;
    /* sh 提升为 int */
    assert(_Generic(sh * 2, int: 1, default: 0));
    assert(sh * 2 == 2000);
}

/* [2] 整数提升在函数实参上的应用
 *     （脚注 48：应用于某些实参表达式） */
static int takes_int(int x) { return x; }

static void test_promotion_function_args(void)
{
    signed char sc = -5;
    short sh = -500;
    unsigned char uc = 200;
    /* 实参经历整数提升为 int */
    assert(takes_int(sc) == -5);
    assert(takes_int(sh) == -500);
    assert(takes_int(uc) == 200);
}

/* [2] 无原型函数调用的默认实参提升
 *     char -> int, float -> double, 且省略号后停止转换 */
static int no_proto_sum();  /* 无原型声明 */

static int no_proto_sum(int a, int b)
{
    return a + b;
}

static void test_no_prototype_default_arg_promotion(void)
{
    /* [2] 无原型函数调用时，char 提升为 int，float 提升为 double */
    char c = 10;
    short s = 20;
    /* 调用无原型函数，实参经历默认实参提升 */
    int r = no_proto_sum(c, s);
    assert(r == 30);
}

/* [3] 验证提升后的值在边界情况下保值 */
static void test_promotion_boundary(void)
{
    /* signed char 的最小值提升后仍为负 */
    signed char sc_min = SCHAR_MIN;
    int promoted = sc_min;
    assert(promoted == SCHAR_MIN);
    assert(promoted < 0);

    /* unsigned char 的最大值提升后仍为正 */
    unsigned char uc_max = UCHAR_MAX;
    int promoted_uc = uc_max;
    assert(promoted_uc == UCHAR_MAX);
    assert(promoted_uc > 0);

    /* short 的最小值 */
    short sh_min = SHRT_MIN;
    int promoted_sh = sh_min;
    assert(promoted_sh == SHRT_MIN);
}

int main(void)
{
    test_rank_precision();
    test_rank_unsigned_equals_signed();
    test_rank_char();
    test_rank_bool();
    test_rank_enum();
    test_rank_transitivity();

    test_integer_promotion_type();
    test_promotion_unsigned_int();
    test_promotion_bitfield();
    test_promotion_preserves_sign();
    test_plain_char_signedness();
    test_promotion_shift_operands();
    test_promotion_unary_operands();
    test_promotion_usual_arithmetic();
    test_promotion_function_args();
    test_no_prototype_default_arg_promotion();
    test_promotion_boundary();

    printf("All C99 6.3.1.1 tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「整数提升不改变 rank 高于 int 的类型」：
 * 下面代码本身不违反约束，仅作说明。真正的负向测试需要违反约束的代码。
 * 6.3.1.1 主要是描述性条款（Semantics），约束较少。
 * 以下片段演示违反相关约束的情况。
 */

/* 违反约束「_Bool 的 rank 小于所有其他标准整数类型」：
 * 无法在源码中直接违反 rank 规则，rank 由实现定义。
 * 但可以违反「位域类型必须是 _Bool, int, signed int, unsigned int」的约束
 * （该约束来自 6.7.2.1，与 6.3.1.1 [2] 位域提升相关）。
 * 期望报错：bit-field 'bf' has invalid type */
struct BadBitField {
    char bf : 4;   /* 违反 6.7.2.1：位域类型不能是 char */
};

/* 违反约束「枚举类型的 rank 等于兼容整数类型」：
 * 无法直接违反，rank 由实现定义。
 * 但可以违反枚举常量必须是 int 可表示值的约束（6.7.2.2）。
 * 期望报错：enumerator value outside range of int */
enum BadEnum {
    BIG = 0xFFFFFFFFFFFFFFFFULL   /* 超出 int 范围 */
};

/* 违反约束「整数提升后类型必须为 int 或 unsigned int」：
 * 无法直接违反，提升规则由实现定义。
 * 但可以违反「移位运算符操作数必须是整数类型」的约束（6.5.7）。
 * 期望报错：invalid operands to binary << */
struct S { int x; } s1, s2;
void bad_shift(void) {
    s1 << s2;   /* 违反 6.5.7：移位操作数必须是整数类型 */
}

/* 违反约束「一元 +, -, ~ 操作数必须是算术类型」（6.5.3.3）。
 * 期望报错：wrong type argument to unary minus */
void bad_unary(void) {
    struct S s;
    -s;   /* 违反 6.5.3.3：结构体不能取负 */
}

/* 违反约束「通常算术转换要求操作数为算术类型」（6.3.1.8）。
 * 期望报错：invalid operands to binary + */
void bad_arithmetic(void) {
    struct S a, b;
    a + b;   /* 违反 6.3.1.8：结构体不能相加 */
}

/* 违反约束「非左值不能赋值」（6.5.16.1）。
 * 函数返回结构体的成员 f().x 不是左值。
 * 期望报错：lvalue required as left operand of assignment */
struct S make_s(void) { struct S s = {0}; return s; }
void bad_assign_nonlvalue(void) {
    make_s().x = 5;   /* 违反 6.5.16.1：非左值不能赋值 */
}

/* 违反约束「强制转换结果不是左值」（6.5.4）。
 * 期望报错：lvalue required as left operand of assignment */
void bad_assign_cast(void) {
    int x = 0;
    (int)x = 5;   /* 违反 6.5.4：强制转换结果不是左值 */
}

/* 违反约束「条件表达式结果不是左值」（6.5.15）。
 * 期望报错：lvalue required as left operand of assignment */
void bad_assign_conditional(void) {
    int a = 1, b = 2;
    (1 ? a : b) = 5;   /* 违反 6.5.15：条件表达式结果不是左值 */
}

/* 违反约束「逗号表达式结果不是左值」（6.5.17）。
 * 期望报错：lvalue required as left operand of assignment */
void bad_assign_comma(void) {
    int a = 1, b = 2;
    (a, b) = 5;   /* 违反 6.5.17：逗号表达式结果不是左值 */
}

/* 违反约束「const 限定类型不能赋值」（6.3.1.1 相关：const 传播）。
 * 期望报错：assignment of read-only member */
struct ConstS { const int x; };
void bad_assign_const_member(void) {
    struct ConstS cs = {0};
    cs.x = 5;   /* 违反 6.3.1.1 相关：const 成员不能赋值 */
}

/* 违反约束「const 限定类型解引用后不能赋值」。
 * 期望报错：assignment of read-only location */
void bad_assign_const_deref(void) {
    const int ci = 0;
    const int *p = &ci;
    *p = 5;   /* 违反 6.3.1.1 相关：const 解引用不能赋值 */
}

#endif /* 负向测试结束 */