/*
 * 测试 C99 7.20.8.1 —— mbstowcs 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（放在 #if 0 中，不参与编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdlib.h>
 *   [2] 转换语义：从初始移位状态开始、遇空字符停止、等价于 mbtowc 但不影响其转换状态
 *   [3] 最多修改 n 个元素；重叠时行为未定义（UB，不作负向测试）
 *   [4] 返回值：非法多字节字符返回 (size_t)(-1)；否则返回修改的元素个数（不含结尾空宽字符）
 *   Footnote 267：若返回值为 n，则数组不会被空宽字符结尾
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型一致 */
static size_t (*fp_mbstowcs)(wchar_t * restrict, const char * restrict, size_t) = mbstowcs;

int main(void)
{
    /* 设置一个确定的多字节编码环境（C 语言环境即可，ASCII 单字节） */
    setlocale(LC_ALL, "C");

    /* [1] 原型可用性 */
    assert(fp_mbstowcs == mbstowcs);

    /* [2][4] 基本转换：ASCII 字符串 -> 宽字符，返回修改的元素个数（不含结尾空宽字符） */
    {
        const char *s = "Hello";
        wchar_t buf[16];
        size_t r;
        memset(buf, 0xAA, sizeof buf);
        r = mbstowcs(buf, s, 16);
        assert(r == 5);                 /* [4] 返回 5，不含结尾空宽字符 */
        assert(buf[0] == L'H');
        assert(buf[1] == L'e');
        assert(buf[2] == L'l');
        assert(buf[3] == L'l');
        assert(buf[4] == L'o');
        assert(buf[5] == L'\0');        /* 结尾空宽字符被写入 */
    }

    /* [2] 遇空字符停止：空字符之后的多字节字符不被检查/转换 */
    {
        const char s[] = { 'A', 'B', '\0', 'C', 'D', '\0' };
        wchar_t buf[16];
        size_t r;
        memset(buf, 0xAA, sizeof buf);
        r = mbstowcs(buf, s, 16);
        assert(r == 2);                 /* 只转换 'A','B' */
        assert(buf[0] == L'A');
        assert(buf[1] == L'B');
        assert(buf[2] == L'\0');
        /* 'C','D' 之后的内容不应被写入 */
        assert(buf[3] == (wchar_t)0xAAAA);
    }

    /* [3] 最多修改 n 个元素：n 小于字符串长度时，只写 n 个，且不写结尾空宽字符 */
    {
        const char *s = "Hello";
        wchar_t buf[16];
        size_t r;
        memset(buf, 0xAA, sizeof buf);
        r = mbstowcs(buf, s, 3);        /* n = 3 */
        assert(r == 3);                 /* [4] 返回修改的元素个数 = 3 */
        assert(buf[0] == L'H');
        assert(buf[1] == L'e');
        assert(buf[2] == L'l');
        /* Footnote 267：返回值为 n 时数组不会被空宽字符结尾 */
        assert(buf[3] == (wchar_t)0xAAAA);
    }

    /* [3] n == 0：不修改任何元素，返回 0 */
    {
        const char *s = "Hello";
        wchar_t buf[4];
        size_t r;
        memset(buf, 0xAA, sizeof buf);
        r = mbstowcs(buf, s, 0);
        assert(r == 0);
        assert(buf[0] == (wchar_t)0xAAAA);
        assert(buf[1] == (wchar_t)0xAAAA);
        assert(buf[2] == (wchar_t)0xAAAA);
        assert(buf[3] == (wchar_t)0xAAAA);
    }

    /* [2][4] 空字符串：转换结果为空，返回 0，写入一个空宽字符 */
    {
        const char *s = "";
        wchar_t buf[4];
        size_t r;
        memset(buf, 0xAA, sizeof buf);
        r = mbstowcs(buf, s, 4);
        assert(r == 0);
        assert(buf[0] == L'\0');
        assert(buf[1] == (wchar_t)0xAAAA);
    }

    /* [2] 等价于 mbtowc 但不影响 mbtowc 的转换状态：
     * 在 C 语言环境下，mbtowc 的转换状态始终为初始状态，
     * 这里验证 mbstowcs 调用后 mbtowc 仍能正常工作。 */
    {
        const char *s = "AB";
        wchar_t buf[4];
        wchar_t wc;
        int st;
        size_t r;
        r = mbstowcs(buf, s, 4);
        assert(r == 2);
        /* mbstowcs 不应影响 mbtowc 的转换状态 */
        st = mbtowc(&wc, "Z", 1);
        assert(st == 1);
        assert(wc == L'Z');
    }

    /* [4] 非法多字节字符：返回 (size_t)(-1)
     * 在 C 语言环境下，字节 0xFF 不是合法的多字节字符。 */
    {
        const char s[] = { 'A', (char)0xFF, 'B', '\0' };
        wchar_t buf[8];
        size_t r;
        memset(buf, 0xAA, sizeof buf);
        r = mbstowcs(buf, s, 8);
        assert(r == (size_t)(-1));      /* [4] 非法多字节字符 */
        assert(buf[0] == L'A');         /* 之前的合法字符已被转换 */
    }

    /* [4] 非法多字节字符出现在开头 */
    {
        const char s[] = { (char)0xFF, 'A', '\0' };
        wchar_t buf[8];
        size_t r;
        r = mbstowcs(buf, s, 8);
        assert(r == (size_t)(-1));
    }

    /* [3] 重叠时行为未定义（UB），此处不作测试，仅注释说明 */

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「mbstowcs 的第一个参数类型为 wchar_t * restrict」：
 * 传入 char * 而非 wchar_t *，gcc -std=c99 应报 incompatible pointer type 错误。 */
{
    char cbuf[16];
    const char *s = "x";
    mbstowcs(cbuf, s, 16);   /* 错误：第一个实参应为 wchar_t * */
}

/* 违反约束「mbstowcs 的第二个参数类型为 const char * restrict」：
 * 传入 wchar_t * 而非 const char *，应报 incompatible pointer type 错误。 */
{
    wchar_t wbuf[16];
    const wchar_t *ws = L"x";
    mbstowcs(wbuf, ws, 16);  /* 错误：第二个实参应为 const char * */
}

/* 违反约束「mbstowcs 的第三个参数类型为 size_t」：
 * 传入指针而非整数，应报 incompatible type 错误。 */
{
    wchar_t wbuf[16];
    const char *s = "x";
    mbstowcs(wbuf, s, s);    /* 错误：第三个实参应为 size_t */
}

/* 违反约束「mbstowcs 需要 3 个实参」：
 * 实参个数不足，应报 too few arguments 错误。 */
{
    wchar_t wbuf[16];
    mbstowcs(wbuf);          /* 错误：缺少实参 */
}

/* 违反约束「mbstowcs 需要 3 个实参」：
 * 实参个数过多，应报 too many arguments 错误。 */
{
    wchar_t wbuf[16];
    const char *s = "x";
    mbstowcs(wbuf, s, 16, 0); /* 错误：实参过多 */
}

/* 违反约束「mbstowcs 的返回类型为 size_t，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报 lvalue required 错误。 */
{
    wchar_t wbuf[16];
    const char *s = "x";
    mbstowcs(wbuf, s, 16) = 0; /* 错误：非左值赋值 */
}

/* 违反约束「mbstowcs 声明于 <stdlib.h>」：
 * 若未包含 <stdlib.h>，则 mbstowcs 未声明，C99 下调用未声明函数为约束违反，
 * 应报 implicit declaration 错误（此处通过不包含头文件来演示）。 */
/* 注意：本文件顶部已包含 <stdlib.h>，此片段仅作说明，实际编译需单独文件。 */

#endif /* 负向测试结束 */