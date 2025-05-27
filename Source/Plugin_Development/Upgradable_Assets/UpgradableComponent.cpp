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

void UUpgradableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUpgradableComponent, CurrentLevel);
}

void UUpgradableComponent::OnRep_CurrentLevel()
{
	OnLevelChanged.Broadcast(CurrentLevel - 1, CurrentLevel);

	// UI feedback, VFX, etc.
}

bool UUpgradableComponent::CanUpgrade_Implementation() const
{
	UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
	return Subsystem && Subsystem->CanUpgrade(this);
}

void UUpgradableComponent::RequestUpgrade_Implementation()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_RequestUpgrade_Implementation();
	}
	else
	{
		Server_RequestUpgrade();
	}
}

bool UUpgradableComponent::Server_RequestUpgrade_Validate() { return true; }

void UUpgradableComponent::Server_RequestUpgrade_Implementation()
{
	if (UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>())
	{
		Subsystem->HandleUpgradeRequest(this);
	}
}

void UUpgradableComponent::ApplyUpgradeInternal()
{
	++CurrentLevel;
	OnRep_CurrentLevel();
}
// Called when the game starts
void UUpgradableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


