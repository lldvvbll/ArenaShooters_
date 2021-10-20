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

void AASPlayerController::OnChangedInnerMatchState(EInnerMatchState State)
{
	ENetMode NetMode = GetNetMode();
	if (NetMode == NM_Client)
	{
		if (State == EInnerMatchState::Finish)
		{
			auto Char = GetPawn<AASCharacter>();
			if (IsValid(Char))
			{
				Char->DisableInput(this);
			}

			if (HudWidget != nullptr)
			{
				HudWidget->StopFinishTimer();
			}
		}
	}
	else if (NetMode == NM_DedicatedServer)
	{
		if (State == EInnerMatchState::Finish)
		{
			auto Char = GetPawn<AASCharacter>();
			if (IsValid(Char))
			{
				Char->SetCanBeDamaged(false);
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
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

		auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
		if (IsValid(GameState))
		{
			if (GameState->GetInnerMatchState() == EInnerMatchState::Prepare)
			{
				PrepareInfoWidget = CreateWidget<UASPrepareInfoUserWidget>(this, PrepareInfoWidgetClass);
				if (PrepareInfoWidget != nullptr)
				{
					PrepareInfoWidget->AddToViewport(1);
				}
			}
		}
	}
}

void AASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("ShowInventory"), EInputEvent::IE_Pressed, this, &AASPlayerController::ShowInventoryWidget);
	InputComponent->BindAction(TEXT("ShowGameMenu"), EInputEvent::IE_Pressed, this, &AASPlayerController::ShowGameMenuWidget);
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

void AASPlayerController::ShowInventoryWidget()
{
	ShowFullScreenWidget<UASInventoryUserWidget>(InventoryWidgetClass);
}

void AASPlayerController::ShowGameMenuWidget()
{
	ShowFullScreenWidget<UASGameMenuUserWidget>(GameMenuWidgetClass);
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
	}
	else
	{
		AS_LOG_S(Error);
	}

	ChangeInputMode(true);
	ShowCrossHair(true);
}
