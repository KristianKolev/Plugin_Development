#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataProvider.h"
#include "UpgradeDataTableProvider.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeDataTableProvider : public UUpgradeDataProvider
{
	GENERATED_BODY()
public:
	UUpgradeDataTableProvider();

	virtual void InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes) override;
};
