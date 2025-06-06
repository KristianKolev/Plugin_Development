#include "UpgradeDataAssetProvider.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UpgradeDefinitionDataAsset.h"

UUpgradeDataAssetProvider::UUpgradeDataAssetProvider()
{
}

void UUpgradeDataAssetProvider::InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    // Query AssetRegistry for all assets under that path
    FAssetRegistryModule& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetList;
    AR.Get().GetAssetsByPath(FName(*FilePath), AssetList, /*bRecursive=*/true);
    
    int32 LoadedAssets = 0;
    for (const FAssetData& AD : AssetList)
    {
        // Only care about UpgradeDefinitionDataAsset assets
        if (AD.AssetClassPath != UUpgradeDefinitionDataAsset::StaticClass()->GetClassPathName())
            continue;

        // Load the asset
        UUpgradeDefinitionDataAsset* Asset = Cast<UUpgradeDefinitionDataAsset>(AD.GetAsset());
        if (!Asset)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpgradeDataAssetProvider: Failed to load UpgradeDefinitionDataAsset '%s'"), 
                *AD.ObjectPath.ToString());
            continue;
        }
        ++LoadedAssets;

        // Determine which PathId to use:
        // If the asset has an explicit OverrideUpgradePath (non-None), use that; otherwise fallback to asset name
        FName PathId = !Asset->OverrideUpgradePath.IsNone() ? Asset->OverrideUpgradePath : AD.AssetName;

        // Convert FUpgradeLevelDataAsset array to FUpgradeLevelData array
        TArray<FUpgradeLevelData> LevelDataArray;
        for (const FUpgradeLevelDataAsset& LevelAsset : Asset->Levels)
        {
            FUpgradeLevelData LevelData;
            
            // Process resource types and costs
            for (const auto& Resource : LevelAsset.UpgradeResourceCosts)
            {
                int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(Resource.Key, OutResourceTypes);
                LevelData.ResourceTypeIndices.Add(ResourceIndex);
                LevelData.UpgradeCosts.Add(Resource.Value);
            }
            
            LevelData.UpgradeSeconds = LevelAsset.UpgradeSeconds;
            LevelData.bUpgradeLocked = LevelAsset.bUpgradeLocked;
            
            LevelDataArray.Add(LevelData);
        }

        // Add the converted levels array to the catalog 
        OutCatalog.Add(PathId, LevelDataArray);
    }

    UE_LOG(LogTemp, Log, TEXT("UpgradeDataAssetProvider: Loaded %d Data Assets from '%s' (found %d PathIds)"),
        LoadedAssets, *FilePath, OutCatalog.Num());
}