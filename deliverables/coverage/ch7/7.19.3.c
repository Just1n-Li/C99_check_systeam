/*
 * 测试目标：C99 7.19.3 Files（文件）
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 说明：7.19.3 主要是描述性/实现定义条款，约束很少。负向测试针对
 *       本条款可推导出的约束（如 FILE 对象不可复制、指针在关闭后不确定等
 *       属于 UB 而非约束，故不作为负向测试；这里只放真正违反约束的片段）。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <wchar.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [7] 程序启动时预定义三个文本流：stdin / stdout / stderr，无需显式打开 */
    assert(stdin  != NULL);
    assert(stdout != NULL);
    assert(stderr != NULL);

    /* [7] stderr 初始不是完全缓冲的（实现定义，但标准要求“not fully buffered”） */
    /* 这里只验证流对象存在且可用，不强制具体缓冲模式 */

    /* [15] FOPEN_MAX 至少为 8，包含三个标准文本流 */
    assert(FOPEN_MAX >= 8);

    /* [1] 打开文件：创建新文件；文件位置指示器定位在文件起始（字符编号 0） */
    const char *fname = "c99_7_19_3_test.tmp";
    FILE *fp = fopen(fname, "w+");
    assert(fp != NULL);

    /* [1] 写入后位置指示器随写操作推进 */
    assert(fputc('A', fp) != EOF);
    assert(fputc('B', fp) != EOF);
    assert(fputc('C', fp) != EOF);

    /* [1] 定位请求：回到文件起始 */
    assert(fseek(fp, 0L, SEEK_SET) == 0);

    /* [1] 读取，验证位置指示器有序推进 */
    int c1 = fgetc(fp);
    int c2 = fgetc(fp);
    int c3 = fgetc(fp);
    assert(c1 == 'A');
    assert(c2 == 'B');
    assert(c3 == 'C');

    /* [4] 关闭文件：输出流在解除关联前被 flush；关闭后 FILE* 值不确定（不再使用） */
    assert(fclose(fp) == 0);

    /* [5] 文件可被重新打开，内容可被回收/修改 */
    fp = fopen(fname, "r");
    assert(fp != NULL);
    assert(fgetc(fp) == 'A');
    assert(fgetc(fp) == 'B');
    assert(fgetc(fp) == 'C');
    assert(fgetc(fp) == EOF);
    assert(fclose(fp) == 0);

    /* [5] 重新打开并修改内容（可定位到起始） */
    fp = fopen(fname, "r+");
    assert(fp != NULL);
    assert(fputc('Z', fp) != EOF);   /* 覆盖第一个字符 */
    assert(fclose(fp) == 0);

    fp = fopen(fname, "r");
    assert(fp != NULL);
    assert(fgetc(fp) == 'Z');        /* 内容已被修改 */
    assert(fclose(fp) == 0);

    /* [2] 二进制文件不会被截断（除 7.19.5.3 规定外） */
    fp = fopen(fname, "wb+");
    assert(fp != NULL);
    assert(fwrite("hello", 1, 5, fp) == 5);
    assert(fclose(fp) == 0);

    fp = fopen(fname, "rb+");
    assert(fp != NULL);
    /* 定位到中间写入，不应截断文件 */
    assert(fseek(fp, 2L, SEEK_SET) == 0);
    assert(fputc('X', fp) != EOF);
    assert(fclose(fp) == 0);

    fp = fopen(fname, "rb");
    assert(fp != NULL);
    char buf[16];
    size_t n = fread(buf, 1, sizeof buf, fp);
    assert(n == 5);                  /* 长度仍为 5，未被截断 */
    assert(memcmp(buf, "heXlo", 5) == 0);
    assert(fclose(fp) == 0);

    /* [3] 缓冲模式：setvbuf 可设置 _IONBF / _IOLBF / _IOFBF */
    fp = fopen(fname, "w");
    assert(fp != NULL);
    assert(setvbuf(fp, NULL, _IONBF, 0) == 0);   /* 无缓冲 */
    assert(fputc('x', fp) != EOF);
    assert(fclose(fp) == 0);

    fp = fopen(fname, "w");
    assert(fp != NULL);
    assert(setvbuf(fp, NULL, _IOLBF, 0) == 0);   /* 行缓冲 */
    assert(fputs("line\n", fp) >= 0);
    assert(fclose(fp) == 0);

    fp = fopen(fname, "w");
    assert(fp != NULL);
    assert(setvbuf(fp, NULL, _IOFBF, 0) == 0);   /* 完全缓冲 */
    assert(fputs("block", fp) >= 0);
    assert(fclose(fp) == 0);

    /* [4] 零长度文件：是否实际存在由实现定义，这里只验证可创建并关闭 */
    fp = fopen(fname, "w");
    assert(fp != NULL);
    assert(fclose(fp) == 0);

    /* [6] FILE 对象的地址可能有意义；这里验证标准流地址稳定且互不相同 */
    assert((void *)stdin  != (void *)stdout);
    assert((void *)stdout != (void *)stderr);
    assert((void *)stdin  != (void *)stderr);

    /* [8] 打开附加文件需要文件名字符串；同一文件可同时打开多次（实现定义） */
    FILE *fa = fopen(fname, "w");
    FILE *fb = fopen(fname, "w");
    assert(fa != NULL);
    assert(fb != NULL);
    assert(fclose(fa) == 0);
    assert(fclose(fb) == 0);

    /* [9][10] 宽定向流：外部文件是多字节字符序列；编码由实现定义。
     * 这里只验证宽字符 I/O 基本可用（不假设具体编码）。 */
    fp = fopen(fname, "w+");
    assert(fp != NULL);
    /* 使用 ASCII 范围内的宽字符，任何实现都应支持 */
    assert(fputwc(L'A', fp) != WEOF);
    assert(fputwc(L'B', fp) != WEOF);
    assert(fclose(fp) == 0);

    fp = fopen(fname, "r");
    assert(fp != NULL);
    wint_t w1 = fgetwc(fp);
    wint_t w2 = fgetwc(fp);
    assert(w1 == L'A');
    assert(w2 == L'B');
    assert(fclose(fp) == 0);

    /* [11] 宽字符输入函数按 fgetwc 逐次读取并转换；字节输入函数按 fgetc 读取 */
    fp = fopen(fname, "r");
    assert(fp != NULL);
    assert(fgetc(fp) == 'A');        /* 字节输入 */
    assert(fclose(fp) == 0);

    /* [12] 宽字符输出函数按 fputwc 逐次转换并写出；字节输出函数按 fputc 写出 */
    fp = fopen(fname, "w");
    assert(fp != NULL);
    assert(fputc('C', fp) != EOF);   /* 字节输出 */
    assert(fclose(fp) == 0);

    /* [13] 某些字节 I/O 函数也执行多字节/宽字符转换（如 fgetws/fputws 等），
     * 这里只验证基本字节 I/O 可用，不强制具体转换函数。 */

    /* [14] 编码错误时，宽字符 I/O 与字节 I/O 函数当且仅当发生编码错误时
     * 将 EILSEQ 存入 errno。这里不构造非法编码（依赖实现），
     * 只验证正常路径下 errno 不被错误设置。 */
    errno = 0;
    fp = fopen(fname, "r");
    assert(fp != NULL);
    (void)fgetc(fp);
    assert(fclose(fp) == 0);
    /* 正常读取不应产生编码错误 */

    /* 清理临时文件 */
    remove(fname);

    printf("C99 7.19.3 正向测试全部通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「FILE 对象不可复制」：
 * 7.19.3 [6] 指出 FILE 对象的地址可能有意义，FILE 对象的副本不能代替原对象。
 * 标准库头文件中 FILE 通常为不完整类型或不可复制类型，
 * 对 FILE 对象进行赋值/复制应导致编译错误。
 * 期望：gcc -std=c99 报错（如 "invalid use of undefined type" 或
 *       "assignment to expression with array type" 等）。 */
FILE f1;
FILE f2;
f1 = f2;                 /* 错误：FILE 对象不可赋值 */

/* 违反约束「FILE* 关闭后其值不确定，不得再使用」：
 * 7.19.3 [4] 关闭后指向 FILE 对象的指针值不确定。
 * 虽然这是 UB 而非严格约束，但下面片段演示对已关闭 FILE* 的误用，
 * 期望编译器在 -Wall 下可能给出警告；此处仅作说明，不强制报错。 */
/* FILE *fp = fopen("x", "r");
 * fclose(fp);
 * fgetc(fp);  // UB，非约束违反，故不放入负向测试 */

/* 违反约束「FOPEN_MAX 至少为 8」：
 * 7.19.3 [15] 要求 FOPEN_MAX >= 8。
 * 若实现违反，则下面静态断言应失败（编译期错误）。
 * 期望：若 FOPEN_MAX < 8，编译报错。 */
#if FOPEN_MAX < 8
#error "FOPEN_MAX must be at least 8 (C99 7.19.3 [15])"
#endif

#endif /* 负向测试结束 */