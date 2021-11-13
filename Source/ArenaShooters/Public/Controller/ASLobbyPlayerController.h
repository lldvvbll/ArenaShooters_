// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/PlayerController.h"
#include "ASLobbyPlayerController.generated.h"

class UASMainMenuUserWidget;
class UASServerBrowserUserWidget;
class UASTimerCaptionUserWidget;

UCLASS()
class ARENASHOOTERS_API AASLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void ShowMainMenu();

protected:
	virtual void BeginPlay() override;

	void ShowCaption(const FString& Caption, float CaptionLifeTimeSec = 5.0f);

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASMainMenuUserWidget> MainMenuWidgetClass;

	UPROPERTY()
	UASMainMenuUserWidget* MainMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASTimerCaptionUserWidget> TimerCaptionUserWidgetClass;
};
