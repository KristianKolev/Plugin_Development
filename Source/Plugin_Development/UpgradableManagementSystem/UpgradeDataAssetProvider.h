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

        virtual void InitializeData(TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog,
                                     TArray<FName>& OutResourceTypes) override;
};
