// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Plugin_DevelopmentPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API APlugin_DevelopmentPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
	int32 NewPS = 1;
};
