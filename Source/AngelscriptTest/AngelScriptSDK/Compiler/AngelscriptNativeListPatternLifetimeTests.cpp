#include "Support/AngelscriptNativeBuilderTestSupport.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "StartAngelscriptHeaders.h"
#include "source/as_scriptfunction.h"
#include "source/as_scriptnode.h"
#include "EndAngelscriptHeaders.h"

#if WITH_ANGELSCRIPT_UNITTESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAngelscriptListPatternLifetimeTest,
    "Angelscript.TestModule.AngelScriptSDK.Compiler.ListPatternLifetime",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAngelscriptListPatternLifetimeTest::RunTest(const FString&)
{
    auto* Engine=AngelscriptNativeTestSupport::CreateBareSdkEngine(this);
    if(!Engine) return false;
    ON_SCOPE_EXIT { AngelscriptNativeTestSupport::DestroyNativeEngine(Engine); };
    asCBuilder Builder(Engine,nullptr);
    asCScriptFunction Function(Engine,nullptr,asFUNC_DUMMY);
    asCScriptNode* Pattern=nullptr;
    const char* Declaration="void f(int&in) {repeat int}";
    const int Result=Builder.ParseFunctionDeclaration(nullptr,Declaration,&Function,true,nullptr,nullptr,Engine->defaultNamespace,&Pattern);
    TestEqual(TEXT("List declaration parses"),Result,0);
    TestNotNull(TEXT("Parser returns a list pattern"),Pattern);
    if(!Pattern) return false;
    // Check ownership before dereferencing: the old implementation returned freed parser-arena memory.
    // 先检查所有权再解引用；旧实现返回的是已经释放的parser arena内存。
    const bool Owned=Builder.MemStack.ContainsPointer(Pattern);
    TestTrue(TEXT("Returned pattern survives in builder arena"),Owned);
    if(!Owned) return false;
    TestEqual(TEXT("Detached root has the expected node kind"),int(Pattern->nodeType),int(snListPattern));
    TestTrue(TEXT("Copied children also belong to the builder"),Pattern->firstChild && Builder.MemStack.ContainsPointer(Pattern->firstChild));
    TestEqual(TEXT("Consumer can register after parser destruction"),Function.RegisterListPattern(Declaration,Pattern),0);
    return true;
}
#endif
