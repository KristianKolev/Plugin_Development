// Fill out your copyright notice in the Description page of Project Settings.

// Sets default values for this component's properties
#include "UpgradableComponent.h"
//#include "Net/UnrealNetwork.h"
#include "UpgradeManagerSubsystem.h"
#include "GameFramework/Actor.h"

UUpgradableComponent::UUpgradableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	LocalLevel = 0;
}

void UUpgradableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		// Register this component with the subsystem
		UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
		if (Subsystem)
		{
			UpgradableID = Subsystem->RegisterUpgradableComponent(this);
		}
	}
}

void UUpgradableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwnerRole() == ROLE_Authority && UpgradableID != -1)
	{
		UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
		if (Subsystem)
		{
			Subsystem->UnregisterUpgradableComponent(UpgradableID);
		}
	}
	Super::EndPlay(EndPlayReason);
}

// void UUpgradableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// 	DOREPLIFETIME(UUpgradableComponent, LocalLevel);
// }

void UUpgradableComponent::Client_SetLevel_Implementation(int32 NewLevel)
{
	const int32 OldLevel = LocalLevel;
	LocalLevel = NewLevel;
	OnLevelChanged.Broadcast(OldLevel, LocalLevel);
}

void UUpgradableComponent::RequestUpgrade(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts)
{
	Server_RequestUpgrade(LevelIncrease, AvailableResourcesNames, AvailableResourceAmounts);
}

bool UUpgradableComponent::Server_RequestUpgrade_Validate(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts) { return true; }

void UUpgradableComponent::Server_RequestUpgrade_Implementation(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts)
{
	if (UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>())
	{
		TMap<FName, int32> AvailableResources;
		for ( int32 i = 0; i < AvailableResourcesNames.Num(); ++i)
		{			
			AvailableResources.Add(AvailableResourcesNames[i], AvailableResourceAmounts[i]);
		}
		Subsystem->HandleUpgradeRequest(UpgradableID, LevelIncrease, AvailableResources);
	}
}
