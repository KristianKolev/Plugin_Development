#pragma once

#include "CoreMinimal.h"
#include "UpgradableComponent.h"
#include "UObject/Interface.h"
#include "UpgradeDataContainers.h"
#include "Upgradable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UUpgradable : public UInterface
{
	GENERATED_BODY()
};

 /*
  * This Interface would be used on an Actor that can be upgraded.
  * It should allow easy and cheap lookups of the upgrade data
  * and access to the upgrade component and system.
  * It requires on per actor implementation by the user.
  */
class PLUGIN_DEVELOPMENT_API IUpgradable
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgrade System")
	int32 GetCurrentUpgradeLevel() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgrade System")
	bool CanUpgrade() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgrade System")
	void RequestUpgrade(int32 LevelIncrease);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgrade System")
	EUpgradableCategory GetUpgradableCategory() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgrade System")
	EUpgradableAspect GetUpgradableAspect() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgrade System")
	int32 GetComponentId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgrade System")
	UUpgradableComponent* GetUpgradableComponent(AActor* TargetActor) const;

	
};
