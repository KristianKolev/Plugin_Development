#include "UpgradeDataAssetProvider.h"
#include "UpgradeDefinitionDataAsset.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "UpgradeManagerSubsystem.h"


UUpgradeDataAssetProvider::UUpgradeDataAssetProvider()
{
}


void UUpgradeDataAssetProvider::InitializeData(TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    if (DetectedAssets.Num() == 0)
    {
        UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_00] No UpgradeDefinitionDataAssets provided"));
        return;
    }

    int32 LoadedAssets = 0;
    for (const FAssetData& AssetData : DetectedAssets)
    {
        if (AssetData.AssetClassPath != UUpgradeDefinitionDataAsset::StaticClass()->GetClassPathName())
            continue;

        UUpgradeDefinitionDataAsset* Asset = Cast<UUpgradeDefinitionDataAsset>(AssetData.GetAsset());
        if (!Asset)
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_01] Failed to load UpgradeDefinitionDataAsset '%s'"), 
                *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedAssets;
        
        // Fallback to asset name if no UpgradePathId is set
        FName PathId = !Asset->UpgradePathId.IsNone() ? Asset->UpgradePathId : AssetData.AssetName;

        TArray<FUpgradeDefinition>* ExistingArray = OutCatalog.Find(PathId);
        if (ExistingArray)
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADECATALOG_WARN_01] Duplicate UpgradePath '%s' found in asset '%s'. Overriding previous data."),
                   *PathId.ToString(), *AssetData.AssetName.ToString());
            ExistingArray->Reset();
        }

        TArray<FUpgradeDefinition>& LevelDataArray = OutCatalog.FindOrAdd(PathId);

        int32 ProcessedLevels = 0;
        for (const FUpgradeDefinitionAsset& LevelAsset : Asset->Levels)
        {
            FUpgradeDefinition LevelData;
            
            if (LevelAsset.UpgradeResourceCosts.Num() == 0)
            {
                UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_02] No resource costs defined for level %d in asset '%s'"), 
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
            UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEASSET_INFO_01] Successfully processed asset '%s' with %d levels"), 
                *AssetData.AssetName.ToString(), ProcessedLevels);
        }
        else
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_03] No valid levels processed in asset '%s'"), 
                *AssetData.AssetName.ToString());
        }
    }

    // Check if any Data Assets were actually loaded
    if (LoadedAssets == 0)
    {
        UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_04] No UpgradeDefinitionDataAssets processed"));
        return;
    }

    UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEASSET_INFO_02] Loaded %d Data Assets (found %d PathIds)"),
        LoadedAssets, OutCatalog.Num());
}