// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/PlayerController.h"
#include "ASLobbyPlayerController.generated.h"

class UASMainMenuUserWidget;
class UASServerBrowserUserWidget;
class UASNotificationUserWidget;

UCLASS()
class ARENASHOOTERS_API AASLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void ShowMainMenu();
	void NotifyMessage(const FString& Message, float Duration = 5.0f);

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASMainMenuUserWidget> MainMenuWidgetClass;

	UPROPERTY()
	UASMainMenuUserWidget* MainMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASNotificationUserWidget> NotificationWidgetClass;
};
