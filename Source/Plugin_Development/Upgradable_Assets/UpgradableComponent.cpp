// Fill out your copyright notice in the Description page of Project Settings.

// Sets default values for this component's properties
#include "UpgradableComponent.h"
#include "Net/UnrealNetwork.h"
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
	UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
	if (Subsystem)
	{
		Subsystem->UnregisterUpgradableComponent(UpgradableID);
	}
	Super::EndPlay(EndPlayReason);
}

void UUpgradableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUpgradableComponent, LocalLevel);
}

bool UUpgradableComponent::CanUpgrade_Implementation() const
{
	UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
	return Subsystem && Subsystem->CanUpgrade(GetComponentId());
}

void UUpgradableComponent::Client_SetLevel_Implementation(int32 NewLevel)
{
	const int32 OldLevel = LocalLevel;
	LocalLevel = NewLevel;
	OnLevelChanged.Broadcast(OldLevel, LocalLevel);
}

void UUpgradableComponent::RequestUpgrade_Implementation(int32 LevelIncrease)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_RequestUpgrade_Implementation(LevelIncrease);
	}
	else
	{
		Server_RequestUpgrade(LevelIncrease);
	}
}

bool UUpgradableComponent::Server_RequestUpgrade_Validate(int32 LevelIncrease) { return true; }

void UUpgradableComponent::Server_RequestUpgrade_Implementation(int32 LevelIncrease)
{
	if (UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>())
	{
		Subsystem->HandleUpgradeRequest(UpgradableID, LevelIncrease);
	}
}
