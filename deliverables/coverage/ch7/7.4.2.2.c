/*
 * 测试 C99 7.4.2.2 —— toupper 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明 #include <ctype.h>  int toupper(int c);
 *   [2] 语义：把小写字母转换为对应的大写字母
 *   [3] 返回值：若 islower(c) 为真且存在对应的大写字符，返回其中之一
 *               （同一 locale 下总是同一个）；否则原样返回参数。
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型：toupper 接受 int 参数，返回 int。
 *     验证函数指针类型与标准原型一致。 */
static int (*toupper_ptr)(int) = toupper;

/* [2] 语义：小写字母 -> 对应大写字母 */
static void test_semantics_lowercase(void)
{
    assert(toupper('a') == 'A');
    assert(toupper('b') == 'B');
    assert(toupper('z') == 'Z');
    assert(toupper('m') == 'M');

    /* 遍历所有小写字母，验证转换结果就是对应的大写字母 */
    for (int c = 'a'; c <= 'z'; ++c) {
        int up = toupper(c);
        assert(up == c - 'a' + 'A');
        assert(isupper(up));
    }
}

/* [3] 返回值：非小写字母的参数应原样返回 */
static void test_returns_unchanged(void)
{
    /* 大写字母原样返回 */
    assert(toupper('A') == 'A');
    assert(toupper('Z') == 'Z');

    /* 数字原样返回 */
    assert(toupper('0') == '0');
    assert(toupper('9') == '9');

    /* 标点、空白原样返回 */
    assert(toupper(' ') == ' ');
    assert(toupper('!') == '!');
    assert(toupper('~') == '~');
    assert(toupper('\n') == '\n');
    assert(toupper('\t') == '\t');

    /* 非字母的可打印字符原样返回 */
    for (int c = 0; c < 128; ++c) {
        if (!islower(c)) {
            assert(toupper(c) == c);
        }
    }
}

/* [3] 返回值：对 islower 为真的字符，结果应满足 isupper 为真，
 *     且同一 locale 下对同一输入总是返回同一个字符（幂等/确定性）。 */
static void test_deterministic_and_upper(void)
{
    for (int c = 'a'; c <= 'z'; ++c) {
        int first = toupper(c);
        int second = toupper(c);
        assert(first == second);      /* 同一 locale 下结果一致 */
        assert(isupper(first));       /* 结果是大写字符 */
        /* 再次应用 toupper 应保持不变（已是大写） */
        assert(toupper(first) == first);
    }
}

/* [1] 通过函数指针调用，验证原型可用 */
static void test_via_pointer(void)
{
    assert(toupper_ptr('q') == 'Q');
    assert(toupper_ptr('Q') == 'Q');
}

/* [1] 参数为 int：EOF 等非字符值应原样返回（不属于 islower 为真的字符） */
static void test_eof_and_int_arg(void)
{
    assert(toupper(EOF) == EOF);
}

int main(void)
{
    test_semantics_lowercase();
    test_returns_unchanged();
    test_deterministic_and_upper();
    test_via_pointer();
    test_eof_and_int_arg();

    printf("C99 7.4.2.2 toupper: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「toupper 的原型为 int toupper(int)」：
 * 用不兼容的实参类型调用（例如传结构体），
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'toupper'。 */
struct S { int x; };
void bad_arg(void)
{
    struct S s;
    (void)toupper(s);   /* 期望编译报错：实参类型不兼容 */
}

/* 违反约束「toupper 返回 int，不是指针」：
 * 把返回值当作指针解引用，类型不匹配，
 * gcc -std=c99 应报错：invalid type argument of unary '*'。 */
void bad_return_use(void)
{
    int v = *toupper('a');   /* 期望编译报错：对 int 解引用 */
    (void)v;
}

/* 违反约束「toupper 需要恰好一个实参」：
 * 参数个数不匹配，gcc -std=c99 应报错。 */
void bad_arity_zero(void)
{
    (void)toupper();          /* 期望编译报错：实参太少 */
}
void bad_arity_two(void)
{
    (void)toupper('a', 'b');  /* 期望编译报错：实参太多 */
}

/* 违反约束「toupper 是函数，不是对象」：
 * 对函数名赋值，gcc -std=c99 应报错。 */
void bad_assign_to_function(void)
{
    toupper = 0;              /* 期望编译报错：对函数赋值 */
}

#endif /* 负向测试结束 */