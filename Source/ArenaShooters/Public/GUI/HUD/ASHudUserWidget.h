// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASHudUserWidget.generated.h"

class UASInventoryStatusUserWidget;
class UProgressBar;

UCLASS()
class ARENASHOOTERS_API UASHudUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	void OnChangedCharacterHealth(float NewHealth) const;

protected:
	UPROPERTY()
	UASInventoryStatusUserWidget* InventoryStatusWidget;

	UPROPERTY()
	UProgressBar* HealthProgressBar;

	float MaxCharHealth;
};
