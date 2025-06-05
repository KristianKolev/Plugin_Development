#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataProvider.h"
#include "UpgradeDataAssetProvider.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeDataAssetProvider : public UUpgradeDataProvider
{
	GENERATED_BODY()
	
public:
	UUpgradeDataAssetProvider();


	virtual void InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes) override;
	virtual int32 AddRequiredResourceType(const FName& ResourceType, TArray<FName>& ResourceTypes) override;	
};

