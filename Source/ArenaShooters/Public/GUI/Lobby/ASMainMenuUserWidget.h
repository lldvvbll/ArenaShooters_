// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASMainMenuUserWidget.generated.h"

class UButton;
class UASServerBrowserUserWidget;

UCLASS()
class ARENASHOOTERS_API UASMainMenuUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedFindServerButton();

	UFUNCTION()
	void OnClickedExitButton();

protected:
	UPROPERTY()
	UButton* FindServerButton;

	UPROPERTY()
	UButton* ExitButton;
};
