/*
 * 测试 C99 7.4.2.1 —— tolower 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：int tolower(int c);  头文件 <ctype.h>
 *   [2] 描述：把大写字母转换为对应的小写字母。
 *   [3] 返回：若参数满足 isupper 为真，且当前 locale 下存在一个或多个
 *            满足 islower 为真的对应字符，则返回其中之一（同一 locale 下
 *            总是同一个）；否则原样返回参数。
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：tolower 接受 int 参数并返回 int。
 *     通过函数指针类型匹配来验证原型签名。 */
static int (*tolower_fp)(int) = tolower;

/* [2] 描述：大写字母 -> 对应小写字母 */
static void test_uppercase_to_lowercase(void)
{
    /* 基本 ASCII 大写字母 */
    assert(tolower('A') == 'a');   /* [2][3] */
    assert(tolower('B') == 'b');
    assert(tolower('Z') == 'z');
    assert(tolower('M') == 'm');

    /* 遍历所有大写字母，验证 isupper 为真时 tolower 返回 islower 为真的字符 */
    for (int c = 0; c < 128; c++) {
        if (isupper(c)) {
            int r = tolower(c);
            /* [3] 返回值必须是 islower 为真的对应字符 */
            assert(islower(r));
            /* [3] 同一 locale 下总是同一个：重复调用结果一致 */
            assert(tolower(c) == r);
        }
    }
}

/* [3] 非大写字母：原样返回 */
static void test_non_uppercase_unchanged(void)
{
    /* 小写字母原样返回 */
    assert(tolower('a') == 'a');   /* [3] */
    assert(tolower('z') == 'z');
    assert(tolower('m') == 'm');

    /* 数字原样返回 */
    assert(tolower('0') == '0');
    assert(tolower('9') == '9');

    /* 标点、空白原样返回 */
    assert(tolower(' ') == ' ');
    assert(tolower('!') == '!');
    assert(tolower('\n') == '\n');
    assert(tolower('\t') == '\t');

    /* 遍历所有非大写字符，验证原样返回 */
    for (int c = 0; c < 128; c++) {
        if (!isupper(c)) {
            assert(tolower(c) == c);   /* [3] 否则原样返回 */
        }
    }
}

/* [1] 参数类型为 int：可传入 EOF 及任意 int 值，行为由 [3] 决定 */
static void test_int_argument(void)
{
    /* EOF 不是大写字母，应原样返回 */
    assert(tolower(EOF) == EOF);   /* [3] */

    /* 传入 unsigned char 范围内的值（0..UCHAR_MAX）应安全 */
    for (int c = 0; c <= UCHAR_MAX; c++) {
        int r = tolower(c);
        if (isupper(c)) {
            assert(islower(r));        /* [3] */
        } else {
            assert(r == c);            /* [3] */
        }
    }
}

/* [3] 幂等性：对已经是小写的结果再次调用不变 */
static void test_idempotent(void)
{
    for (int c = 'A'; c <= 'Z'; c++) {
        int lower = tolower(c);
        assert(tolower(lower) == lower);   /* [3] 小写再转仍为自身 */
    }
}

/* [1] 通过函数指针调用，验证原型可用 */
static void test_via_pointer(void)
{
    assert(tolower_fp('A') == 'a');   /* [1][2] */
    assert(tolower_fp('a') == 'a');   /* [3] */
}

int main(void)
{
    test_uppercase_to_lowercase();
    test_non_uppercase_unchanged();
    test_int_argument();
    test_idempotent();
    test_via_pointer();

    printf("C99 7.4.2.1 tolower: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「tolower 的原型为 int tolower(int)」：
 * 以不兼容的参数类型（结构体）调用，gcc -std=c99 应报错
 * （incompatible type for argument / passing struct to int parameter）。 */
struct S { int x; };
void bad_call_struct(void)
{
    struct S s;
    tolower(s);   /* 错误：参数应为 int，不能传结构体 */
}

/* 违反约束「tolower 返回 int」：
 * 把返回值当作结构体使用（成员访问），类型不匹配，应报错。 */
void bad_return_use(void)
{
    struct S s;
    s = tolower('A');   /* 错误：int 不能赋给 struct S */
}

/* 违反约束「tolower 需要 <ctype.h> 中的原型」：
 * 若未包含 <ctype.h>，在 C99 中隐式函数声明被禁止，
 * 调用 tolower 应报错（implicit declaration of function）。
 * 注意：本片段假设未包含 <ctype.h> 时编译。 */
void bad_no_prototype(void)
{
    /* 在未声明 tolower 的情况下调用，C99 禁止隐式声明，应报错 */
    int r = tolower('A');   /* 错误：implicit declaration of function 'tolower' */
    (void)r;
}

/* 违反约束「tolower 的参数为 int」：
 * 传入指针类型，与 int 不兼容，应报错。 */
void bad_call_pointer(void)
{
    char *p = "A";
    tolower(p);   /* 错误：char* 不能转换为 int 参数 */
}

#endif /* 负向测试结束 */