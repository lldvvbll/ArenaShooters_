// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASGameMenuUserWidget.generated.h"

class UButton;
class UWidgetSwitcher;

UCLASS()
class ARENASHOOTERS_API UASGameMenuUserWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	enum EWidgetIndex
	{
		GameMenu = 0,
		KeySettings,
	};

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnClickedContinueButton();

	UFUNCTION()
	void OnClickedKeySettingsButton();

	UFUNCTION()
	void OnClickedKeySettingsBackButton();

	UFUNCTION()
	void OnClickedGoToMainMenuButton();

	UFUNCTION()
	void OnClickedQuitGameButton();

	UFUNCTION()
	void OnEscKeyPressed();

public:
	DECLARE_EVENT_OneParam(UASGameMenuUserWidget, FOnConstructedEvent, UUserWidget*);
	FOnConstructedEvent OnConstructed;

	DECLARE_EVENT_OneParam(UASGameMenuUserWidget, FOnDestructedEvent, UUserWidget*);
	FOnDestructedEvent OnDestructed;

protected:
	UPROPERTY()
	UWidgetSwitcher* GameMenuWidgetSwitcher;

	UPROPERTY()
	UButton* ContinueButton;

	UPROPERTY()
	UButton* KeySettingsButton;

	UPROPERTY()
	UButton* KeySettingsBackButton;

	UPROPERTY()
	UButton* GoToMainMenuButton;

	UPROPERTY()
	UButton* QuitGameButton;
};
