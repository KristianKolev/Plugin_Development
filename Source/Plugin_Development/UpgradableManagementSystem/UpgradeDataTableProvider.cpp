#include "UpgradeDataTableProvider.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"

UUpgradeDataTableProvider::UUpgradeDataTableProvider()
{
}

void UUpgradeDataTableProvider::InitializeData(const FString& FolderPath, TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetsInFolder;
    RegistryModule.Get().GetAssetsByPath(FName(*FolderPath), AssetsInFolder, /*bRecursive=*/true);

    // Check if any assets were found
    if (AssetsInFolder.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPGRADETABLE_ERR_00] No assets found in path: '%s'"), *FolderPath);
        return;
    }

    int32 LoadedTables = 0;
    for (const FAssetData& AssetData : AssetsInFolder)
    {
        if (AssetData.AssetClassPath != UDataTable::StaticClass()->GetClassPathName())
            continue;

        UDataTable* Table = Cast<UDataTable>(AssetData.GetAsset());
        if (!Table)
        {
            UE_LOG(LogTemp, Warning, TEXT("[UPGRADETABLE_ERR_01] Failed to load DataTable '%s'"), *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedTables;

        FName PathId = FName(*Table->GetName());
        TArray<FUpgradeDefinition>& LevelArray = OutCatalog.FindOrAdd(PathId);
        
        int32 ProcessedRows = 0;
        for (auto& Pair : Table->GetRowMap())
        {
            FUpgradeDefinitionAsset* LevelAsset = reinterpret_cast<FUpgradeDefinitionAsset*>(Pair.Value);
            if (!LevelAsset)
            {
                UE_LOG(LogTemp, Warning, TEXT("[UPGRADETABLE_ERR_02] Invalid row '%s' in DataTable %s"), *Pair.Key.ToString(), *Table->GetName());
                continue;
            }

            FUpgradeDefinition LevelData;
            for (const auto& ResourcePair : LevelAsset->UpgradeResourceCosts)
            {
                int32 TypeIdx = AddOrFindRequiredResourceTypeIndex(ResourcePair.Key, OutResourceTypes);
                LevelData.ResourceTypeIndices.Add(TypeIdx);
                LevelData.UpgradeCosts.Add(ResourcePair.Value);
            }
            
            LevelData.UpgradeSeconds = LevelAsset->UpgradeSeconds;
            LevelData.bUpgradeLocked = LevelAsset->bUpgradeLocked;
            LevelArray.Add(LevelData);
            ++ProcessedRows;
        }
        UE_LOG(LogTemp, Log, TEXT("[UPGRADETABLE_INFO_01] Successfully processed DataTable '%s' with %d rows"), *Table->GetName(), ProcessedRows);
    }

    // Check if any DataTables were actually loaded
    if (LoadedTables == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPGRADETABLE_ERR_03] No DataTables found in path: '%s'"), *FolderPath);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[UPGRADETABLE_INFO_02] Loaded %d DataTables from '%s' (found %d PathIds)"),
           LoadedTables, *FolderPath, OutCatalog.Num());
}