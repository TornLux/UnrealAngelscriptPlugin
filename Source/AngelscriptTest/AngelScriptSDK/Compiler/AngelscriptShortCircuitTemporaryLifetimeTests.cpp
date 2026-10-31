#include "Support/AngelscriptNativeCoreTestSupport.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace
{
struct FShortCircuitValue { bool Value = true; };
struct FShortCircuitCounts { int32 Constructed = 0; int32 Destroyed = 0; };
constexpr asPWORD ShortCircuitCountsSlot = 0x53434C494645;

FShortCircuitCounts& Counts(asIScriptGeneric* Call)
{
    return *static_cast<FShortCircuitCounts*>(Call->GetEngine()->GetUserData(ShortCircuitCountsSlot));
}
void ConstructShortCircuitValue(asIScriptGeneric* Call)
{
    new (Call->GetObject()) FShortCircuitValue;
    ++Counts(Call).Constructed;
}
void CopyShortCircuitValue(asIScriptGeneric* Call)
{
    new (Call->GetObject()) FShortCircuitValue(*static_cast<FShortCircuitValue*>(Call->GetArgAddress(0)));
    ++Counts(Call).Constructed;
}
void AssignShortCircuitValue(asIScriptGeneric* Call)
{
    *static_cast<FShortCircuitValue*>(Call->GetObject()) = *static_cast<FShortCircuitValue*>(Call->GetArgAddress(0));
    Call->SetReturnAddress(Call->GetObject());
}
void DestroyShortCircuitValue(asIScriptGeneric* Call)
{
    // Count without dereferencing: the regression destroys storage that was never constructed.
    // 只计数、不解引用；回归场景会析构根本没有构造过的存储，不能让测试本身非法释放内存。
    ++Counts(Call).Destroyed;
}
void MakeShortCircuitValue(asIScriptGeneric* Call)
{
    new (Call->GetAddressOfReturnLocation()) FShortCircuitValue;
    ++Counts(Call).Constructed;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAngelscriptShortCircuitTemporaryLifetimeTest,
    "Angelscript.TestModule.AngelScriptSDK.Compiler.ShortCircuitTemporaryLifetime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAngelscriptShortCircuitTemporaryLifetimeTest::RunTest(const FString&)
{
    for (bool WithoutLineCues : {false, true})
    {
        auto* Engine = AngelscriptNativeTestSupport::CreateBareSdkEngine(this);
        if (!Engine) return false;
        ON_SCOPE_EXIT { AngelscriptNativeTestSupport::DestroyNativeEngine(Engine); };
        FShortCircuitCounts Counter;
        Engine->SetUserData(&Counter, ShortCircuitCountsSlot);
        Engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, WithoutLineCues);
        const auto Register = [this](int Result) { return TestTrue(TEXT("Lifetime fixture binds"), Result >= 0); };
        if (!Register(Engine->RegisterObjectType("FShortCircuitValue", sizeof(FShortCircuitValue), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_APP_CLASS_ALLINTS))
            || !Register(Engine->RegisterObjectBehaviour("FShortCircuitValue", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructShortCircuitValue), asCALL_GENERIC))
            || !Register(Engine->RegisterObjectBehaviour("FShortCircuitValue", asBEHAVE_CONSTRUCT, "void f(const FShortCircuitValue& in)", asFUNCTION(CopyShortCircuitValue), asCALL_GENERIC))
            || !Register(Engine->RegisterObjectBehaviour("FShortCircuitValue", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestroyShortCircuitValue), asCALL_GENERIC))
            || !Register(Engine->RegisterObjectMethod("FShortCircuitValue", "FShortCircuitValue& opAssign(const FShortCircuitValue& in)", asFUNCTION(AssignShortCircuitValue), asCALL_GENERIC))
            || !Register(Engine->RegisterObjectProperty("FShortCircuitValue", "bool Value", asOFFSET(FShortCircuitValue, Value)))
            || !Register(Engine->RegisterGlobalFunction("FShortCircuitValue MakeValue()", asFUNCTION(MakeShortCircuitValue), asCALL_GENERIC))) return false;
        auto* Module = Engine->GetModule("ShortCircuitLifetime", asGM_ALWAYS_CREATE);
        Module->AddScriptSection("ShortCircuitLifetime", R"(
            bool Or(bool Skip) { return Skip || MakeValue().Value; }
            bool And(bool Evaluate) { return Evaluate && MakeValue().Value; }
            bool Pass(bool Result) { return Result; }
            bool OrArgument(bool Skip) { return Pass(Skip || MakeValue().Value); }
            bool AndArgument(bool Evaluate) { return Pass(Evaluate && MakeValue().Value); }
        )");
        if (!TestEqual(TEXT("Lifetime fixture compiles"), Module->Build(), 0)) return false;
        auto* Context = Engine->CreateContext();
        ON_SCOPE_EXIT { Context->Release(); };
        for (const char* Function : {"bool Or(bool)", "bool And(bool)", "bool OrArgument(bool)", "bool AndArgument(bool)"})
        {
            for (bool Argument : {false, true})
            {
                Counter = {};
                Context->Prepare(Module->GetFunctionByDecl(Function));
                Context->SetArgByte(0, Argument);
                TestEqual(TEXT("Short-circuit function completes"), Context->Execute(), int(asEXECUTION_FINISHED));
                Context->Unprepare();
                const int Expected = FCStringAnsi::Strstr(Function, "Or") ? !Argument : Argument;
                const FString Label = FString::Printf(TEXT("%s(%d), no line cues=%d"), UTF8_TO_TCHAR(Function), Argument, WithoutLineCues);
                TestEqual(Label + TEXT(" only evaluates the selected operand"), Counter.Constructed, Expected);
                TestEqual(Label + TEXT(" only destroys constructed temporaries"), Counter.Destroyed, Counter.Constructed);
            }
        }
    }
    return true;
}
#endif
