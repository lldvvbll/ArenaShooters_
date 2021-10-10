// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Character/ASCharacter.h"
#include "Item/ASWeapon.h"
#include "ItemActor/ASWeaponActor.h"
#include "GUI/Inventory/ASInventoryUserWidget.h"
#include "GUI/ASCrossHairUserWidget.h"
#include "GUI/ASGameMenuUserWidget.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "GUI/ASPrepareInfoUserWidget.h"
#include "GUI/HUD/ASHudUserWidget.h"

AASPlayerController::AASPlayerController()
{
	UIInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	CurrentFullScreenWidgetType = EFullScreenWidgetType::None;
}

void AASPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (auto ASChar = Cast<AASCharacter>(InPawn))
	{
		ASChar->OnScope.AddUObject(this, &AASPlayerController::OnScope);
		ASChar->OnUnscope.AddUObject(this, &AASPlayerController::OnUnscope);
	}
}

void AASPlayerController::ChangeInputMode(bool bGameMode)
{
	if (bGameMode)
	{
		SetInputMode(GameInputMode);
		bShowMouseCursor = false;
	}
	else
	{
		SetInputMode(UIInputMode);
		bShowMouseCursor = true;
	}
}

void AASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		CrossHair = CreateWidget<UASCrossHairUserWidget>(this, CrossHairClass);
		if (CrossHair != nullptr)
		{
			CrossHair->AddToViewport();
		}

		HudWidget = CreateWidget<UASHudUserWidget>(this, HudWidgetClass);
		if (HudWidget != nullptr)
		{
			HudWidget->AddToViewport();
		}

		if (auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>())
		{
			if (!GameState->IsMatchProcess())
			{
				PrepareInfoWidget = CreateWidget<UASPrepareInfoUserWidget>(this, PrepareInfoWidgetClass);
				if (PrepareInfoWidget != nullptr)
				{
					GameState->OnStartTimeForProcess.AddUObject(PrepareInfoWidget, &UASPrepareInfoUserWidget::StartCountDown);
					GameState->OnChangedNumPlayers.AddUObject(PrepareInfoWidget, &UASPrepareInfoUserWidget::SetNumPlayers);

					PrepareInfoWidget->SetMaxNumPlayers(GameState->GetMaxNumPlayer());
					PrepareInfoWidget->AddToViewport(1);
				}
			}
		}
	}
}

void AASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("ShowInventory"), EInputEvent::IE_Pressed, this, &AASPlayerController::ToggleShowInventoryWidget);
	InputComponent->BindAction(TEXT("ShowGameMenu"), EInputEvent::IE_Pressed, this, &AASPlayerController::ToggleShowGameMenuWidget);
}

void AASPlayerController::OnScope(const TWeakObjectPtr<UASWeapon>& UsingWeapon)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (UsingWeapon.IsValid())
		{
			if (AASWeaponActor* WeaponActor = UsingWeapon->GetActor().Get())
			{
				SetViewTargetWithBlend(WeaponActor, 0.3f, EViewTargetBlendFunction::VTBlend_EaseOut);
			}
		}
	}	

	if (IsLocalPlayerController())
	{
		ShowCrossHair(false);
	}
}

void AASPlayerController::OnUnscope()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetViewTargetWithBlend(GetCharacter());
	}

	if (IsLocalPlayerController())
	{
		ShowCrossHair(true);
	}
}

void AASPlayerController::ShowCrossHair(bool bShow)
{
	if (CrossHair != nullptr)
	{
		CrossHair->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AASPlayerController::ToggleShowInventoryWidget()
{
	if (CurrentFullScreenWidget != nullptr)
		return;

	if (InventoryWidget == nullptr)
	{
		InventoryWidget = CreateWidget<UASInventoryUserWidget>(this, InventoryWidgetClass);
		if (InventoryWidget != nullptr)
		{
			InventoryWidget->OnConstructed.AddUObject(this, &AASPlayerController::OnConstructedFullScreenWidget);
			InventoryWidget->OnDestructed.AddUObject(this, &AASPlayerController::OnDestructedFullScreenWidget);
			
			if (auto ASChar = Cast<AASCharacter>(GetCharacter()))
			{
				InventoryWidget->OnConstructed.AddUObject(ASChar, &AASCharacter::OnConstructedFullScreenWidget);
				InventoryWidget->OnDestructed.AddUObject(ASChar, &AASCharacter::OnDestructedFullScreenWidget);
			}

			InventoryWidget->AddToViewport(1);

			CurrentFullScreenWidget = InventoryWidget;
		}
	}
	else
	{
		checkNoEntry();
	}
}

void AASPlayerController::ToggleShowGameMenuWidget()
{
	if (CurrentFullScreenWidget != nullptr)
		return;

	if (GameMenuWidget == nullptr)
	{
		GameMenuWidget = CreateWidget<UASGameMenuUserWidget>(this, GameMenuWidgetClass);
		if (GameMenuWidget != nullptr)
		{
			GameMenuWidget->OnConstructed.AddUObject(this, &AASPlayerController::OnConstructedFullScreenWidget);
			GameMenuWidget->OnDestructed.AddUObject(this, &AASPlayerController::OnDestructedFullScreenWidget);
			
			if (auto ASChar = Cast<AASCharacter>(GetCharacter()))
			{
				GameMenuWidget->OnConstructed.AddUObject(ASChar, &AASCharacter::OnConstructedFullScreenWidget);
				GameMenuWidget->OnDestructed.AddUObject(ASChar, &AASCharacter::OnDestructedFullScreenWidget);
			}

			GameMenuWidget->AddToViewport(1);

			CurrentFullScreenWidget = GameMenuWidget;
		}
	}
	else
	{
		checkNoEntry();
	}
}

void AASPlayerController::OnConstructedFullScreenWidget(UUserWidget* ConstructedWidget)
{
	if (CurrentFullScreenWidget == nullptr)
	{
		CurrentFullScreenWidget = ConstructedWidget;
	}
	else
	{
		AS_LOG_S(Error);
	}

	ChangeInputMode(false);
	ShowCrossHair(false);
}

void AASPlayerController::OnDestructedFullScreenWidget(UUserWidget* DestructedWidget)
{
	if (CurrentFullScreenWidget == DestructedWidget)
	{
		CurrentFullScreenWidget = nullptr;

		if (InventoryWidget == DestructedWidget)
		{
			InventoryWidget = nullptr;
		}
		else if (GameMenuWidget == DestructedWidget)
		{
			GameMenuWidget = nullptr;
		}
	}
	else
	{
		AS_LOG_S(Error);
	}

	ChangeInputMode(true);
	ShowCrossHair(true);
}
