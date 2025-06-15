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
       virtual void InitializeData(TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog,
                                   TArray<FName>& OutResourceTypes) override;
};
