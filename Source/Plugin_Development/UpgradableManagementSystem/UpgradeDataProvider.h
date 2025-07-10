#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "UpgradeDataContainers.h"
#include "MightyraiderFunctionLibrary.h"
#include "UpgradeDataProvider.generated.h"

UCLASS(Abstract)
class PLUGIN_DEVELOPMENT_API UUpgradeDataProvider : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Scans FolderPath once and creates provider instances for each supported
     * data type found. Providers are selected via a class-to-provider mapping
     * so new data types can be registered without editing this function.
     * The returned providers already contain any detected assets or files and
     * are ready for InitializeData().
     */
    virtual TArray<UUpgradeDataProvider*> Scan(const FString& FolderPath);

    // Currently supported Data Tables and Data Assets
    virtual void ScanForAssets(const FString& FolderPath, TArray<UUpgradeDataProvider*>& Providers);
    // Currently supported files (e.g. JSON). Extend ExtensionToProvider map in ScanForFiles to support more types.
    virtual void ScanForFiles(const FString& FolderPath, TArray<UUpgradeDataProvider*>& Providers);

    /**
     * Initializes upgrade-related data using any assets/files gathered by Scan().
     *
     * @param OutCatalog       Reference to the catalog of all upgrade paths and their corresponding definitions.
     * @param OutResourceTypes Reference to an array populated with all encountered resource type names.
     */
    virtual void InitializeData(TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog,
                                TArray<FName>& OutResourceTypes) PURE_VIRTUAL(UUpgradeDataProvider::InitializeData, );

protected:
    /** Assets discovered during Scan() */
    UPROPERTY()
    TArray<FAssetData> DetectedAssets;

    /** File paths discovered during Scan() */
    UPROPERTY()
    TArray<FString> DetectedFiles;
    
    // Helper function to add a resource type to the resource type array if it doesn't already exist.
    virtual int32 AddOrFindRequiredResourceTypeIndex(const FName& ResourceType, TArray<FName>& ResourceTypes);

    virtual const FRequirementsScalingSegment* FindSegment(const TArray<FRequirementsScalingSegment>& Segments, int32 Level) const;

    virtual int32 ComputeRequirementsBySegment(const FRequirementsScalingSegment* Segment, int32 PreviousCost, int32 Level) const;
    
};