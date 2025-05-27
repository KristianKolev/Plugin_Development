

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
	virtual int32 GetMaxLevels() const PURE_VIRTUAL(UUpgradeDataProvider::GetMaxLevels, return 0;);
};