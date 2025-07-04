// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnerManagementSubsystem.h"

USpawnerManagementSubsystem::USpawnerManagementSubsystem()
{
}

void USpawnerManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USpawnerManagementSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USpawnerManagementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}
