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

    int32 LoadedTables = 0;
    for (const FAssetData& AssetData : AssetsInFolder)
    {
        if (AssetData.AssetClassPath != UDataTable::StaticClass()->GetClassPathName())
            continue;

        UDataTable* Table = Cast<UDataTable>(AssetData.GetAsset());
        if (!Table)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpgradeDataTableProvider: Failed to load DataTable '%s'"), *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedTables;

        FName PathId = FName(*Table->GetName());
        TArray<FUpgradeDefinition>& LevelArray = OutCatalog.FindOrAdd(PathId);

        for (auto& Pair : Table->GetRowMap())
        {
            FUpgradeDefinitionAsset* LevelAsset = reinterpret_cast<FUpgradeDefinitionAsset*>(Pair.Value);
            if (!LevelAsset)
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid row '%s' in DataTable %s"), *Pair.Key.ToString(), *Table->GetName());
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
        }
    }
    UE_LOG(LogTemp, Log, TEXT("UpgradeDataTableProvider: Loaded %d DataTables from '%s' (found %d PathIds)"),
           LoadedTables, *FolderPath, OutCatalog.Num());
}
