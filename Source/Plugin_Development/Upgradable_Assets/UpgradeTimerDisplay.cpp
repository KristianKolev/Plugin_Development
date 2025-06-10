// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeTimerDisplay.h"


void UUpgradeTimerDisplay::NativeConstruct()
{
	Super::NativeConstruct();
	if (TrackedComponent)
	{
		TrackedComponent->OnUpgradeStarted.AddDynamic(this, &UUpgradeTimerDisplay::HandleUpgradeStarted);
		TrackedComponent->OnUpgradeCanceled.AddDynamic(this, &UUpgradeTimerDisplay::HandleUpgradeCanceled);
		TrackedComponent->OnLevelChanged.AddDynamic(this, &UUpgradeTimerDisplay::HandleLevelChanged);
		TrackedComponent->OnTimeToUpgradeChanged.AddDynamic(this, &UUpgradeTimerDisplay::HandleTimeToUpgradeChanged);
		
	}
}

void UUpgradeTimerDisplay::HandleUpgradeStarted_Implementation(float SecondsUntilCompleted)
{
	CountdownText->SetText(FText::AsNumber(SecondsUntilCompleted));
	if (CountdownHandle.IsValid())
	{
		StopCountdownTimer();
		RemainingTime = SecondsUntilCompleted;
	}
	else
	{
		TotalTime = SecondsUntilCompleted;
		RemainingTime = SecondsUntilCompleted;
	}
	StartCountdownTimer();
}

void UUpgradeTimerDisplay::HandleUpgradeCanceled_Implementation(int32 CurrentLevel)
{
	StopCountdownTimer();
	TotalTime = -1.f;
	RemainingTime = -1.f;
	LevelText->SetText(FText::AsNumber(CurrentLevel));
}

void UUpgradeTimerDisplay::HandleTimeToUpgradeChanged_Implementation(float DeltaTime)
{
	const int32 Sign = FMath::Sign(DeltaTime);
	const TCHAR SignChar = (Sign >= 0 ? TEXT('+') : TEXT('-'));
	
	const float AbsDelta = FMath::Abs(DeltaTime);
	
	const FString Formatted = FString::Printf(TEXT("%c%.0f"), SignChar, AbsDelta);
	
	CountdownChangedText->SetText(FText::FromString(Formatted));
	RemainingTime += DeltaTime;

	
	if (RemainingTime <= 0.f)
	{
		CountdownText->SetText(FText::AsNumber(0.f));
		UpgradeProgress->SetPercent(1.f);
		
	}
	else
	{
		const float ElapsedTime = TotalTime - RemainingTime; 
		UpgradeProgress->SetPercent(ElapsedTime / TotalTime);
	}
}

void UUpgradeTimerDisplay::HandleLevelChanged_Implementation(int32 OldLevel, int32 NewLevel)
{
	StopCountdownTimer();
	TotalTime = -1.f;
	RemainingTime = -1.f;
	LevelText->SetText(FText::AsNumber(NewLevel));
}

void UUpgradeTimerDisplay::StartCountdownTimer()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("UpdateCountdownText"));
	if (RemainingTime > 10.f)
	{
		GetWorld()->GetTimerManager().SetTimer(CountdownHandle, TimerDelegate, 1.f, true);
		CountdownTimerRate = 1.f;
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(CountdownHandle, TimerDelegate, 0.1f, true);
		CountdownTimerRate = 0.1f;
	}
	
}

void UUpgradeTimerDisplay::UpdateCountdownText()
{

	RemainingTime -= CountdownTimerRate;
	
	CountdownText->SetText(FText::AsNumber(RemainingTime));
	const float ElapsedTime = TotalTime - RemainingTime; 
	UpgradeProgress->SetPercent(ElapsedTime / TotalTime);
	
	if (RemainingTime < 10.f && (CountdownTimerRate - 0.1f > KINDA_SMALL_NUMBER) )
	{
		StopCountdownTimer();
		StartCountdownTimer();
	}
}

void UUpgradeTimerDisplay::StopCountdownTimer()
{
	if (CountdownHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownHandle);
	}
}


