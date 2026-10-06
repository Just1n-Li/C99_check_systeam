/*
 * 测试目标：C99 7.24.3.10  The ungetwc function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 声明与原型（<wchar.h> 中 wint_t ungetwc(wint_t, FILE*)）
 *   [2] 推回语义、逆序返回、定位函数丢弃推回字符、外部存储不变
 *   [3] 保证一个宽字符推回；过多推回可能失败
 *   [4] c == WEOF 时失败且流不变
 *   [5] 成功调用清除 EOF 指示器；文件位置指示器语义
 *   [6] 返回值：成功返回推回的宽字符，失败返回 WEOF
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* 辅助：创建一个包含宽字符内容的临时文件，返回文件名 */
static const char *make_wfile(const char *name, const wchar_t *ws)
{
    FILE *f = fopen(name, "wb");
    assert(f != NULL);
    /* 以宽字符形式写入（实现定义的编码，但同一实现读写一致） */
    for (const wchar_t *p = ws; *p; ++p) {
        assert(fputwc(*p, f) != WEOF);
    }
    assert(fclose(f) == 0);
    return name;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型存在性：函数指针类型匹配 wint_t (*)(wint_t, FILE*) */
    {
        wint_t (*fp)(wint_t, FILE *) = ungetwc;
        assert(fp != NULL);
    }

    /* [2][6] 基本推回：推回一个宽字符，随后读取应得到该字符；
     *        成功时返回推回的宽字符本身。 */
    {
        const char *fn = make_wfile("t_ungetwc_1.tmp", L"ABC");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        wint_t c1 = fgetwc(f);          /* 读 'A' */
        assert(c1 == L'A');

        wint_t r = ungetwc(c1, f);      /* 推回 'A' */
        assert(r == c1);                /* [6] 返回推回的宽字符 */

        wint_t c2 = fgetwc(f);          /* 应再次读到 'A' */
        assert(c2 == L'A');

        wint_t c3 = fgetwc(f);          /* 继续读到 'B' */
        assert(c3 == L'B');

        assert(fclose(f) == 0);
        remove(fn);
    }

    /* [2] 逆序返回：连续推回多个宽字符，读取时按推回的逆序返回。 */
    {
        const char *fn = make_wfile("t_ungetwc_2.tmp", L"XY");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        wint_t x = fgetwc(f);           /* 'X' */
        wint_t y = fgetwc(f);           /* 'Y' */
        assert(x == L'X' && y == L'Y');

        /* 先推回 'X'，再推回 'Y'；读取顺序应为 'Y' 然后 'X' */
        assert(ungetwc(x, f) == x);
        assert(ungetwc(y, f) == y);

        assert(fgetwc(f) == L'Y');      /* 逆序：最后推回的先返回 */
        assert(fgetwc(f) == L'X');

        assert(fclose(f) == 0);
        remove(fn);
    }

    /* [2] 定位函数（fseek）丢弃所有推回字符，外部存储不变。 */
    {
        const char *fn = make_wfile("t_ungetwc_3.tmp", L"HELLO");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        wint_t h = fgetwc(f);           /* 'H' */
        assert(h == L'H');
        assert(ungetwc(h, f) == h);     /* 推回 'H' */

        /* fseek 丢弃推回字符，回到文件开头 */
        assert(fseek(f, 0, SEEK_SET) == 0);

        /* 定位后读取应为文件首字符 'H'（推回被丢弃，不是推回的 'H' 优先） */
        assert(fgetwc(f) == L'H');
        assert(fgetwc(f) == L'E');

        assert(fclose(f) == 0);
        remove(fn);
    }

    /* [3] 保证一个宽字符推回：即使紧跟格式化宽字符输入函数之后。 */
    {
        const char *fn = make_wfile("t_ungetwc_4.tmp", L"123 456");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        int v = 0;
        assert(fwscanf(f, L"%d", &v) == 1);
        assert(v == 123);

        /* 紧跟 fwscanf 之后，仍保证一个宽字符推回 */
        wint_t sp = fgetwc(f);          /* 读空格 */
        assert(sp == L' ');
        assert(ungetwc(sp, f) == sp);   /* 保证成功 */
        assert(fgetwc(f) == L' ');      /* 读回空格 */

        assert(fclose(f) == 0);
        remove(fn);
    }

    /* [4] c == WEOF 时操作失败，返回 WEOF，且流不变。 */
    {
        const char *fn = make_wfile("t_ungetwc_5.tmp", L"Z");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        wint_t z = fgetwc(f);           /* 'Z' */
        assert(z == L'Z');

        /* 推回 WEOF 应失败 */
        wint_t r = ungetwc(WEOF, f);
        assert(r == WEOF);              /* [6] 失败返回 WEOF */

        /* 流不变：下一个读取仍是 'Z'（WEOF 未被推入） */
        assert(fgetwc(f) == L'Z');

        assert(fclose(f) == 0);
        remove(fn);
    }

    /* [5] 成功调用清除 EOF 指示器。 */
    {
        const char *fn = make_wfile("t_ungetwc_6.tmp", L"Q");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        wint_t q = fgetwc(f);           /* 'Q' */
        assert(q == L'Q');

        /* 读到文件末尾，设置 EOF 指示器 */
        wint_t e = fgetwc(f);
        assert(e == WEOF);
        assert(feof(f) != 0);           /* EOF 指示器已设置 */

        /* 推回一个宽字符，成功调用应清除 EOF 指示器 */
        assert(ungetwc(q, f) == q);
        assert(feof(f) == 0);           /* [5] EOF 指示器被清除 */

        /* 读回推回的字符 */
        assert(fgetwc(f) == L'Q');

        assert(fclose(f) == 0);
        remove(fn);
    }

    /* [5] 读取/丢弃所有推回字符后，文件位置指示器与推回前相同。 */
    {
        const char *fn = make_wfile("t_ungetwc_7.tmp", L"ABCDE");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        /* 前进到位置 2（读两个字符） */
        assert(fgetwc(f) == L'A');
        assert(fgetwc(f) == L'B');
        long pos_before = ftell(f);
        assert(pos_before >= 0);

        /* 推回一个字符 */
        assert(ungetwc(L'B', f) == L'B');

        /* 读回推回的字符，位置指示器应恢复到推回前 */
        assert(fgetwc(f) == L'B');
        long pos_after = ftell(f);
        assert(pos_after == pos_before);    /* [5] 位置指示器相同 */

        assert(fclose(f) == 0);
        remove(fn);
    }

    /* [3] 过多推回可能失败：至少保证一个，尝试多个时允许失败。
     *     这里只验证「至少一个成功」，不把「多个必失败」当作约束。 */
    {
        const char *fn = make_wfile("t_ungetwc_8.tmp", L"M");
        FILE *f = fopen(fn, "rb");
        assert(f != NULL);

        wint_t m = fgetwc(f);
        assert(m == L'M');

        /* 第一个推回必须成功（保证一个） */
        assert(ungetwc(m, f) == m);

        /* 第二个推回可能成功也可能失败，两种结果都合法，不做断言 */
        (void)ungetwc(m, f);

        assert(fclose(f) == 0);
        remove(fn);
    }

    printf("All positive tests for C99 7.24.3.10 ungetwc passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「ungetwc 的第一个实参类型为 wint_t」：
     * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
    {
        struct S { int x; } s;
        FILE *f = 0;
        ungetwc(s, f);              /* 错误：实参类型不匹配 */
    }

    /* 违反约束「ungetwc 的第二个实参类型为 FILE *」：
     * 传入 int，gcc -std=c99 应报错（指针与整数不兼容）。 */
    {
        ungetwc(L'A', 42);          /* 错误：第二个实参应为 FILE* */
    }

    /* 违反约束「ungetwc 需要两个实参」：
     * 实参个数不足，gcc -std=c99 应报错。 */
    {
        FILE *f = 0;
        ungetwc(L'A');              /* 错误：实参个数太少 */
    }

    /* 违反约束「ungetwc 需要两个实参」：
     * 实参个数过多，gcc -std=c99 应报错。 */
    {
        FILE *f = 0;
        ungetwc(L'A', f, f);        /* 错误：实参个数太多 */
    }

    /* 违反约束「ungetwc 返回 wint_t，不能作为左值赋值」：
     * 函数调用结果不是左值，gcc -std=c99 应报错。 */
    {
        FILE *f = 0;
        ungetwc(L'A', f) = L'B';    /* 错误：对非左值赋值 */
    }

#endif

    return 0;
}