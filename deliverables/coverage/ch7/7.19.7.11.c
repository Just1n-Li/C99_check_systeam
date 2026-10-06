/*
 * 测试条款：C99 7.19.7.11  The ungetc function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明
 *   [2] 推回字符、逆序返回、文件定位函数丢弃推回字符、外部存储不变
 *   [3] 保证一个字符推回；过多推回可能失败
 *   [4] c == EOF 时失败且流不变
 *   [5] 成功调用清除 EOF 指示器；文件位置指示器语义
 *   [6] 返回值：成功返回推回后的字符，失败返回 EOF
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型：int ungetc(int c, FILE *stream); 头文件 <stdio.h> 中声明 */
static void test_prototype(void)
{
    /* 取函数指针，验证签名与返回类型为 int、参数为 (int, FILE*) */
    int (*fp)(int, FILE *) = ungetc;
    assert(fp != NULL);
}

/* [2] 推回字符，后续读取按推回逆序返回；外部存储不变 */
static void test_pushback_reverse_order(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    /* 写入 "abc" 到临时文件 */
    assert(fputs("abc", f) >= 0);
    rewind(f);

    /* 读一个字符 'a' */
    int c1 = fgetc(f);
    assert(c1 == 'a');

    /* 推回 'X' 再推回 'Y'，后续读取应先得 'Y' 再得 'X'（逆序） */
    assert(ungetc('X', f) == 'X');
    assert(ungetc('Y', f) == 'Y');

    int r1 = fgetc(f);
    int r2 = fgetc(f);
    assert(r1 == 'Y');
    assert(r2 == 'X');

    /* 继续读取应得到原文件内容 'b'、'c'（外部存储未改变） */
    assert(fgetc(f) == 'b');
    assert(fgetc(f) == 'c');
    assert(fgetc(f) == EOF);

    fclose(f);
}

/* [2] 文件定位函数（fseek/rewind）丢弃推回字符 */
static void test_positioning_discards_pushback(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("hello", f) >= 0);
    rewind(f);

    assert(fgetc(f) == 'h');       /* 位置在 'e' 前 */
    assert(ungetc('Z', f) == 'Z'); /* 推回 'Z' */

    /* fseek 丢弃推回字符 */
    assert(fseek(f, 0, SEEK_SET) == 0);
    assert(fgetc(f) == 'h');       /* 回到开头 */

    /* rewind 同样丢弃推回字符 */
    assert(fgetc(f) == 'e');
    assert(ungetc('Q', f) == 'Q');
    rewind(f);
    assert(fgetc(f) == 'h');

    fclose(f);
}

/* [3] 保证一个字符推回 */
static void test_one_char_guaranteed(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("A", f) >= 0);
    rewind(f);

    assert(fgetc(f) == 'A');
    /* 至少保证一次推回成功 */
    int r = ungetc('B', f);
    assert(r == 'B');
    assert(fgetc(f) == 'B');

    fclose(f);
}

/* [4] c == EOF 时操作失败，流不变 */
static void test_eof_pushback_fails(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("xy", f) >= 0);
    rewind(f);

    assert(fgetc(f) == 'x');       /* 位置在 'y' 前 */

    /* 推回 EOF 必须失败，返回 EOF */
    int r = ungetc(EOF, f);
    assert(r == EOF);

    /* 流不变：下一个字符仍是 'y' */
    assert(fgetc(f) == 'y');

    fclose(f);
}

/* [5] 成功调用清除 EOF 指示器 */
static void test_clears_eof_indicator(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("k", f) >= 0);
    rewind(f);

    assert(fgetc(f) == 'k');
    /* 读到文件末尾，设置 EOF 指示器 */
    assert(fgetc(f) == EOF);
    assert(feof(f) != 0);

    /* 成功推回一个字符应清除 EOF 指示器 */
    assert(ungetc('k', f) == 'k');
    assert(feof(f) == 0);

    /* 读回推回的字符 */
    assert(fgetc(f) == 'k');

    fclose(f);
}

/* [5] 二进制流：每次成功 ungetc 使文件位置指示器减一 */
static void test_binary_position_decrement(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("0123456789", f) >= 0);
    rewind(f);

    /* 前进到位置 5 */
    assert(fseek(f, 5, SEEK_SET) == 0);
    long pos_before = ftell(f);
    assert(pos_before == 5);

    /* 成功推回一个字符，位置指示器应减一 */
    assert(ungetc('Z', f) == 'Z');
    long pos_after = ftell(f);
    assert(pos_after == pos_before - 1);

    /* 读回推回字符后位置恢复 */
    assert(fgetc(f) == 'Z');
    assert(ftell(f) == pos_before);

    fclose(f);
}

/* [5] 读取/丢弃所有推回字符后，位置指示器与推回前相同 */
static void test_position_restored_after_consume(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("abcdef", f) >= 0);
    rewind(f);

    assert(fgetc(f) == 'a');
    long pos = ftell(f);           /* 位置在 'b' 前 */

    assert(ungetc('P', f) == 'P');
    assert(ungetc('Q', f) == 'Q');

    /* 读掉所有推回字符 */
    assert(fgetc(f) == 'Q');
    assert(fgetc(f) == 'P');

    /* 位置指示器恢复到推回前 */
    assert(ftell(f) == pos);

    fclose(f);
}

/* [6] 返回值：成功返回推回后的字符（转换为 unsigned char 再转 int） */
static void test_return_value(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("m", f) >= 0);
    rewind(f);
    assert(fgetc(f) == 'm');

    /* 成功：返回推回后的字符 */
    int r = ungetc('A', f);
    assert(r == 'A');

    /* 失败：返回 EOF */
    assert(ungetc(EOF, f) == EOF);

    fclose(f);
}

/* [2] 推回字符被转换为 unsigned char 后存储 */
static void test_conversion_to_unsigned_char(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    assert(fputs("n", f) >= 0);
    rewind(f);
    assert(fgetc(f) == 'n');

    /* 传入 0x1FF，转换为 unsigned char 后为 0xFF */
    int r = ungetc(0x1FF, f);
    assert(r == 0xFF);
    assert(fgetc(f) == 0xFF);

    fclose(f);
}

int main(void)
{
    test_prototype();
    test_pushback_reverse_order();
    test_positioning_discards_pushback();
    test_one_char_guaranteed();
    test_eof_pushback_fails();
    test_clears_eof_indicator();
    test_binary_position_decrement();
    test_position_restored_after_consume();
    test_return_value();
    test_conversion_to_unsigned_char();

    printf("All ungetc (C99 7.19.7.11) positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「ungetc 的第一个参数必须为 int 类型」：
 * 传入结构体类型，gcc -std=c99 应报错（参数类型不兼容）。 */
struct S { int x; } s;
FILE *fp;
ungetc(s, fp);

/* 违反约束「ungetc 的第二个参数必须为 FILE * 类型」：
 * 传入 int，gcc -std=c99 应报错（参数类型不兼容）。 */
ungetc('a', 42);

/* 违反约束「ungetc 的实参个数必须为 2」：
 * 只传一个参数，gcc -std=c99 应报错（实参太少）。 */
ungetc('a');

/* 违反约束「ungetc 的实参个数必须为 2」：
 * 传三个参数，gcc -std=c99 应报错（实参太多）。 */
ungetc('a', fp, 1);

/* 违反约束「ungetc 返回 int，不能作为左值赋值」：
 * 函数调用结果不是左值，gcc -std=c99 应报错。 */
ungetc('a', fp) = 5;

#endif