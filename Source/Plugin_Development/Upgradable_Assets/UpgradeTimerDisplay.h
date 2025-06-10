// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "UpgradableComponent.h"
#include "UpgradeTimerDisplay.generated.h"


// class UTextBlock;
// class UProgressBar;
// class UUpgradableComponent;
/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeTimerDisplay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade System", meta=(ExposeOnSpawn="true"))
	UUpgradableComponent* TrackedComponent = nullptr;

protected:

	UPROPERTY(meta=(BindWidget))
	UTextBlock* LevelText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CountdownText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CountdownChangedText;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* UpgradeProgress;

	UPROPERTY()
	FTimerHandle CountdownHandle;

	UFUNCTION()
	void UpdateCountdownText();

	UFUNCTION()
	void StartCountdownTimer();
	
	UFUNCTION()
	void StopCountdownTimer();

	UFUNCTION(BlueprintNativeEvent, Category = "Upgrade System|Timer")
	void HandleLevelChanged(int32 OldLevel, int32 NewLevel);

	UFUNCTION(BlueprintNativeEvent, Category = "Upgrade System|Timer")
	void HandleUpgradeStarted(float SecondsUntilCompleted);

	UFUNCTION(BlueprintNativeEvent, Category = "Upgrade System|Timer")
	void HandleUpgradeCanceled(int32 CurrentLevel);

	UFUNCTION(BlueprintNativeEvent, Category = "Upgrade System|Timer")
	void HandleTimeToUpgradeChanged(float DeltaTime);
	
	UPROPERTY()
	float TotalTime = 0.f;
	UPROPERTY()
	float RemainingTime = 0.f;
	UPROPERTY()
	float CountdownTimerRate = 0.f;
};
