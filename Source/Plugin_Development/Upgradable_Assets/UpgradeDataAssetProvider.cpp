#include "UpgradeDataAssetProvider.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "UpgradeDefinitionDataAsset.h"

UUpgradeDataAssetProvider::UUpgradeDataAssetProvider()
{
}

void UUpgradeDataAssetProvider::InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    // Query AssetRegistry for all assets under that path
    FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetList;
    RegistryModule.Get().GetAssetsByPath(FName(*FilePath), AssetList, /*bRecursive=*/true);

   
 
    
    int32 LoadedAssets = 0;
    for (const FAssetData& AssetData : AssetList)
    {
        // Only care about UpgradeDefinitionDataAsset assets
        if (AssetData.AssetClassPath != UUpgradeDefinitionDataAsset::StaticClass()->GetClassPathName())
            continue;

        // Load the asset
        UUpgradeDefinitionDataAsset* Asset = Cast<UUpgradeDefinitionDataAsset>(AssetData.GetAsset());
        if (!Asset)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpgradeDataAssetProvider: Failed to load UpgradeDefinitionDataAsset '%s'"), 
                *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedAssets;

        // Determine which PathId to use:
        // If the asset has an explicit OverrideUpgradePath (non-None), use that; otherwise fallback to asset name
        FName PathId = !Asset->UpgradePath.IsNone() ? Asset->UpgradePath : AssetData.AssetName;
        TArray<FUpgradeLevelData>& LevelDataArray = OutCatalog.FindOrAdd(PathId);

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
    }

    UE_LOG(LogTemp, Log, TEXT("UpgradeDataAssetProvider: Loaded %d Data Assets from '%s' (found %d PathIds)"),
        LoadedAssets, *FilePath, OutCatalog.Num());
}