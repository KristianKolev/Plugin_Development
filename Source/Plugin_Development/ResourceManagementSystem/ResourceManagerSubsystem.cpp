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
	
	FAssetRegistryModule& Arm = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AR = Arm.Get();

	// This returns the FTopLevelAssetPath that AssetRegistry expects
	FTopLevelAssetPath ClassPath = UResourceDefinition::StaticClass()->GetClassPathName();

	TArray<FAssetData> AssetDatas;
	AR.GetAssetsByClass(ClassPath, AssetDatas, /*bSearchSubClasses=*/false);

	for (auto& AD : AssetDatas)
	{
		if (UResourceDefinition* Def = Cast<UResourceDefinition>(AD.GetAsset()))
		{
			Definitions.Add(Def->ResourceName, Def);
		}
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
	}
}

void UResourceManagerSubsystem::UnregisterComponent(UResourceSystemComponent* Comp)
{
	if (Comp && GetWorld()->GetAuthGameMode())
	{
		ComponentResourceMap.Remove(Comp);
	}
}


void UResourceManagerSubsystem::AddResource(UResourceSystemComponent* ResourceComponent, FName ResourceName, int32 Amount)
{
	if (!GetWorld()->GetAuthGameMode() || !ResourceComponent || Amount <= 0) return;

	FResourceBucket& Bucket = ComponentResourceMap.FindOrAdd(ResourceComponent);
	int32& CurrentAmount = Bucket.Resources.FindOrAdd(ResourceName);
	CurrentAmount += Amount;

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
	if (!Bucket)	return false;

	int32* CurrentAmount = Bucket->Resources.Find(ResourceName);
	if (!CurrentAmount || *CurrentAmount < Amount) return false;

	*CurrentAmount -= Amount;
	
	//ResourceComponent->OnResourceChanged.Broadcast(ResourceName, *CurrentAmount, (Amount * -1));
	ResourceComponent->Client_UpdateResource(ResourceName, *CurrentAmount, (Amount * -1));
	return true;
}

UResourceDefinition* UResourceManagerSubsystem::GetDefinition(FName ResourceName) const
{
	return *Definitions.Find(ResourceName);
}
