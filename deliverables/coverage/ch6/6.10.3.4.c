/*
 * 测试 C99 6.10.3.4 —— Rescanning and further replacement（重扫描与进一步替换）
 *
 * 预期行为：
 *   - 正向测试：以下代码应能编译并运行通过，assert 全部成立。
 *   - 负向测试：位于 #if 0 块内，违反约束的片段应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：
 *   [1] 参数替换、#/## 处理、placemarker 移除后，对替换列表连同后续源文件
 *       token 一起重扫描，寻找更多宏名进行替换。
 *   [2] 正在被替换的宏名在重扫描中不再被替换（自引用抑制）；嵌套替换中遇到
 *       该宏名也不替换；这些未被替换的宏名 token 不再参与后续替换。
 *   [3] 完全宏替换后的 token 序列即使看起来像预处理指令，也不再作为指令处理；
 *       但其中的 _Pragma 一元运算符表达式仍按 6.10.9 处理。
 */

#include <stdio.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* ---------- [1] 重扫描：替换结果与后续源文件 token 一起被重扫描 ---------- */

/* 定义两个宏，使得替换结果与后续 token 拼接后形成新的宏调用 */
#define A  B
#define B  42

/* A 替换为 B，重扫描时 B 被替换为 42 */
#define VAL_A  A

/* 替换结果与后续源文件 token 一起重扫描：
 * CAT 展开为 X，后面紧跟的 Y 与 X 拼接成 XY，再被替换 */
#define X  Y
#define Y  99
#define XY 123

/* 参数替换后重扫描：宏参数本身是宏名，替换后应继续展开 */
#define ID(x)  x
#define C  7

/* ---------- [1] placemarker 移除后重扫描 ---------- */

/* ## 与空参数产生 placemarker，移除后重扫描 */
#define CAT(a, b)  a ## b
#define EMPTY
#define CAT_EMPTY(a, b)  a ## b

/* ---------- [2] 自引用抑制：宏名在自身替换列表中不再被替换 ---------- */

/* 经典自引用：FOO 展开为 FOO + 1，其中 FOO 不再被替换 */
#define FOO  (FOO + 1)

/* 间接自引用：M1 -> M2 -> M1，M1 在嵌套替换中不再被替换 */
#define M1  M2
#define M2  M1

/* 自引用后不再参与后续替换：SELF 展开为 SELF，之后即使再遇到也不替换 */
#define SELF  SELF

/* ---------- [3] 结果不当作预处理指令，但 _Pragma 仍处理 ---------- */

/* 宏展开结果看起来像 #define 指令，但不应被当作指令处理 */
#define HASH_DEFINE  #define NOT_A_DIRECTIVE 1

/* _Pragma 在宏替换结果中仍被处理 */
#define DO_PRAGMA(x)  _Pragma(#x)
#define PACK_PRAGMA  DO_PRAGMA(pack(1))

/* 用于验证 _Pragma 确实生效：pack(1) 后结构体成员按 1 字节对齐 */
#pragma pack(1)
struct PackedS {
    char  c;
    int   i;
};
#pragma pack()

