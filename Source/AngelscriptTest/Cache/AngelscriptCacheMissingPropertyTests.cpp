#include "Misc/AutomationTest.h"
#include "as_restore.h"
#include "as_scriptengine.h"

#if WITH_ANGELSCRIPT_UNITTESTS
namespace
{
class FPropertyWriterProbe : public asCWriter
{
public:
    explicit FPropertyWriterProbe(asCScriptEngine* Engine) : asCWriter(nullptr, nullptr, Engine, true) {}
    bool Find(short Offset, int Type)
    {
        asDWORD Code[4]{}; Code[0] = asBC_LoadThisR;
        FindObjectPropIndex(Offset, Type, Code);
        return !error;
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAngelscriptCacheMissingPropertyTest,
    "Angelscript.Cache.Writer.MissingPropertyFailsClosed", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAngelscriptCacheMissingPropertyTest::RunTest(const FString&)
{
    auto* Engine = static_cast<asCScriptEngine*>(asCreateScriptEngine(ANGELSCRIPT_VERSION));
    if (!TestNotNull(TEXT("Engine"), Engine)) return false;
    Engine->RegisterObjectType("PropertyProbe", 0, asOBJ_REF | asOBJ_NOCOUNT);
    Engine->RegisterObjectProperty("PropertyProbe", "int Value", 8);
    const int Type = Engine->GetTypeIdByDecl("PropertyProbe");
    FPropertyWriterProbe Valid(Engine), Missing(Engine), InvalidType(Engine);
    TestTrue(TEXT("Known property still serializes"), Valid.Find(8, Type));
    TestFalse(TEXT("Unregistered host offset fails instead of dereferencing null"), Missing.Find(12, Type));
    TestFalse(TEXT("Unknown object type fails instead of dereferencing null"), InvalidType.Find(12, asTYPEID_INT32));
    Engine->ShutDownAndRelease(); return true;
}
#endif
