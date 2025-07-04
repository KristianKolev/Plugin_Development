// Fill out your copyright notice in the Description page of Project Settings.


#include "MightyraiderFunctionLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"

FString UMightyraiderFunctionLibrary::GetProjectVersion()
{
	FString AppVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		AppVersion,
		GGameIni
	);
	return AppVersion;
}

TArray<FAssetData> UMightyraiderFunctionLibrary::GetAssetsInFolder(const FString& FolderPath)
{
	// Gather all assets in one pass
	TArray<FAssetData> AssetsInFolder;
	FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	RegistryModule.Get().GetAssetsByPath(FName(*FolderPath), AssetsInFolder, /*bRecursive=*/true);
	UE_LOG(LogTemp, Log, TEXT("[ASSETSCAN_INFO_01] Found %d asset(s) in '%s'"), AssetsInFolder.Num(), *FolderPath);

	return AssetsInFolder;
}

TArray<FString> UMightyraiderFunctionLibrary::GetFilesInFolder(const FString& FolderPath, const FString& FileExtension)
{
	FString DiskPath = FPackageName::LongPackageNameToFilename(FolderPath);
	TArray<FString> FoundFiles;
	const FString Wildcard = FString::Printf(TEXT("*.%s"), *FileExtension);
	IFileManager::Get().FindFilesRecursive(FoundFiles, *DiskPath, *Wildcard, true, false);
	UE_LOG(LogTemp, Log, TEXT("[FILESCAN_INFO_01] Found %d '%s' file(s)"), FoundFiles.Num(), *FileExtension);

	return FoundFiles;
}

