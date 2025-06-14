// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceManagerSubsystem.h"

#include "ResourceDefinition.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/GameModeBase.h"

UResourceManagerSubsystem::UResourceManagerSubsystem()
{
}

void UResourceManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ComponentResourceMap.Empty();
	Definitions.Empty();
	
        const FString ScanPath = TEXT("/Game/Data/Resources");
        UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_01] Scanning folder %s for any assets"), *ScanPath);

	// Query AssetRegistry for all assets under that path
	FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetsInFolder;
        RegistryModule.Get().GetAssetsByPath(FName(*ScanPath), AssetsInFolder, /*bRecursive=*/true);
        UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_02] Found %d assets under %s"), AssetsInFolder.Num(), *ScanPath);
    
	if (AssetsInFolder.Num() == 0)
	{
                UE_LOG(LogTemp, Warning, TEXT("[RESOURCEMGR_ERR_00] No assets found in path: '%s'"), *ScanPath);
		return;
	}
	
	// 3) Try casting each one to UResourceDefinition
	for (const FAssetData& AssetData : AssetsInFolder)
	{
		if (AssetData.AssetClassPath != UResourceDefinition::StaticClass()->GetClassPathName())
			continue;
		
		UResourceDefinition* Asset = Cast<UResourceDefinition>(AssetData.GetAsset());
		if (!Asset)
		{
                UE_LOG(LogTemp, Warning, TEXT("[RESOURCEMGR_ERR_01] Failed to load UResourceDefinition '%s'"),
                        *AssetData.ObjectPath.ToString());
			continue;
		}
		
		if (Definitions.Contains(Asset->ResourceName))
		{
                        UE_LOG(LogTemp, Warning, TEXT("[RESOURCEMGR_ERR_02] Duplicate ResourceName '%s' in %s"),
                                *Asset->ResourceName.ToString(), *AssetData.ObjectPath.ToString());
                }
                Definitions.Add(Asset->ResourceName, Asset);
                UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_03] Registered Definition '%s' (Name: %s)"),
                        *AssetData.AssetName.ToString(), *Asset->ResourceName.ToString());
		
	}

        if (Definitions.Num() == 0)
        {
                UE_LOG(LogTemp, Warning, TEXT("[RESOURCEMGR_ERR_03] No UResourceDefinition assets found after scanning %s!"),
                        *ScanPath);
        }
        else
        {
                UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_04] Registered %d resource definitions from '%s'"),
                        Definitions.Num(), *ScanPath);
        }
}

void UResourceManagerSubsystem::Deinitialize()
{
	ComponentResourceMap.Empty();
	Definitions.Empty();
	Super::Deinitialize();
}

void UResourceManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

void UResourceManagerSubsystem::RegisterComponent(UResourceSystemComponent* Comp)
{
        // Only register on server-authoritative side
        if (Comp && GetWorld()->GetAuthGameMode())
        {
                ComponentResourceMap.FindOrAdd(Comp);
                UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_05] Registered component %s"), *Comp->GetName());
        }
}

void UResourceManagerSubsystem::UnregisterComponent(UResourceSystemComponent* Comp)
{
        if (Comp && GetWorld()->GetAuthGameMode())
        {
                ComponentResourceMap.Remove(Comp);
                UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_06] Unregistered component %s"), *Comp->GetName());
        }
}


void UResourceManagerSubsystem::AddResource(UResourceSystemComponent* ResourceComponent, FName ResourceName, int32 Amount)
{
        if (!GetWorld()->GetAuthGameMode() || !ResourceComponent || Amount <= 0) return;

        FResourceBucket& Bucket = ComponentResourceMap.FindOrAdd(ResourceComponent);
        int32& CurrentAmount = Bucket.Resources.FindOrAdd(ResourceName);
        const int32 OldAmount = CurrentAmount;
        CurrentAmount += Amount;
        UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_07] Added %d of '%s' to %s (old: %d, new: %d, diff: +%d)"),
                Amount, *ResourceName.ToString(), *ResourceComponent->GetName(), OldAmount, CurrentAmount, Amount);

	//ResourceComponent->OnResourceChanged.Broadcast(ResourceName, CurrentAmount, Amount);
	ResourceComponent->Client_UpdateResource(ResourceName, CurrentAmount, Amount);
}

int32 UResourceManagerSubsystem::GetResource(const UResourceSystemComponent* ResourceComponent, FName ResourceName) const
{
	if (!ResourceComponent)	return -1;
	
	if (const FResourceBucket* Bucket = ComponentResourceMap.Find(ResourceComponent))
	{
		if (const int32* Amount = Bucket->Resources.Find(ResourceName))
		{
			return *Amount;
		}
	}
	return -1;
}

void UResourceManagerSubsystem::GetAllResources(const UResourceSystemComponent* Comp, TMap<FName, int32>& OutAvailableResources) const
{
	if (!Comp) return;
	if (ComponentResourceMap.Contains(Comp))
	{
		OutAvailableResources = ComponentResourceMap.Find(Comp)->Resources;
	}
}

bool UResourceManagerSubsystem::SpendResource(UResourceSystemComponent* ResourceComponent, FName ResourceName, int32 Amount)
{
    if (!GetWorld()->GetAuthGameMode() || !ResourceComponent || Amount <= 0) return false;

    FResourceBucket* Bucket = ComponentResourceMap.Find(ResourceComponent);
    if (!Bucket)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RESOURCEMGR_ERR_04] Component %s has no resource bucket"), *ResourceComponent->GetName());
        return false;
    }

    int32* CurrentAmount = Bucket->Resources.Find(ResourceName);
    if (!CurrentAmount)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RESOURCEMGR_ERR_05] Resource '%s' not found for component %s"), *ResourceName.ToString(), *ResourceComponent->GetName());
        return false;
    }
    if (*CurrentAmount < Amount)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RESOURCEMGR_ERR_06] Not enough '%s' for component %s (have: %d, need: %d)"), *ResourceName.ToString(), *ResourceComponent->GetName(), *CurrentAmount, Amount);
        return false;
    }

    const int32 OldAmount = *CurrentAmount;
    *CurrentAmount -= Amount;
    UE_LOG(LogTemp, Log, TEXT("[RESOURCEMGR_INFO_08] Spent %d of '%s' from %s (old: %d, new: %d, diff: -%d)"), Amount, *ResourceName.ToString(), *ResourceComponent->GetName(), OldAmount, *CurrentAmount, Amount);

    //ResourceComponent->OnResourceChanged.Broadcast(ResourceName, *CurrentAmount, (Amount * -1));
    ResourceComponent->Client_UpdateResource(ResourceName, *CurrentAmount, (Amount * -1));
    return true;
}

UResourceDefinition* UResourceManagerSubsystem::GetDefinition(FName ResourceName) const
{
	if (Definitions.Contains(ResourceName))
	{
		return Definitions[ResourceName];
	}
	return nullptr;
	
}
