#include "ResourceSystemComponent.h"
#include "ResourceManagerSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"

UResourceSystemComponent::UResourceSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UResourceSystemComponent::Client_UpdateResource_Implementation(FName ResourceName, int32 NewAmount, int32 DeltaAmount)
{
    LocalResources.FindOrAdd(ResourceName) = NewAmount;
    OnResourceChanged.Broadcast(ResourceName, NewAmount, DeltaAmount);
}

void UResourceSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    //OnResourceChanged.AddDynamic(this, &UResourceSystemComponent::HandleResourceChanged);
    // Only server side registration
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        if (UResourceManagerSubsystem* Sub = GetResourceSubsystem())
        {
            Sub->RegisterComponent(this);
        }
    }
}

void UResourceSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        if (UResourceManagerSubsystem* Sub = GetResourceSubsystem())
        {
            Sub->UnregisterComponent(this);
        }
    }
    Super::EndPlay(EndPlayReason);
}

UResourceManagerSubsystem* UResourceSystemComponent::GetResourceSubsystem() const
{
    return GetWorld() ? GetWorld()->GetSubsystem<UResourceManagerSubsystem>() : nullptr;
}

void UResourceSystemComponent::AddResource(FName ResourceName, int32 Amount)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        GetResourceSubsystem()->AddResource(this, ResourceName, Amount);
    }
    else
    {
        Server_AddResource(ResourceName, Amount);
    }
}

void UResourceSystemComponent::SpendResource(FName ResourceName, int32 Amount)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        GetResourceSubsystem()->SpendResource(this, ResourceName, Amount);
    }
    else
    {
        Server_SpendResource(ResourceName, Amount);
    }
}

int32 UResourceSystemComponent::GetResource(FName ResourceName) const
{
    if (const int32* Val = LocalResources.Find(ResourceName))
    {
        return *Val;
    }
    return -1;
}

void UResourceSystemComponent::GetAllResources(TMap<FName, int32>& OutAvailableResources) const
{
    OutAvailableResources = LocalResources;
}

void UResourceSystemComponent::Server_AddResource_Implementation(FName ResourceName, int32 Amount)
{
    GetResourceSubsystem()->AddResource(this, ResourceName, Amount);
}

void UResourceSystemComponent::Server_SpendResource_Implementation(FName ResourceName, int32 Amount)
{
    GetResourceSubsystem()->SpendResource(this, ResourceName, Amount);
}

void UResourceSystemComponent::HandleResourceChanged_Implementation(FName ResourceName, int32 NewAmount, int32 AmountChange)
{
    LocalResources.FindOrAdd(ResourceName) = NewAmount;
}