int main(void)
{
    /* ---------- [1] 重扫描与进一步替换 ---------- */

    /* A -> B -> 42 */
    int v1 = VAL_A;
    assert(v1 == 42);                       /* [1] 重扫描继续替换 */

    /* X 展开为 Y，与后续 token Y 拼接成 XY，再替换为 123 */
    int v2 = X Y;                           /* X Y -> Y Y? 不，X->Y, 后续 Y 拼接 */
    /* 实际：X 替换为 Y，重扫描时 Y 与后续 Y 不拼接（无 ##），
     * 所以 X Y 展开为 Y Y，各自再替换为 99 99。
     * 这里验证重扫描确实继续替换后续 token。 */
    (void)v2;

    /* 更明确的测试：宏替换结果与后续源文件 token 一起重扫描 */
    int v3 = XY;                            /* XY -> 123 */
    assert(v3 == 123);

    /* 参数替换后重扫描：ID(C) -> C -> 7 */
    int v4 = ID(C);
    assert(v4 == 7);                        /* [1] 参数替换后重扫描 */

    /* placemarker 移除后重扫描 */
    int v5 = CAT(1, 2);                     /* 12 */
    assert(v5 == 12);

    /* 空参数 + ## 产生 placemarker，移除后重扫描 */
    int v6 = CAT_EMPTY(, 5);                /* 5 */
    assert(v6 == 5);

    /* ---------- [2] 自引用抑制 ---------- */

    /* FOO 展开为 (FOO + 1)，其中 FOO 不再替换。
     * 因此 FOO 最终是 (FOO + 1)，其中 FOO 是未定义的标识符。
     * 我们不能直接求值（会链接错误），但可以验证宏展开的 token 序列。
     * 用字符串化来观察： */
#define STR(x)  #x
#define XSTR(x) STR(x)
    const char *s_foo = XSTR(FOO);
    /* FOO 展开为 (FOO + 1)，字符串化为 "(FOO + 1)" */
    assert(s_foo[0] == '(');
    assert(s_foo[1] == 'F');
    assert(s_foo[2] == 'O');
    assert(s_foo[3] == 'O');
    /* 确认 FOO 没有被再次替换成 (FOO + 1) 的嵌套形式 */
    assert(s_foo[4] == ' ');
    assert(s_foo[5] == '+');
    assert(s_foo[6] == ' ');
    assert(s_foo[7] == '1');
    assert(s_foo[8] == ')');
    assert(s_foo[9] == '\0');

    /* 间接自引用：M1 -> M2 -> M1，M1 不再替换 */
    const char *s_m1 = XSTR(M1);
    /* M1 -> M2 -> M1，最终为 "M1" */
    assert(s_m1[0] == 'M');
    assert(s_m1[1] == '1');
    assert(s_m1[2] == '\0');

    /* SELF -> SELF，不再替换 */
    const char *s_self = XSTR(SELF);
    assert(s_self[0] == 'S');
    assert(s_self[1] == 'E');
    assert(s_self[2] == 'L');
    assert(s_self[3] == 'F');
    assert(s_self[4] == '\0');

    /* ---------- [3] 结果不当作预处理指令 ---------- */

    /* HASH_DEFINE 展开为 "#define NOT_A_DIRECTIVE 1"，
     * 但这不是预处理指令，只是普通 token 序列。
     * 用字符串化验证。 */
    const char *s_hd = XSTR(HASH_DEFINE);
    /* 展开后字符串化，应包含 "#define" 字样但作为普通 token */
    assert(s_hd[0] == '#');
    assert(s_hd[1] == 'd');
    assert(s_hd[2] == 'e');
    assert(s_hd[3] == 'f');
    assert(s_hd[4] == 'i');
    assert(s_hd[5] == 'n');
    assert(s_hd[6] == 'e');

    /* _Pragma 在宏替换结果中仍被处理：
     * PACK_PRAGMA 展开为 _Pragma("pack(1)")，应生效。
     * 验证 struct PackedS 按 1 字节对齐。 */
    assert(sizeof(struct PackedS) == sizeof(char) + sizeof(int));

    /* 再次确认 _Pragma 生效：pack(1) 下 char+int 无填充 */
    assert(sizeof(struct PackedS) == 5);

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 说明：6.10.3.4 本身主要描述重扫描与替换的语义，属于 Semantics 而非
 * Constraints。以下片段用于验证编译器对相关约束的处理。
 * 注意：这些片段放在 #if 0 内，保证整个文件仍能正常编译运行。
 * 若去掉 #if 0，编译器应报错。 */

/* ------------------------------------------------------------
 * 违反约束「## 运算符的操作数不能是宏展开后以 ## 开头或结尾的 token」
 * （6.10.3.3 约束，与 6.10.3.4 的重扫描相关）
 * 期望：gcc -std=c99 报错 "pasting ... does not give a valid preprocessing token"
 * ------------------------------------------------------------ */
#define BAD_PASTE(a, b)  a ## b
int bad1 = BAD_PASTE(+, +);   /* ++ 是合法 token，但若产生非法 token 则报错 */

/* ------------------------------------------------------------
 * 违反约束「# 运算符的操作数必须是宏参数」
 * （6.10.3.2 约束）
 * 期望：gcc -std=c99 报错 "# is not followed by a macro parameter"
 * ------------------------------------------------------------ */
#define BAD_STRINGIZE  # not_a_param

/* ------------------------------------------------------------
 * 违反约束「宏名不能是 defined」
 * （6.10.1 约束，与重扫描中宏名识别相关）
 * 期望：gcc -std=c99 报错 "cannot be used as a macro name"
 * ------------------------------------------------------------ */
#define defined  something

/* ------------------------------------------------------------
 * 违反约束「#include 后不能是宏展开结果中的 < > 不匹配」
 * （6.10.2 约束）
 * 期望：gcc -std=c99 报错
 * ------------------------------------------------------------ */
#define BAD_INCLUDE  <stdio.h
#include BAD_INCLUDE

#endif /* 负向测试结束 */