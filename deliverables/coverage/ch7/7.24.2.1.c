/*
 * 测试条款：C99 7.24.2.1 The fwprintf function
 *
 * 预期行为：
 *   正向测试：以下使用 fwprintf 的代码应能编译并运行通过，
 *             输出结果与预期字符串一致（用 assert 校验）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应被编译器拒绝。
 *
 * 说明：fwprintf 是宽字符版本的 fprintf，写入宽字符流。
 *       本测试用 tmpfile() 创建临时文件，用 fwprintf 写入，
 *       再用 fgetwc/fread 读回校验。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* 辅助：把宽字符流内容读回为宽字符串 */
static size_t read_back(FILE *fp, wchar_t *buf, size_t n)
{
    size_t i = 0;
    int c;
    rewind(fp);
    while (i + 1 < n && (c = fgetwc(fp)) != WEOF) {
        buf[i++] = (wchar_t)c;
    }
    buf[i] = L'\0';
    return i;
}

int main(void)
{
    /* 使用 C locale 以便宽字符与字节一致，便于断言 */
    setlocale(LC_ALL, "C");

    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    FILE *fp;
    wchar_t buf[256];

    /* [1] 原型：int fwprintf(FILE * restrict stream,
     *                       const wchar_t * restrict format, ...);
     * 验证返回值为写入的宽字符数。 */
    fp = tmpfile();
    assert(fp != NULL);
    {
        int n = fwprintf(fp, L"hello");
        assert(n == 5);                 /* [2] 返回写入的宽字符数 */
        read_back(fp, buf, 256);
        assert(wcscmp(buf, L"hello") == 0);
    }
    fclose(fp);

    /* [3] 普通宽字符（非 %）原样复制到输出流 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"abc%%def");          /* %% 输出一个 % */
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"abc%def") == 0);
    fclose(fp);

    /* [4] 转换说明：%d 十进制整数 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%d", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"42") == 0);
    fclose(fp);

    /* [4] 最小字段宽度：右对齐（默认），空格填充 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%5d]", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[   42]") == 0);
    fclose(fp);

    /* [4] 精度：d 转换的最小位数 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%.5d", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"00042") == 0);
    fclose(fp);

    /* [4] 精度：f 转换小数点后的位数 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%.2f", 3.14159);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"3.14") == 0);
    fclose(fp);

    /* [4] 精度：s 转换的最大宽字符数 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%.3s", L"abcdef");
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"abc") == 0);
    fclose(fp);

    /* [5] 星号字段宽度：int 参数提供宽度，出现在被转换参数之前 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%*d]", 6, 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[    42]") == 0);
    fclose(fp);

    /* [5] 星号精度 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%.*d", 5, 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"00042") == 0);
    fclose(fp);

    /* [5] 负字段宽度 = - 标志 + 正宽度（左对齐） */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%*d]", -6, 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[42    ]") == 0);
    fclose(fp);

    /* [5] 负精度 = 精度被省略 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%.*d", -1, 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"42") == 0);
    fclose(fp);

    /* [6] '-' 标志：左对齐 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%-5d]", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[42   ]") == 0);
    fclose(fp);

    /* [6] '+' 标志：有符号转换总是带符号 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%+d %+d", 42, -42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"+42 -42") == 0);
    fclose(fp);

    /* [6] ' '（空格）标志：非负时前缀空格 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[% d][% d]", 42, -42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[ 42][-42]") == 0);
    fclose(fp);

    /* [6] ' ' 与 '+' 同时出现：空格标志被忽略 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%+ d]", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[+42]") == 0);
    fclose(fp);

    /* [6] '#' 标志：o 转换强制首位为 0 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%#o", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"052") == 0);   /* 42 八进制 = 52 */
    fclose(fp);

    /* [6] '#' 标志：x 转换非零结果加 0x 前缀 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%#x", 255);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"0xff") == 0);
    fclose(fp);

    /* [6] '#' 标志：f 转换总是含小数点 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%#.0f", 3.0);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"3.") == 0);
    fclose(fp);

    /* [6] '0' 标志：用前导零填充到字段宽度 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%05d]", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[00042]") == 0);
    fclose(fp);

    /* [6] '0' 与 '-' 同时出现：0 标志被忽略 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%0-5d]", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[42   ]") == 0);
    fclose(fp);

    /* [6] '0' 与精度同时出现（d 转换）：0 标志被忽略 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"[%05.3d]", 42);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"[  042]") == 0);
    fclose(fp);

    /* [7] 长度修饰符 hh：signed char */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%hhd", (int)(signed char)-1);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"-1") == 0);
    fclose(fp);

    /* [7] 长度修饰符 h：short int */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%hd", (int)(short)-1234);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"-1234") == 0);
    fclose(fp);

    /* [7] 长度修饰符 l：long int */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%ld", 123456789L);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"123456789") == 0);
    fclose(fp);

    /* [7] 长度修饰符 ll：long long int */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%lld", 1234567890123LL);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"1234567890123") == 0);
    fclose(fp);

    /* [7] 长度修饰符 l 对 s 转换：指向 wchar_t 的指针 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%ls", L"wide");
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"wide") == 0);
    fclose(fp);

    /* [7] 长度修饰符 l 对 c 转换：wint_t 参数 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%lc", (wint_t)L'A');
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"A") == 0);
    fclose(fp);

    /* [2] 格式耗尽而参数多余：多余参数被求值但被忽略 */
    fp = tmpfile();
    assert(fp != NULL);
    {
        int side = 0;
        fwprintf(fp, L"x", (side = 1, 0));  /* 多余参数被求值 */
        assert(side == 1);                  /* 确实被求值 */
        read_back(fp, buf, 256);
        assert(wcscmp(buf, L"x") == 0);
    }
    fclose(fp);

    /* [2] 遇到格式串结尾即返回 */
    fp = tmpfile();
    assert(fp != NULL);
    {
        int n = fwprintf(fp, L"");
        assert(n == 0);
    }
    fclose(fp);

    /* [4] 精度形式：仅一个句点，精度取 0 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%.d", 0);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"") == 0);      /* 精度 0 且值为 0，输出空 */
    fclose(fp);

    /* [6] '#' 标志：o 转换，值与精度均为 0 时打印单个 0 */
    fp = tmpfile();
    assert(fp != NULL);
    fwprintf(fp, L"%#.0o", 0);
    read_back(fp, buf, 256);
    assert(wcscmp(buf, L"0") == 0);
    fclose(fp);

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「fwprintf 的第一个参数必须是 FILE *」：
     * 传入 int，gcc -std=c99 应报 incompatible type 错误。 */
    fwprintf(42, L"%d", 1);

    /* 违反约束「fwprintf 的第二个参数必须是 const wchar_t *」：
     * 传入窄字符串字面量（char *），gcc -std=c99 应报 incompatible type 错误。 */
    fwprintf(stdout, "hello");

    /* 违反约束「fwprintf 的第二个参数必须是 const wchar_t *」：
     * 传入 int，gcc -std=c99 应报 incompatible type 错误。 */
    fwprintf(stdout, 123);

    /* 违反约束「fwprintf 的返回类型为 int，不能作为左值赋值」：
     * 函数调用结果不是左值，赋值应报错。 */
    fwprintf(stdout, L"x") = 5;

    /* 违反约束「restrict 限定：stream 与 format 均被 restrict 限定」：
     * 这里演示对 restrict 指针的非法使用——把 restrict 指针赋给非 restrict
     * 指针本身合法，但下面这种对 const 限定对象的写入违反约束。 */
    {
        const wchar_t *fmt = L"%d";
        *fmt = L'x';        /* 写入 const 限定对象，应报错 */
    }

#endif

    return 0;
}