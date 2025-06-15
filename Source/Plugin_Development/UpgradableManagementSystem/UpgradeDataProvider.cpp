#include "UpgradeDataProvider.h"
#include "UpgradeDataTableProvider.h"
#include "UpgradeDataAssetProvider.h"
#include "UpgradeJsonProvider.h"
#include "UpgradeDefinitionDataAsset.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"

TArray<UUpgradeDataProvider*> UUpgradeDataProvider::Scan(const FString& FolderPath)
{
    TArray<UUpgradeDataProvider*> Providers;

    FString AssetPath = FolderPath;
    if (!AssetPath.StartsWith(TEXT("/Game")))
    {
        AssetPath = FPaths::Combine(TEXT("/Game"), AssetPath);
    }
    ScanForAssets(AssetPath, Providers);
    ScanForFiles(AssetPath, Providers);
    return Providers;
}

void UUpgradeDataProvider::ScanForAssets(const FString& FolderPath, TArray<UUpgradeDataProvider*>& Providers)
{
    // Gather all assets in one pass
    TArray<FAssetData> AssetsInFolder;
    FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    RegistryModule.Get().GetAssetsByPath(FName(*FolderPath), AssetsInFolder, /*bRecursive=*/true);

    // Map asset class path to provider class for easy extension
    const TMap<FTopLevelAssetPath, TSubclassOf<UUpgradeDataProvider>> ClassToProvider = {
        {UDataTable::StaticClass()->GetClassPathName(), UUpgradeDataTableProvider::StaticClass()},
        {UUpgradeDefinitionDataAsset::StaticClass()->GetClassPathName(), UUpgradeDataAssetProvider::StaticClass()}
    };

    // Temporary storage of assets grouped by provider class
    TMap<TSubclassOf<UUpgradeDataProvider>, TArray<FAssetData>> ProviderAssets;
    for (const FAssetData& AssetData : AssetsInFolder)
    {
        if (const TSubclassOf<UUpgradeDataProvider>* ProviderClass = ClassToProvider.Find(AssetData.AssetClassPath))
        {
            ProviderAssets.FindOrAdd(*ProviderClass).Add(AssetData);
        }
    }

    // Instantiate providers and assign detected assets
    for (const auto& Pair : ProviderAssets)
    {
        if (!Pair.Key) continue;
        UUpgradeDataProvider* Provider = NewObject<UUpgradeDataProvider>(this, *Pair.Key);
        Provider->DetectedAssets = Pair.Value;
        Providers.Add(Provider);
    }
}

void UUpgradeDataProvider::ScanForFiles(const FString& FolderPath, TArray<UUpgradeDataProvider*>& Providers)
{
    FString DiskPath = FPackageName::LongPackageNameToFilename(FolderPath);

    // Map file extension to provider class for easy extension
    const TMap<FString, TSubclassOf<UUpgradeDataProvider>> ExtensionToProvider = {
        {TEXT("json"), UUpgradeJsonProvider::StaticClass()}
    };

    // Temporary storage of files grouped by provider class
    TMap<TSubclassOf<UUpgradeDataProvider>, TArray<FString>> ProviderFiles;

    for (const auto& Pair : ExtensionToProvider)
    {
        if (!Pair.Value) continue;

        TArray<FString> FoundFiles;
        const FString Wildcard = FString::Printf(TEXT("*.%s"), *Pair.Key);
        IFileManager::Get().FindFilesRecursive(FoundFiles, *DiskPath, *Wildcard, true, false);

        if (FoundFiles.Num() > 0)
        {
            ProviderFiles.FindOrAdd(Pair.Value).Append(FoundFiles);
        }
    }

    // Instantiate providers and assign detected files
    for (const auto& Pair : ProviderFiles)
    {
        if (!Pair.Key) continue;
        UUpgradeDataProvider* Provider = NewObject<UUpgradeDataProvider>(this, *Pair.Key);
        Provider->DetectedFiles = Pair.Value;
        Providers.Add(Provider);
    }
}

int32 UUpgradeDataProvider::AddOrFindRequiredResourceTypeIndex(const FName& ResourceType, TArray<FName>& ResourceTypes)
{
    int32 FoundIndex = ResourceTypes.IndexOfByKey(ResourceType);
    
    if (FoundIndex != INDEX_NONE)   return FoundIndex;
    
    int32 NewIndex = ResourceTypes.Num();
    ResourceTypes.Add(ResourceType);
    
    return NewIndex;
}
