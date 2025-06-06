#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UpgradeLevelData.h"
#include "UpgradeDataProvider.generated.h"

UCLASS(Abstract)
class PLUGIN_DEVELOPMENT_API UUpgradeDataProvider : public UObject
{
    GENERATED_BODY()

public:

    virtual void InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes) PURE_VIRTUAL(UUpgradeDataProviderBase::InitializeData, );
    
    // Changed from pure virtual to virtual with implementation
    virtual int32 AddOrFindRequiredResourceTypeIndex(const FName& ResourceType, TArray<FName>& ResourceTypes)
    {
        int32 FoundIndex = ResourceTypes.IndexOfByKey(ResourceType);
        if (FoundIndex != INDEX_NONE)
        {
            return FoundIndex;
        }
        int32 NewIndex = ResourceTypes.Num();
        ResourceTypes.Add(ResourceType);
        return NewIndex;
    }
};