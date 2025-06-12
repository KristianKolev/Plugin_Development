// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceManagerSubsystem.h"
#include "GameFramework/PlayerState.h"

UResourceManagerSubsystem::UResourceManagerSubsystem()
{
}

void UResourceManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UResourceManagerSubsystem::Deinitialize()
{
	PlayerResourceMap.Empty();
	Super::Deinitialize();
}

void UResourceManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

void UResourceManagerSubsystem::AddResource(UResourceComponent* ResourceComponent, FName ResourceName, int32 Amount)
{
	if (!ResourceComponent || Amount <= 0) return;

	FResourceBucket& Bucket = PlayerResourceMap.FindOrAdd(ResourceComponent);
	int32& CurrentAmount = Bucket.Resources.FindOrAdd(ResourceName);
	CurrentAmount += Amount;

	ResourceComponent->OnResourceChanged.Broadcast(ResourceName, CurrentAmount, Amount);
}

int32 UResourceManagerSubsystem::GetResource(const UResourceComponent* ResourceComponent, FName ResourceName) const
{
	if (!ResourceComponent)	return -1;
	
	if (const FResourceBucket* Bucket = PlayerResourceMap.Find(ResourceComponent))
	{
		if (const int32* Amount = Bucket->Resources.Find(ResourceName))
		{
			return *Amount;
		}
	}
	return -1;
}

bool UResourceManagerSubsystem::SpendResource(UResourceComponent* ResourceComponent, FName ResourceName, int32 Amount)
{
	if (!ResourceComponent || Amount <= 0) return false;

	FResourceBucket* Bucket = PlayerResourceMap.Find(ResourceComponent);
	if (!Bucket)	return false;

	int32* CurrentAmount = Bucket->Resources.Find(ResourceName);
	if (!CurrentAmount || *CurrentAmount < Amount) return false;

	*CurrentAmount -= Amount;
	
	ResourceComponent->OnResourceChanged.Broadcast(ResourceName, *CurrentAmount, (Amount * -1));
	return true;
}