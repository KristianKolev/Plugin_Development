#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataContainers.h"
#include "UpgradeDataProvider.generated.h"

UCLASS(Abstract)
class PLUGIN_DEVELOPMENT_API UUpgradeDataProvider : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Initializes upgrade-related data from a specified file and populates the relevant output parameters.
     *
     * @param FolderPath The folder path to load data from.
     * @param OutCatalog Reference to a catalog of all upgrade paths and their corresponding upgrade definitions. This lives in the Subsystem.
     * @param OutResourceTypes Reference to an array that will be populated with unique resource type names encountered during initialization. This lives in the Subsystem.
     *
     */
    virtual void InitializeData(const FString& FolderPath, TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog, TArray<FName>& OutResourceTypes) PURE_VIRTUAL(UUpgradeDataProviderBase::InitializeData, );
    
    // Helper function to add a resource type to the resource type array if it doesn't already exist.
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