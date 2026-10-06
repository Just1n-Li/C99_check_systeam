/*
 * 验证 C99 7.19.6 —— 格式化输入/输出函数
 *
 * 条款 [1]：格式化输入/输出函数的行为，应如同在与每个转换说明符相关联的
 *           动作之后存在一个序列点（sequence point）。
 * 脚注 240：fprintf 系列函数对 %n 说明符执行“写入内存”的动作。
 *
 * 预期行为：
 *   正向测试 —— 程序应能编译并运行通过（assert 全部成立）。
 *   负向测试 —— 违反约束的代码应被编译器拒绝（编译报错），
 *               统一放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 说明：序列点本身是“可观察副作用顺序”的语义要求，无法用 assert 直接
 *       断言“存在序列点”，但可以通过“每个说明符的动作（含 %n 写内存）
 *       按序完成、且其副作用在下一个说明符动作前已生效”来间接验证。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 基本语义：printf 的每个说明符动作按序发生，输出结果可预期。
     *     用 snprintf 把结果写入缓冲区，再逐字节比较，验证“按序完成”。 */
    {
        char buf[64];
        int n = snprintf(buf, sizeof buf, "%d-%s-%c", 42, "abc", 'Z');
        /* 返回值 = 本应写入的字符数（不含结尾 '\0'） */
        assert(n == (int)strlen("42-abc-Z"));
        assert(strcmp(buf, "42-abc-Z") == 0);
    }

    /* [1] 序列点语义：%n 的动作是“写入内存”（脚注 240）。
     *     在同一个 printf 调用中，%n 之前的所有说明符动作必须已经完成，
     *     因此 %n 写入的计数必须等于其前面已输出字符的个数。 */
    {
        char buf[64];
        int count = -1;
        int n = snprintf(buf, sizeof buf, "AB%nCD", &count);
        /* 到 %n 为止已输出 "AB"，共 2 个字符 */
        assert(count == 2);
        /* 整个调用共输出 "ABCD"，4 个字符 */
        assert(n == 4);
        assert(strcmp(buf, "ABCD") == 0);
    }

    /* [1] 多个 %n 说明符：每个 %n 的动作在其自身位置之后立即生效，
     *     体现“每个说明符动作之后存在序列点”的按序语义。 */
    {
        char buf[64];
        int c1 = -1, c2 = -1, c3 = -1;
        int n = snprintf(buf, sizeof buf, "%d%n%s%n%c%n", 123, &c1, "xy", &c2, '!', &c3);
        assert(c1 == 3);   /* "123"        -> 3  */
        assert(c2 == 5);   /* "123xy"      -> 5  */
        assert(c3 == 6);   /* "123xy!"     -> 6  */
        assert(n == 6);
        assert(strcmp(buf, "123xy!") == 0);
    }

    /* [1] 格式化输入函数同样适用：scanf 的每个说明符动作按序发生，
     *     用 %n 记录已消费的输入字符数，验证按序语义。 */
    {
        int a = 0, b = 0;
        int p1 = -1, p2 = -1;
        /* 输入 "10 20"：%d 读 10，%n 记位置，%d 读 20，%n 记位置 */
        int r = sscanf("10 20", "%d%n %d%n", &a, &p1, &b, &p2);
        assert(r == 2);
        assert(a == 10);
        assert(b == 20);
        assert(p1 == 2);   /* 读完 "10" 后已消费 2 个字符 */
        assert(p2 == 5);   /* 读完 "10 20" 后已消费 5 个字符 */
    }

    /* [1] 序列点语义的“副作用可见性”：%n 写入的值在后续说明符动作
     *     执行时已经可见（因为每个说明符动作后存在序列点）。 */
    {
        char buf[64];
        int count = -1;
        /* 先输出 "hello"，%n 写入 5，随后继续输出 */
        int n = snprintf(buf, sizeof buf, "hello%n world", &count);
        assert(count == 5);
        assert(n == (int)strlen("hello world"));
        assert(strcmp(buf, "hello world") == 0);
    }

    /* [1] fprintf 到文件流：%n 同样执行“写入内存”的动作（脚注 240），
     *     且其值反映该说明符之前已写入的字符数。 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int count = -1;
        int n = fprintf(fp, "abc%ndef", &count);
        assert(count == 3);
        assert(n == 6);
        fclose(fp);
    }

    /* [1] fscanf 从文件流读取：每个说明符动作按序发生，%n 记录位置。 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fputs("77 88", fp);
        rewind(fp);
        int x = 0, y = 0, q1 = -1, q2 = -1;
        int r = fscanf(fp, "%d%n %d%n", &x, &q1, &y, &q2);
        assert(r == 2);
        assert(x == 77);
        assert(y == 88);
        assert(q1 == 2);
        assert(q2 == 5);
        fclose(fp);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /*
     * 违反约束「%n 对应的实参必须是指向有符号整数的指针」（C99 7.19.6.1/7.19.6.2）。
     * 传入非指针类型，gcc -std=c99 -Wall 应报错（类型不匹配 / 格式警告）。
     */
    {
        int not_a_pointer = 0;
        printf("x%n", not_a_pointer);   /* 期望报错：%n 需要 int* 实参 */
    }

    /*
     * 违反约束「%d 对应的实参必须是整数类型」。
     * 传入 double，gcc -std=c99 应报错（格式与实参类型不匹配）。
     */
    {
        double d = 3.14;
        printf("%d", d);                /* 期望报错：%d 需要 int 实参 */
    }

    /*
     * 违反约束「%s 对应的实参必须是 char* 类型」。
     * 传入 int，gcc -std=c99 应报错。
     */
    {
        int i = 0;
        printf("%s", i);                /* 期望报错：%s 需要 char* 实参 */
    }

    /*
     * 违反约束「格式化函数的格式字符串必须是字符串字面量或指向字符数组的指针」。
     * 传入整数常量作为格式串，gcc -std=c99 应报错。
     */
    {
        printf(123);                    /* 期望报错：格式串类型错误 */
    }

#endif

    return 0;
}