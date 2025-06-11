#include "UpgradeDataAssetProvider.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UpgradeDefinitionDataAsset.h"

UUpgradeDataAssetProvider::UUpgradeDataAssetProvider()
{
}

void UUpgradeDataAssetProvider::InitializeData(const FString& FolderPath, TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    // Query AssetRegistry for all assets under that path
    FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetsInFolder;
    RegistryModule.Get().GetAssetsByPath(FName(*FolderPath), AssetsInFolder, /*bRecursive=*/true);
    
    if (AssetsInFolder.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPGRADEASSET_ERR_00] No assets found in path: '%s'"), *FolderPath);
        return;
    }
 
    int32 LoadedAssets = 0;
    for (const FAssetData& AssetData : AssetsInFolder)
    {
        if (AssetData.AssetClassPath != UUpgradeDefinitionDataAsset::StaticClass()->GetClassPathName())
            continue;

        UUpgradeDefinitionDataAsset* Asset = Cast<UUpgradeDefinitionDataAsset>(AssetData.GetAsset());
        if (!Asset)
        {
            UE_LOG(LogTemp, Warning, TEXT("[UPGRADEASSET_ERR_01] Failed to load UpgradeDefinitionDataAsset '%s'"), 
                *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedAssets;
        
        // Fallback to asset name if no UpgradePathId is set
        FName PathId = !Asset->UpgradePathId.IsNone() ? Asset->UpgradePathId : AssetData.AssetName;
        TArray<FUpgradeDefinition>& LevelDataArray = OutCatalog.FindOrAdd(PathId);

        int32 ProcessedLevels = 0;
        for (const FUpgradeDefinitionAsset& LevelAsset : Asset->Levels)
        {
            FUpgradeDefinition LevelData;
            
            if (LevelAsset.UpgradeResourceCosts.Num() == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("[UPGRADEASSET_ERR_02] No resource costs defined for level %d in asset '%s'"), 
                    ProcessedLevels, *AssetData.AssetName.ToString());
                continue;
            }

            for (const auto& Resource : LevelAsset.UpgradeResourceCosts)
            {
                int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(Resource.Key, OutResourceTypes);
                LevelData.ResourceTypeIndices.Add(ResourceIndex);
                LevelData.UpgradeCosts.Add(Resource.Value);
            }
            
            LevelData.UpgradeSeconds = LevelAsset.UpgradeSeconds;
            LevelData.bUpgradeLocked = LevelAsset.bUpgradeLocked;
            LevelDataArray.Add(LevelData);
            ++ProcessedLevels;
        }
        
        if (ProcessedLevels > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("[UPGRADEASSET_INFO_01] Successfully processed asset '%s' with %d levels"), 
                *AssetData.AssetName.ToString(), ProcessedLevels);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[UPGRADEASSET_ERR_03] No valid levels processed in asset '%s'"), 
                *AssetData.AssetName.ToString());
        }
    }

    // Check if any Data Assets were actually loaded
    if (LoadedAssets == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPGRADEASSET_ERR_04] No UpgradeDefinitionDataAssets found in path: '%s'"), *FolderPath);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[UPGRADEASSET_INFO_02] Loaded %d Data Assets from '%s' (found %d PathIds)"),
        LoadedAssets, *FolderPath, OutCatalog.Num());
}