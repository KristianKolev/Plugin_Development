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
	CurrentLevel = 0;
}

void UUpgradableComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register this component with the subsystem
	UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
	if (Subsystem)
	{
		UpgradableID = Subsystem->RegisterUpgradableComponent(this);
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
	DOREPLIFETIME(UUpgradableComponent, CurrentLevel);
}

void UUpgradableComponent::OnRep_CurrentLevel(int32 OldLevel)
{
    OnLevelChanged.Broadcast(OldLevel, CurrentLevel);
    // UI feedback, VFX, etc.
}

bool UUpgradableComponent::CanUpgrade_Implementation() const
{
	UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
	return Subsystem && Subsystem->CanUpgrade(this);
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

void UUpgradableComponent::ApplyUpgradeInternal(int32 NewLevel)
{
    const int32 OldLevel = CurrentLevel;
    CurrentLevel = NewLevel;
    OnRep_CurrentLevel(OldLevel);
}
// Called when the game starts