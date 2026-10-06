/*
 * 验证 C99 6.11.2 Linkages of identifiers
 * 预期行为：正向测试运行通过（可能伴随过时特性警告），负向测试无（因无约束）。
 */

#include <stdio.h>
#include <assert.h>

/* [1] Declaring an identifier with internal linkage at file scope without the static storage-class specifier is an obsolescent feature. */
static int internal_id = 10;
extern int internal_id; /* 过时特性：此处声明 internal_id 具有内部链接，但未使用 static */

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
int main(void) {
    /* [1] 验证过时特性代码仍能正常工作，internal_id 保持内部链接和值 */
    assert(internal_id == 10);
    internal_id = 20;
    assert(internal_id == 20);
    
    printf("C99 6.11.2 test passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 6.11.2 条款属于 "Future language directions"（未来方向），仅描述 "obsolescent feature"（过时特性）。
   该条款未定义任何 "Constraints"（约束），因此违反此条款不会导致编译报错，编译器通常会给出警告而非错误。
   故此处无负向测试。 */
#endif