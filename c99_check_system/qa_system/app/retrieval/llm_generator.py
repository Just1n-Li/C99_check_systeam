"""
app/retrieval/llm_generator.py — 大语言模型调用模块
负责：马明晓
使用 LangChain ChatOpenAI 对接 OpenAI 兼容协议。
支持任何兼容 OpenAI 协议的模型服务（DeepSeek、通义千问、Ollama、vLLM 等），
只需在 .env 中配置 base_url、api_key、model_name 即可切换。
"""

from langchain_openai import ChatOpenAI
from langchain_core.messages import SystemMessage, HumanMessage

from config import settings


def create_llm(
    temperature: float = None,
    max_tokens: int = None,
) -> ChatOpenAI:
    """
    创建 LLM 实例（统一工厂方法）。
    所有模块通过此函数获取 LLM，保持配置一致。
    支持 Ollama（USE_OLLAMA=on）与任意 OpenAI 兼容 API。
    """
    return ChatOpenAI(
        base_url=settings.resolved_llm_base_url(),
        api_key=settings.resolved_llm_api_key(),
        model_name=settings.LLM_MODEL_NAME,
        temperature=temperature if temperature is not None else settings.LLM_TEMPERATURE,
        max_tokens=max_tokens or settings.LLM_MAX_TOKENS,
        timeout=settings.LLM_TIMEOUT,
        max_retries=settings.LLM_MAX_RETRIES,
    )


# ── 意图识别 Prompt ─────────────────────────────────────────────
INTENT_SYSTEM_PROMPT = """你是一个法律法规智能问答系统的意图识别模块。
分析用户问题，判断类别并只返回 JSON。

意图类别：
- labor_law：劳动法 / 劳动合同 / 工资 / 试用期等
- civil_law：民法 / 合同 / 侵权 / 诉讼时效等
- consumer_law：消费者权益 / 退货 / 欺诈赔偿等
- dispute_procedure：劳动争议仲裁 / 调解程序等
- general_legal：其他法律法规咨询
- unknown：无法归类

只返回：
{"intent": "意图标签", "entity": "核心实体（法律名/条文关键词）"}
无法提取实体时 entity 为空字符串。"""

# ── 回答生成 Prompt（C99 测试代码生成）──────────────────────────
ANSWER_SYSTEM_PROMPT = """你是一个 C 语言（C99 标准）编译器符合性测试代码生成器。

根据给定的 C99 条款原文，生成一个 C 测试程序，**同时包含「正向测试」和「负向测试」两部分**：

【正向测试】验证条款的语义（Semantics）：能编译、能运行的正常 C 代码，用 assert 或 printf 验证运行结果，覆盖条款的每条语义要求。

【负向测试】验证条款的约束（Constraints）：写出故意违反约束的代码片段，期望编译器拒绝（编译报错）。

格式规则：
1. 完整代码放在一个 ```c 代码块内。
2. 用醒目的注释把两部分分开，让读者一眼看出哪里是正向、哪里是负向，例如：
   /* ========== 正向测试：以下代码应能编译并运行通过 ========== */
   （正常 C 代码 + assert）
   /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
   #if 0
   /* 违反约束「操作数必须为算术类型」：结构体不能相乘，gcc -std=c99 应报错 */
   struct S { int x; } a, b;
   a * b;
   #endif
3. 所有负向片段统一放在同一个 `#if 0 ... #endif` 块里，保证整个文件仍能正常编译运行；每个负向片段前用注释说明违反了哪条约束、期望什么报错。
4. 负向测试只针对「约束（constraint）」（违反后编译器必须报错的规则）；不要把「未定义行为(UB)」当作负向测试——UB 代码仍能编译通过，只是运行行为未定义。
5. 程序开头注释写明：验证 C99 哪一条款、预期行为（正向运行通过、负向编译报错）。
6. 严格基于条款内容，不要引入条款未涉及的特性。
7. 代码块之外可附 1~3 句中文说明，但代码块是主体。
8. 逐条覆盖条款的每个编号段落（[1][2]…）、Constraints、Semantics、EXAMPLE，不要漏任何一段；每个测试用注释标注对应段落号（如 /* [6] 无原型默认实参提升 */）。
9. 这些语义最容易被漏，务必测到：无原型函数调用的默认实参提升（char→int、float→double）与省略号（...）后停止转换；「非左值」结果（函数返回结构体的成员 f().x、强制转换、条件/逗号表达式的结果）用负向测试验证「对它赋值应编译报错」；const/volatile 限定类型在成员访问与解引用上的传播。

直接给出代码，不要有「好的」「当然」等废话开场。"""

class LLMGenerator:
    """
    大语言模型生成器（LangChain ChatOpenAI 版本）。
    通过 OpenAI 兼容协议对接任意模型服务。
    """

    def __init__(self):
        self.llm = create_llm()

    async def simple_chat(self, system_prompt: str, user_message: str) -> str:
        """
        简单的单轮对话接口，供 IntentParser 调用做意图识别。
        """
        messages = [
            SystemMessage(content=system_prompt),
            HumanMessage(content=user_message),
        ]
        response = await self.llm.ainvoke(messages)
        return response.content

    async def generate_answer(self, question: str, facts: str, intent: str) -> str:
        """
        根据图谱事实生成最终自然语言回答，供 AnswerBuilder 调用。
        """
        user_message = f"""C99 条款：{question}

条款原文（C99 标准）：
{facts}

请根据以上条款，生成用于测试编译器是否符合该条款的 C 测试程序："""

        messages = [
            SystemMessage(content=ANSWER_SYSTEM_PROMPT),
            HumanMessage(content=user_message),
        ]
        response = await self.llm.ainvoke(messages)
        return response.content
