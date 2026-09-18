#pragma once

#include "Misc/EngineVersionComparison.h"
#include "Misc/CoreDelegates.h"
#include "UObject/UnrealType.h"

namespace AngelscriptUECompatibility
{
    // UE 5.8 removed the EObjectFlags constructor argument. / UE 5.7 仍需要属性标记参数。
    template<typename PropertyType>
    PropertyType* NewProperty(FFieldVariant Owner, const FName& Name)
    {
#if UE_VERSION_OLDER_THAN(5, 8, 0)
        return new PropertyType(Owner, Name, RF_Public);
#else
        return new PropertyType(Owner, Name);
#endif
    }

    // The delegate became an accessor in UE 5.8. / 兼容 UE 5.7 的公开委托成员。
    inline FSimpleMulticastDelegate& OnPostEngineInit()
    {
#if UE_VERSION_OLDER_THAN(5, 8, 0)
        return FCoreDelegates::OnPostEngineInit;
#else
        return FCoreDelegates::GetOnPostEngineInit();
#endif
    }
}
