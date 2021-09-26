// Fill out your copyright notice in the Description page of Project Settings.


#include "ASPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Character/ASCharacter.h"
#include "Item/ASWeapon.h"
#include "ItemActor/ASWeaponActor.h"
#include "GUI/ASInventoryUserWidget.h"
#include "GUI/ASCrossHairUserWidget.h"

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
	}
}

void AASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("ShowInventory"), EInputEvent::IE_Pressed, this, &AASPlayerController::ShowInventoryWidget);
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
	if (InventoryWidget == nullptr)
	{
		InventoryWidget = CreateWidget<UASInventoryUserWidget>(this, InventoryWidgetClass);
		if (InventoryWidget != nullptr)
		{
			InventoryWidget->Bind();
			InventoryWidget->AddToViewport(1);
			ChangeInputMode(false);
		}

		ShowCrossHair(false);
	}
	else
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
		ChangeInputMode(true);

		ShowCrossHair(true);
	}
}
