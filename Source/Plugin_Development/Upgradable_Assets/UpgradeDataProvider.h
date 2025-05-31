

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
	virtual const FUpgradeLevelData* GetLevelData(int32 Level) const PURE_VIRTUAL(UUpgradeDataProvider::GetLevelData, return nullptr;);
	virtual int32 GetMaxLevel() const PURE_VIRTUAL(UUpgradeDataProvider::GetMaxLevels, return 0;);
	virtual int32 AddRequiredResourceType(FName &ResourceType, TArray<FName>& ResourceTypes) PURE_VIRTUAL(UUpgradeDataProvider::AddRequiredResourceType, return 0;)
};