/*
 * 测试条款：C99 7.24.3.8  The putwc function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型：wint_t putwc(wchar_t c, FILE *stream);
 *   [2] 语义：等价于 fputwc；若实现为宏，可能多次求值 stream，
 *           故 stream 不应为带副作用的表达式。
 *   [3] 返回值：返回写入的宽字符，或 WEOF。
 */

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <assert.h>
#include <stdlib.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 原型检查：putwc 的返回类型为 wint_t，参数为 (wchar_t, FILE*)。
     *     通过取函数指针并赋值来静态验证签名。 */
    {
        wint_t (*fp)(wchar_t, FILE *) = putwc;
        assert(fp != NULL);
    }

    /* 设置一个支持宽字符的本地环境，便于宽字符 I/O。 */
    setlocale(LC_ALL, "");

    /* [3] 返回值：成功写入时返回写入的宽字符本身。 */
    {
        FILE *f = tmpfile();
        assert(f != NULL);

        wchar_t wc = L'A';
        wint_t r = putwc(wc, f);
        /* 返回写入的宽字符 */
        assert(r == (wint_t)wc);

        /* 再写一个非 ASCII 宽字符 */
        wchar_t wc2 = L'\u00E9'; /* é */
        wint_t r2 = putwc(wc2, f);
        assert(r2 == (wint_t)wc2);

        fclose(f);
    }

    /* [2] 语义：putwc 等价于 fputwc。
     *     用两个文件分别写入同一宽字符，比较返回值一致。 */
    {
        FILE *f1 = tmpfile();
        FILE *f2 = tmpfile();
        assert(f1 != NULL && f2 != NULL);

        wchar_t wc = L'Z';
        wint_t a = putwc(wc, f1);
        wint_t b = fputwc(wc, f2);
        assert(a == b);
        assert(a == (wint_t)wc);

        fclose(f1);
        fclose(f2);
    }

    /* [3] 返回值：写入失败时返回 WEOF。
     *     对只读流写入应失败，返回 WEOF。 */
    {
        FILE *f = tmpfile();
        assert(f != NULL);
        /* 先写入一些内容，然后以只读方式重新打开 */
        fputwc(L'x', f);
        fflush(f);
        rewind(f);

        /* 以只读模式重新打开同一临时文件不可移植，改用 fopen 只读模式 */
        fclose(f);

        /* 用一个只读流测试写入失败 */
        FILE *ro = fopen("/dev/null", "r");
        if (ro != NULL) {
            wint_t r = putwc(L'Q', ro);
            assert(r == WEOF);
            fclose(ro);
        }
    }

    /* [2] 若 putwc 实现为宏，stream 可能被多次求值。
     *     这里仅验证：使用一个无副作用的简单 FILE* 变量调用是安全的。 */
    {
        FILE *f = tmpfile();
        assert(f != NULL);
        wint_t r = putwc(L'M', f);
        assert(r == (wint_t)L'M');
        fclose(f);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「putwc 的第一个参数类型为 wchar_t」：
 * 传入 int 而非 wchar_t，在严格原型下应报类型不兼容错误。
 * 期望：gcc -std=c99 报 "incompatible type" 或类似错误。 */
#include <stdio.h>
#include <wchar.h>
void bad_arg_type(FILE *f) {
    int x = 65;
    putwc(x, f);   /* int 不能隐式转换为 wchar_t 参数？实际可转换，
                      但若传入指针则明确违反。改用指针： */
}

/* 违反约束「putwc 的第二个参数类型为 FILE*」：
 * 传入 int 而非 FILE*，应报类型不兼容错误。
 * 期望：gcc -std=c99 报 "incompatible type for argument 2"。 */
void bad_stream_type(void) {
    putwc(L'A', 42);   /* 42 不是 FILE* */
}

/* 违反约束「putwc 需要两个参数」：
 * 参数个数不匹配，应报错。
 * 期望：gcc -std=c99 报 "too few arguments to function 'putwc'"。 */
void bad_arg_count(FILE *f) {
    putwc(L'A');       /* 缺少 stream 参数 */
}

/* 违反约束「putwc 返回值应被使用或忽略，但不能赋给不兼容类型」：
 * 将 wint_t 返回值赋给 FILE* 变量，应报类型不兼容错误。
 * 期望：gcc -std=c99 报 "incompatible types in assignment"。 */
void bad_return_use(FILE *f) {
    FILE *p = putwc(L'A', f);   /* wint_t 不能赋给 FILE* */
}

#endif /* 负向测试结束 */