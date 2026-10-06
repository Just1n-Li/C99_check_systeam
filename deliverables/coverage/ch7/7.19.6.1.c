/*
 * 测试目标：C99 7.19.6.1 The fprintf function
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过，assert 全部成立。
 *   负向测试：违反约束的片段应导致编译报错（放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：[1] 原型、[2] 描述/返回值、[3] 格式串组成、[4] 转换规格组成、
 *           [5] * 宽度/精度、[6] 标志字符、[7] 长度修饰符。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <wchar.h>
#include <limits.h>

/* 辅助：把 fprintf 输出写入内存缓冲区，便于断言 */
static char buf[512];

static void reset(void) { memset(buf, 0, sizeof buf); }

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型：fprintf 返回 int，接受 FILE* restrict, const char* restrict, ... */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int n = fprintf(fp, "hello");
        assert(n == 5);                 /* [2] 返回写入的字符数 */
        fclose(fp);
    }

    /* [2] 格式串耗尽后多余实参被求值但被忽略；返回值为写入字符数 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int n = fprintf(fp, "ab", 1, 2, 3);   /* 多余实参被忽略 */
        assert(n == 2);
        fclose(fp);
    }

    /* [3] 普通多字节字符原样拷贝；% 引入转换规格 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int n = fprintf(fp, "A%%B");    /* %% 输出一个 % */
        assert(n == 3);
        fclose(fp);
    }

    /* [4] 最小字段宽度：默认右对齐，左侧补空格 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%5d]", 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[   42]") == 0);
        fclose(fp);
    }

    /* [4] 精度：d 的最小位数 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%.5d", 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "00042") == 0);
        fclose(fp);
    }

    /* [4] 精度：s 的最大字节数 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%.3s", "abcdef");
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "abc") == 0);
        fclose(fp);
    }

    /* [4] 只有句点：精度取 0 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%.d", 0);          /* 精度 0，值为 0 → 无数字输出 */
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "") == 0);
        fclose(fp);
    }

    /* [5] * 提供字段宽度；负宽度等价于 - 标志 + 正宽度 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%*d]", 5, 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[   42]") == 0);
        fclose(fp);
    }
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%*d]", -5, 42);   /* 负宽度 → 左对齐 */
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[42   ]") == 0);
        fclose(fp);
    }

    /* [5] * 提供精度；负精度等价于省略精度 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%.*d", 5, 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "00042") == 0);
        fclose(fp);
    }
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%.*d", -1, 42);    /* 负精度 → 省略精度 */
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "42") == 0);
        fclose(fp);
    }

    /* [5] 宽度与精度同时用 *，宽度实参先于精度实参，再是被转换实参 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%*.*d]", 8, 5, 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[   00042]") == 0);
        fclose(fp);
    }

    /* [6] '-' 左对齐 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%-5d]", 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[42   ]") == 0);
        fclose(fp);
    }

    /* [6] '+' 有符号转换总是带符号 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%+d %+d", 42, -42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "+42 -42") == 0);
        fclose(fp);
    }

    /* [6] ' ' 空格标志：非负时前缀空格；与 + 同时出现时空格被忽略 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[% d][% d]", 42, -42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[ 42][-42]") == 0);
        fclose(fp);
    }
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%+ d]", 42);      /* 空格被忽略 */
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[+42]") == 0);
        fclose(fp);
    }

    /* [6] '#' 替代形式：o 强制首位 0 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%#o", 8);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "010") == 0);
        fclose(fp);
    }
    /* [6] '#' 替代形式：x 非零结果前缀 0x */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%#x", 255);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "0xff") == 0);
        fclose(fp);
    }
    /* [6] '#' 替代形式：浮点总是含小数点 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%#.0f", 1.0);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "1.") == 0);
        fclose(fp);
    }
    /* [6] '#' 替代形式：g 不删除尾随零 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%#g", 1.0);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "1.00000") == 0);
        fclose(fp);
    }

    /* [6] '0' 前导零填充 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%05d]", 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[00042]") == 0);
        fclose(fp);
    }
    /* [6] '0' 与 '-' 同时出现时 '0' 被忽略 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%0-5d]", 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[42   ]") == 0);
        fclose(fp);
    }
    /* [6] '0' 与精度同时出现时 '0' 被忽略 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "[%05.3d]", 42);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "[  042]") == 0);
        fclose(fp);
    }

    /* [7] hh：signed char / unsigned char */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        signed char sc = -1;
        unsigned char uc = 200;
        fprintf(fp, "%hhd %hhu", sc, uc);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "-1 200") == 0);
        fclose(fp);
    }

    /* [7] h：short int / unsigned short int */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        short s = -1234;
        unsigned short us = 60000;
        fprintf(fp, "%hd %hu", s, us);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "-1234 60000") == 0);
        fclose(fp);
    }

    /* [7] l：long int / unsigned long int */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        long l = -123456789L;
        unsigned long ul = 4000000000UL;
        fprintf(fp, "%ld %lu", l, ul);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "-123456789 4000000000") == 0);
        fclose(fp);
    }

    /* [7] l 对浮点无影响 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fprintf(fp, "%.2f %.2lf", 1.5, 1.5);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "1.50 1.50") == 0);
        fclose(fp);
    }

    /* [7] ll：long long int / unsigned long long int */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        long long ll = -1234567890123LL;
        unsigned long long ull = 12345678901234567890ULL;
        fprintf(fp, "%lld %llu", ll, ull);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "-1234567890123 12345678901234567890") == 0);
        fclose(fp);
    }

    /* [7] l 与 c：wint_t 实参 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        wint_t wc = L'A';
        fprintf(fp, "%lc", wc);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "A") == 0);
        fclose(fp);
    }

    /* [7] l 与 s：wchar_t* 实参 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        wchar_t ws[] = L"hi";
        fprintf(fp, "%ls", ws);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "hi") == 0);
        fclose(fp);
    }

    /* [7] n：写入已写字符数 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int count = -1;
        fprintf(fp, "abc%n", &count);
        assert(count == 3);
        fclose(fp);
    }

    /* [7] hh 与 n：指向 signed char 的指针 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        signed char sc = -1;
        fprintf(fp, "ab%hhn", &sc);
        assert(sc == 2);
        fclose(fp);
    }

    /* [7] h 与 n：指向 short int 的指针 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        short s = -1;
        fprintf(fp, "abcd%hn", &s);
        assert(s == 4);
        fclose(fp);
    }

    /* [7] l 与 n：指向 long int 的指针 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        long l = -1;
        fprintf(fp, "abcde%ln", &l);
        assert(l == 5);
        fclose(fp);
    }

    /* [7] ll 与 n：指向 long long int 的指针 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        long long ll = -1;
        fprintf(fp, "abcdef%lln", &ll);
        assert(ll == 6);
        fclose(fp);
    }

    /* [7] j：intmax_t / uintmax_t */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        intmax_t im = -1;
        uintmax_t um = 1;
        fprintf(fp, "%jd %ju", im, um);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "-1 1") == 0);
        fclose(fp);
    }

    /* [7] z：size_t / 有符号对应类型 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        size_t sz = 42;
        fprintf(fp, "%zu", sz);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "42") == 0);
        fclose(fp);
    }

    /* [7] t：ptrdiff_t */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        ptrdiff_t pd = -7;
        fprintf(fp, "%td", pd);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "-7") == 0);
        fclose(fp);
    }

    /* [7] L：long double */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        long double ld = 1.5L;
        fprintf(fp, "%.2Lf", ld);
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "1.50") == 0);
        fclose(fp);
    }

    /* [2] 无原型函数调用中的默认实参提升：char→int, float→double
     * 这里通过 fprintf 的 ... 参数验证：传入 char/float 会被提升，
     * 用 %d/%f 读取应得到正确值。 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        char c = 'A';
        float f = 1.5f;
        fprintf(fp, "%d %.1f", c, f);   /* c 提升为 int，f 提升为 double */
        fflush(fp);
        rewind(fp);
        reset();
        fread(buf, 1, sizeof buf - 1, fp);
        assert(strcmp(buf, "65 1.5") == 0);
        fclose(fp);
    }

    /* [2] 省略号后停止转换：多余实参被求值但被忽略 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int side = 0;
        int n = fprintf(fp, "x", (side = 1, 0));  /* 多余实参被求值 */
        assert(n == 1);
        assert(side == 1);
        fclose(fp);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fprintf 第一个参数必须是 FILE*」：传入 int，gcc -std=c99 应报错 */
void neg1(void) {
    int x = 0;
    fprintf(x, "hello");
}

/* 违反约束「fprintf 第二个参数必须是 const char*」：传入 int，应报错 */
void neg2(void) {
    FILE *fp = tmpfile();
    fprintf(fp, 42);
}

/* 违反约束「fprintf 至少需要两个参数」：只传一个参数，应报错 */
void neg3(void) {
    FILE *fp = tmpfile();
    fprintf(fp);
}

/* 违反约束「fprintf 需要 FILE* 实参」：不传参数，应报错 */
void neg4(void) {
    fprintf();
}

/* 违反约束「restrict 限定：stream 与 format 不能指向同一对象」——
 * 这里用类型不兼容演示：把 FILE* 赋给 const char* 应报错 */
void neg5(void) {
    FILE *fp = tmpfile();
    const char *p = fp;   /* 类型不兼容，应报错 */
    (void)p;
}

/* 违反约束「format 必须是 const char*」：把 FILE* 作为 format 传入，
 * 类型不兼容，应报错 */
void neg6(void) {
    FILE *fp = tmpfile();
    FILE *fp2 = tmpfile();
    fprintf(fp, fp2);
}

#endif