/*
 * 测试 C99 7.4.1 —— 字符分类函数 (Character classification functions)
 *
 * 条款要求：
 *   [1] 本子条款中的函数，当且仅当参数 c 的值符合函数描述时，返回非零（真）。
 *
 * 预期行为：
 *   - 正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   - 负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 说明：7.4.1 本身只给出“当且仅当”的语义约定，具体每个函数
 *       (isalnum, isalpha, isblank, iscntrl, isdigit, isgraph,
 *        islower, isprint, ispunct, isspace, isupper, isxdigit)
 *        的判定集合由 7.4.1.1 ~ 7.4.1.12 给出。这里按 7.4.1 的
 *        “当且仅当”语义，对每个函数做正反两面的验证。
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] “当且仅当”语义：对每个分类函数，验证
     *     - 属于该类的字符返回非零（真）
     *     - 不属于该类的字符返回零（假）
     * 用“真值”而非具体非零值比较，因为标准只保证“非零”。 */

    /* ---- isalnum：字母或数字 ---- */
    assert(isalnum('A') != 0);   /* 字母 -> 真 */
    assert(isalnum('z') != 0);
    assert(isalnum('0') != 0);   /* 数字 -> 真 */
    assert(isalnum('9') != 0);
    assert(isalnum(' ') == 0);   /* 空格既非字母也非数字 -> 假 */
    assert(isalnum('!') == 0);

    /* ---- isalpha：字母 ---- */
    assert(isalpha('A') != 0);
    assert(isalpha('m') != 0);
    assert(isalpha('Z') != 0);
    assert(isalpha('0') == 0);   /* 数字不是字母 -> 假 */
    assert(isalpha(' ') == 0);

    /* ---- isblank：标准空白字符（空格、水平制表） ---- */
    assert(isblank(' ') != 0);
    assert(isblank('\t') != 0);
    assert(isblank('\n') == 0);  /* 换行不是 blank -> 假 */
    assert(isblank('a') == 0);

    /* ---- iscntrl：控制字符 ---- */
    assert(iscntrl('\n') != 0);
    assert(iscntrl('\t') != 0);
    assert(iscntrl('\0') != 0);
    assert(iscntrl('a') == 0);   /* 可打印字符不是控制字符 -> 假 */
    assert(iscntrl(' ') == 0);

    /* ---- isdigit：十进制数字 ---- */
    assert(isdigit('0') != 0);
    assert(isdigit('5') != 0);
    assert(isdigit('9') != 0);
    assert(isdigit('a') == 0);   /* 字母不是数字 -> 假 */
    assert(isdigit(' ') == 0);

    /* ---- isgraph：有图形表示的字符（可打印且非空格） ---- */
    assert(isgraph('a') != 0);
    assert(isgraph('!') != 0);
    assert(isgraph(' ') == 0);   /* 空格无图形表示 -> 假 */
    assert(isgraph('\n') == 0);

    /* ---- islower：小写字母 ---- */
    assert(islower('a') != 0);
    assert(islower('z') != 0);
    assert(islower('A') == 0);   /* 大写不是小写 -> 假 */
    assert(islower('0') == 0);

    /* ---- isprint：可打印字符（含空格） ---- */
    assert(isprint('a') != 0);
    assert(isprint(' ') != 0);   /* 空格可打印 -> 真 */
    assert(isprint('\n') == 0);  /* 换行不可打印 -> 假 */
    assert(isprint('\0') == 0);

    /* ---- ispunct：标点符号 ---- */
    assert(ispunct('!') != 0);
    assert(ispunct('.') != 0);
    assert(ispunct(',') != 0);
    assert(ispunct('a') == 0);   /* 字母不是标点 -> 假 */
    assert(ispunct(' ') == 0);

    /* ---- isspace：空白字符 ---- */
    assert(isspace(' ') != 0);
    assert(isspace('\t') != 0);
    assert(isspace('\n') != 0);
    assert(isspace('\v') != 0);
    assert(isspace('\f') != 0);
    assert(isspace('\r') != 0);
    assert(isspace('a') == 0);   /* 字母不是空白 -> 假 */

    /* ---- isupper：大写字母 ---- */
    assert(isupper('A') != 0);
    assert(isupper('Z') != 0);
    assert(isupper('a') == 0);   /* 小写不是大写 -> 假 */
    assert(isupper('0') == 0);

    /* ---- isxdigit：十六进制数字 ---- */
    assert(isxdigit('0') != 0);
    assert(isxdigit('9') != 0);
    assert(isxdigit('a') != 0);
    assert(isxdigit('F') != 0);
    assert(isxdigit('g') == 0);  /* 'g' 不是十六进制数字 -> 假 */
    assert(isxdigit(' ') == 0);

    /* [1] “当且仅当”的互斥性检查：
     * 一个字符不可能同时是 digit 和 alpha（对 ASCII 字母数字集合）。 */
    assert(!(isdigit('a') != 0 && isalpha('a') != 0));
    assert(!(isdigit('5') != 0 && isalpha('5') != 0));

    /* [1] 返回值语义：标准只保证“非零表示真、零表示假”，
     * 因此用 != 0 / == 0 判断，而非假定返回 1 / 0。 */
    {
        int r = isalpha('A');
        assert(r != 0);          /* 真值：非零即可 */
        r = isalpha('1');
        assert(r == 0);          /* 假值：必须为 0 */
    }

    printf("C99 7.4.1 character classification: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「7.4 字符处理函数的参数必须是可表示为 unsigned char 的值
     * 或等于 EOF」——传入超出该范围的负值（如 -2）属于约束违反，
     * 期望编译器/实现拒绝或至少给出诊断。
     * 注意：标准对“非 EOF 的负值”要求参数可表示为 unsigned char，
     * 传入 -2 违反该约束，gcc -std=c99 -Wall 应给出警告/错误。
     */
    {
        int bad = -2;
        (void)isalpha(bad);   /* 违反 7.4 参数约束：-2 既非 EOF 也非 unsigned char 值 */
    }

    /*
     * 违反约束「7.4.1 函数声明于 <ctype.h>」——未包含头文件直接调用，
     * 在 C99 中隐式函数声明是约束违反，应编译报错。
     */
    {
        (void)isdigit('5');   /* 未 #include <ctype.h>，隐式声明违反 C99 约束 */
    }

    /*
     * 违反约束「函数参数为 int 类型」——传入结构体类型，
     * 参数类型不匹配，应编译报错。
     */
    {
        struct S { int x; } s;
        (void)isupper(s);     /* 参数类型不匹配：结构体不能传给 int 形参 */
    }
#endif
}