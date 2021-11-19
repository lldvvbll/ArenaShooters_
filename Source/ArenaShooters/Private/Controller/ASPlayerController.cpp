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
#include "Net/UnrealNetwork.h"
#include "GUI/ASTimerCaptionUserWidget.h"
#include "GameMapsSettings.h"
#include "ASGameInstance.h"
#include "Controller/ASPlayerState.h"
#include "GUI/ASNotificationUserWidget.h"

AASPlayerController::AASPlayerController()
{
	UIInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
}

void AASPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AASPlayerController, RespawnTime, COND_AutonomousOnly);
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
		switch (State)
		{
		case EInnerMatchState::Prepare:
			{
				RemoveFullScreenWidget();
			}
			break;
		case EInnerMatchState::Process:
			{
				RemoveFullScreenWidget();
				RemovePrepareInfoWidget();
			}
			break;
		case EInnerMatchState::Finish:
			{
				if (HudWidget != nullptr)
				{
					HudWidget->StopFinishTimer();
				}

				RemoveRespawnTimerWidget();
			}
			break;
		default:
			checkNoEntry();
			break;
		}
	}
	else if (NetMode == NM_DedicatedServer)
	{
		switch (State)
		{
		case EInnerMatchState::Prepare:
			// nothing
			break;
		case EInnerMatchState::Process:
			{
				RespawnPlayer();
			}
			break;
		case EInnerMatchState::Finish:
			{
				ClearRespawnTimer();
			}
			break;
		default:
			checkNoEntry();
			break;
		}
	}
}

void AASPlayerController::SetRespawnTimer(FTimespan Delay)
{
	float RespawnDelaySec = Delay.GetTotalSeconds();
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AASPlayerController::OnCalledRespawnTimer, RespawnDelaySec);
	
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		RespawnTime = GameState->GetServerWorldTimeSeconds() + RespawnDelaySec;
	}
}

void AASPlayerController::ClearRespawnTimer()
{
	if (GetWorldTimerManager().IsTimerActive(RespawnTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	}
}

void AASPlayerController::RespawnPlayer()
{
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (IsValid(GameMode))
	{
		GameMode->RestartPlayer(this);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASPlayerController::ShowRestartTimerWidget(float EndTimerSec)
{
	RestartTimerWidget = CreateWidget<UASTimerCaptionUserWidget>(this, TimerCaptionUserWidgetClass);
	if (RestartTimerWidget != nullptr)
	{
		RestartTimerWidget->AddToViewport(5);

		FText Caption = FText::FromString(TEXT("RESTART IN"));
		RestartTimerWidget->SetInfoWithEndTime(Caption, EndTimerSec);
	}
}

void AASPlayerController::RemoveFullScreenWidget()
{
	if (CurrentFullScreenWidget != nullptr)
	{
		CurrentFullScreenWidget->RemoveFromParent();
		CurrentFullScreenWidget = nullptr;
	}
}

void AASPlayerController::GoToMainMenu()
{
	auto GameInstance = GetGameInstance<UASGameInstance>();
	if (IsValid(GameInstance))
	{
		ClientTravel(UGameMapsSettings::GetGameDefaultMap(), ETravelType::TRAVEL_Absolute);

		GameInstance->DestroySession();
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASPlayerController::NotifyMessage(const FString& Message, float Duration/* = 5.0f*/)
{
	auto NotiWidget = CreateWidget<UASNotificationUserWidget>(this, NotificationWidgetClass);
	if (NotiWidget != nullptr)
	{
		NotiWidget->AddToViewport(5);

		NotiWidget->SetInfo(Message, Duration);
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
				ShowPrepareInfoWidget();
			}
		}
		else
		{
			AS_LOG_S(Error);
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
	ShowFullScreenWidget<UASGameMenuUserWidget>(GameMenuWidgetClass, 5);
}

void AASPlayerController::ShowPrepareInfoWidget()
{
	PrepareInfoWidget = CreateWidget<UASPrepareInfoUserWidget>(this, PrepareInfoWidgetClass);
	if (PrepareInfoWidget != nullptr)
	{
		PrepareInfoWidget->AddToViewport(2);
	}
}

void AASPlayerController::RemovePrepareInfoWidget()
{
	if (PrepareInfoWidget != nullptr)
	{
		PrepareInfoWidget->RemoveFromParent();
		PrepareInfoWidget = nullptr;
	}
}

void AASPlayerController::ShowRespawnTimerWidget(float EndTimeSec)
{
	RespawnTimerWidget = CreateWidget<UASTimerCaptionUserWidget>(this, TimerCaptionUserWidgetClass);
	if (RespawnTimerWidget != nullptr)
	{
		RespawnTimerWidget->AddToViewport(2);

		FText Caption = FText::FromString(TEXT("RESPAWN IN"));
		RespawnTimerWidget->SetInfoWithEndTime(Caption, EndTimeSec);

		if (CurrentFullScreenWidget != nullptr)
		{
			RespawnTimerWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AASPlayerController::RemoveRespawnTimerWidget()
{
	if (RespawnTimerWidget != nullptr)
	{
		RespawnTimerWidget->RemoveFromParent();
		RespawnTimerWidget = nullptr;
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

	if (RespawnTimerWidget != nullptr)
	{
		RespawnTimerWidget->SetVisibility(ESlateVisibility::Hidden);
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

	if (RespawnTimerWidget != nullptr)
	{
		RespawnTimerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	ChangeInputMode(true);
	ShowCrossHair(true);
}

void AASPlayerController::OnCalledRespawnTimer()
{
	RespawnPlayer();
}

void AASPlayerController::OnRep_RespawnTime()
{
	ShowRespawnTimerWidget(RespawnTime);
}

void AASPlayerController::OnSetRestartTime(float RestartTime)
{
	if (GetNetMode() == NM_Client)
	{
		ShowRestartTimerWidget(RestartTime);
	}
}
