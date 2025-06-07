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
 
    int32 LoadedAssets = 0;
    for (const FAssetData& AssetData : AssetsInFolder)
    {
        if (AssetData.AssetClassPath != UUpgradeDefinitionDataAsset::StaticClass()->GetClassPathName())
            continue;

        UUpgradeDefinitionDataAsset* Asset = Cast<UUpgradeDefinitionDataAsset>(AssetData.GetAsset());
        if (!Asset)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpgradeDataAssetProvider: Failed to load UpgradeDefinitionDataAsset '%s'"), 
                *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedAssets;
        // Fallback to asset name if no UpgradePathId is set
        FName PathId = !Asset->UpgradePathId.IsNone() ? Asset->UpgradePathId : AssetData.AssetName;
        TArray<FUpgradeDefinition>& LevelDataArray = OutCatalog.FindOrAdd(PathId);

        for (const FUpgradeDefinitionAsset& LevelAsset : Asset->Levels)
        {

            FUpgradeDefinition LevelData;
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
        LoadedAssets, *FolderPath, OutCatalog.Num());
}