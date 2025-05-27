



#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataProvider.h"
#include "UpgradeJsonProvider.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeJsonProvider : public UUpgradeDataProvider
{
	GENERATED_BODY()

public:
	void InitializeFromJson(const FString& Json);

	virtual const FUpgradeLevelData* GetLevelData(int32 Level) const override;
	virtual int32 GetMaxLevel() const override;

protected:
	UPROPERTY()
	TArray<FUpgradeLevelData> LevelDataArray;
};