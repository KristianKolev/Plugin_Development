// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MightyraiderFunctionLibrary.generated.h"

/**
 * A collection of functions that I find useful and need to use in many projects.
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UMightyraiderFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	/* Display the Project version number */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Config")
	static FString GetProjectVersion();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Asset Access")
	static TArray<FAssetData> GetAssetsInFolder (const FString& FolderPath);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Asset Access")
	static TArray<FString> GetFilesInFolder (const FString& FolderPath, const FString& FileExtension);
};
