// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/PlayerController.h"
#include "ASPlayerController.generated.h"

class UASCrossHairUserWidget;
class UASWeapon;
class UASInventoryUserWidget;

UCLASS()
class ARENASHOOTERS_API AASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AASPlayerController();

	virtual void SetPawn(APawn* InPawn) override;

	void ChangeInputMode(bool bGameMode);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void OnScope(const TWeakObjectPtr<UASWeapon>& UsingWeapon);
	void OnUnscope();

	void ShowCrossHair(bool bShow);
	void ShowInventoryWidget();

private:
	UPROPERTY(EditDefaultsOnly, Category = UI, Meta = (AllowPrivateAccess = true))
	TSubclassOf<UASCrossHairUserWidget> CrossHairClass;

	UPROPERTY()
	UASCrossHairUserWidget* CrossHair;

	UPROPERTY(EditDefaultsOnly, Category = UI, Meta = (AllowPrivateAccess = true))
	TSubclassOf<UASInventoryUserWidget> InventoryWidgetClass;

	UPROPERTY()
	UASInventoryUserWidget* InventoryWidget;

	FInputModeGameOnly GameInputMode;
	FInputModeGameAndUI UIInputMode;
};
