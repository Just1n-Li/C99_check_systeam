/*
 * 测试 C99 7.25.3.2.1 —— towctrans 函数
 *
 * 预期行为：
 *   正向测试：包含 <wctype.h>，调用 towctrans / wctrans，验证
 *             towctrans(wc, wctrans("tolower")) == towlower(wc)
 *             towctrans(wc, wctrans("toupper")) == towupper(wc)
 *             以及返回值语义 [4]。
 *   负向测试：违反约束的代码应导致编译报错（放在 #if 0 中）。
 *
 * 说明：本条款本身没有显式的 "Constraints" 段，负向测试针对
 *       函数原型/参数类型约束（wint_t / wctrans_t）以及头文件声明。
 */

#include <stdio.h>
#include <wctype.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：头文件 <wctype.h> 提供 towctrans 声明，
 *     原型为 wint_t towctrans(wint_t wc, wctrans_t desc);
 *     通过取函数指针验证签名匹配。 */
static wint_t (*fp_towctrans)(wint_t, wctrans_t) = towctrans;

int main(void)
{
    /* 使用默认 "C" locale 即可，wctrans 在 C locale 下也须可用。 */
    setlocale(LC_CTYPE, "C");

    /* [1] 原型可用性：函数指针非空。 */
    assert(fp_towctrans != NULL);

    /* [2] wctrans("tolower") / wctrans("toupper") 返回有效的 wctrans_t。 */
    wctrans_t lower = wctrans("tolower");
    wctrans_t upper = wctrans("toupper");
    assert(lower != (wctrans_t)0);
    assert(upper != (wctrans_t)0);

    /* [3] towctrans(wc, wctrans("tolower")) 行为等同 towlower(wc)。
     *     在 C locale 下测试若干宽字符。 */
    {
        wint_t samples[] = { L'A', L'Z', L'a', L'z', L'0', L'!', L' ' };
        size_t i;
        for (i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
            wint_t wc = samples[i];
            /* [3] 与 towlower 等价 */
            assert(towctrans(wc, lower) == towlower(wc));
            /* [3] 与 towupper 等价 */
            assert(towctrans(wc, upper) == towupper(wc));
        }
    }

    /* [3] 具体语义检查：'A' -> 'a'（tolower），'a' -> 'A'（toupper）。 */
    assert(towctrans(L'A', lower) == L'a');
    assert(towctrans(L'a', upper) == L'A');

    /* [4] 返回值：返回映射后的值；对无映射字符应返回原字符。
     *     在 C locale 下 '0' 无大小写映射，应原样返回。 */
    assert(towctrans(L'0', lower) == L'0');
    assert(towctrans(L'0', upper) == L'0');

    /* [4] 返回值类型为 wint_t，可安全与 wint_t 比较。 */
    {
        wint_t r = towctrans(L'B', lower);
        assert(r == (wint_t)L'b');
    }

    /* [2] 同一 desc 可重复使用（LC_CTYPE 未变）。 */
    assert(towctrans(L'C', lower) == towctrans(L'C', lower));

    printf("C99 7.25.3.2.1 towctrans: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「towctrans 的第一个参数类型为 wint_t」：
 * 传入结构体类型，gcc -std=c99 应报 incompatible type 错误。 */
struct S { int x; };
struct S s;
void bad1(void) {
    towctrans(s, wctrans("tolower"));   /* 错误：第一个实参不是 wint_t */
}

/* 违反约束「towctrans 的第二个参数类型为 wctrans_t」：
 * 传入字符串字面量（char*），应报 incompatible type 错误。 */
void bad2(void) {
    towctrans(L'A', "tolower");         /* 错误：第二个实参不是 wctrans_t */
}

/* 违反约束「wctrans 的参数为 const char *」：
 * 传入整数，应报 incompatible type 错误。 */
void bad3(void) {
    wctrans(42);                        /* 错误：实参不是 const char * */
}

/* 违反约束「towctrans 返回 wint_t」：
 * 把返回值赋给结构体，应报 incompatible type 错误。 */
void bad4(void) {
    struct S t;
    t = towctrans(L'A', wctrans("tolower"));  /* 错误：wint_t 不能赋给 struct S */
}

/* 违反约束「调用 towctrans 需可见原型」：
 * 若未包含 <wctype.h>，隐式声明与 C99 约束冲突（此处演示参数个数错误）。
 * 期望：too few arguments to function 'towctrans'。 */
void bad5(void) {
    towctrans(L'A');                    /* 错误：实参个数不足 */
}

#endif /* 负向测试结束 */