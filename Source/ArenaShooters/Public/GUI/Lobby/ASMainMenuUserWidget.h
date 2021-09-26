// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASMainMenuUserWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UASServerBrowserUserWidget;

UCLASS()
class ARENASHOOTERS_API UASMainMenuUserWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	enum EWidgetIndex
	{
		MainMenu = 0,
		FindServer,
		KeySettings,
	};

public:
	void ShowMainMenu();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedFindServerButton();

	UFUNCTION()
	void OnClickedKeySettingsButton();

	UFUNCTION()
	void OnClickedKeySettingsBackButton();

	UFUNCTION()
	void OnClickedExitButton();
	
	UFUNCTION()
	void OnEscKeyPressed();

protected:
	UPROPERTY()
	UWidgetSwitcher* MainMenuWidgetSwitcher;

	UPROPERTY()
	UButton* FindServerButton;

	UPROPERTY()
	UASServerBrowserUserWidget* ServerBrowserWidget;

	UPROPERTY()
	UButton* KeySettingsButton;

	UPROPERTY()
	UButton* KeySettingsBackButton;

	UPROPERTY()
	UButton* ExitButton;

	bool bFirstFindServer;
};
