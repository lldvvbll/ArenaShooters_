// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/PlayerController.h"
#include "ASPlayerController.generated.h"

class UASCrossHairUserWidget;
class UASWeapon;
class UASInventoryUserWidget;
class UASGameMenuUserWidget;
class UASPrepareInfoUserWidget;
class UASHudUserWidget;
class AASCharacter;

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
	void ToggleShowInventoryWidget();
	void ToggleShowGameMenuWidget();

	template <typename TWidget>
	void ToggleFullScreenWidget(const TSubclassOf<TWidget>& WidgetClass)
	{
		if (CurrentFullScreenWidget == nullptr)
		{
			auto FullScreenWidget = CreateWidget<TWidget>(this, WidgetClass);
			if (FullScreenWidget != nullptr)
			{
				FullScreenWidget->OnConstructed.AddUObject(this, &AASPlayerController::OnConstructedFullScreenWidget);
				FullScreenWidget->OnDestructed.AddUObject(this, &AASPlayerController::OnDestructedFullScreenWidget);

				if (auto ASChar = Cast<AASCharacter>(GetCharacter()))
				{
					FullScreenWidget->OnConstructed.AddUObject(ASChar, &AASCharacter::OnConstructedFullScreenWidget);
					FullScreenWidget->OnDestructed.AddUObject(ASChar, &AASCharacter::OnDestructedFullScreenWidget);
				}

				FullScreenWidget->AddToViewport(1);

				CurrentFullScreenWidget = FullScreenWidget;
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
	}

	void OnConstructedFullScreenWidget(UUserWidget* ConstructedWidget);
	void OnDestructedFullScreenWidget(UUserWidget* DestructedWidget);

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASCrossHairUserWidget> CrossHairClass;

	UPROPERTY()
	UASCrossHairUserWidget* CrossHair;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASInventoryUserWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASGameMenuUserWidget> GameMenuWidgetClass;

	UPROPERTY()
	UUserWidget* CurrentFullScreenWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASPrepareInfoUserWidget> PrepareInfoWidgetClass;

	UPROPERTY()
	UASPrepareInfoUserWidget* PrepareInfoWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASHudUserWidget> HudWidgetClass;

	UPROPERTY()
	UASHudUserWidget* HudWidget;

	FInputModeGameOnly GameInputMode;
	FInputModeGameAndUI UIInputMode;
};
