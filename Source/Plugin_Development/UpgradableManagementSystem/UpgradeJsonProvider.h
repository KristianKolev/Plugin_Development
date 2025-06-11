#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataProvider.h"
#include "UpgradeJsonProvider.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeJsonProvider : public UUpgradeDataProvider
{
	GENERATED_BODY()
public:
	UUpgradeJsonProvider();
	// Consider if I should make the JSON values dynamic.
	virtual void InitializeData(const FString& FolderPath, TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog, TArray<FName>& OutResourceTypes) override;
};
