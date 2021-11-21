// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "GameFramework/PlayerController.h"
#include "Common/ASEnums.h"
#include "ASPlayerController.generated.h"

class UASCrossHairUserWidget;
class UASWeapon;
class UASInventoryUserWidget;
class UASGameMenuUserWidget;
class UASPrepareInfoUserWidget;
class UASHudUserWidget;
class AASCharacter;
class UASTimerCaptionUserWidget;
class AASPlayerState;
class UASNotificationUserWidget;

UCLASS()
class ARENASHOOTERS_API AASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AASPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetPawn(APawn* InPawn) override;

	void ChangeInputMode(bool bGameMode);

	virtual void OnChangedInnerMatchState(EInnerMatchState State);

	void SetRespawnTimer(FTimespan Delay);
	void ClearRespawnTimer();
	void RespawnPlayer();

	void ShowRestartTimerWidget(float EndTimerSec);

	void RemoveFullScreenWidget();

	void GoToMainMenu();

	void NotifyMessage(const FString& Message, float Duration = 5.0f);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void OnScope(const TWeakObjectPtr<UASWeapon>& UsingWeapon);
	void OnUnscope();

	void ShowCrossHair(bool bShow);
	void ShowInventoryWidget();
	void ShowGameMenuWidget();
	void ShowPrepareInfoWidget();
	void RemovePrepareInfoWidget();
	void ShowRespawnTimerWidget(float EndTimeSec);
	void RemoveRespawnTimerWidget();

	template <typename TWidget>
	TWidget* ShowFullScreenWidget(const TSubclassOf<TWidget>& WidgetClass, int32 ZOrder = 1)
	{
		if (CurrentFullScreenWidget == nullptr)
		{
			auto FullScreenWidget = CreateWidget<TWidget>(this, WidgetClass);
			if (ensure(FullScreenWidget != nullptr))
			{
				FullScreenWidget->OnConstructed.AddUObject(this, &AASPlayerController::OnConstructedFullScreenWidget);
				FullScreenWidget->OnDestructed.AddUObject(this, &AASPlayerController::OnDestructedFullScreenWidget);

				if (auto ASChar = Cast<AASCharacter>(GetCharacter()))
				{
					FullScreenWidget->OnConstructed.AddUObject(ASChar, &AASCharacter::OnConstructedFullScreenWidget);
					FullScreenWidget->OnDestructed.AddUObject(ASChar, &AASCharacter::OnDestructedFullScreenWidget);
				}

				FullScreenWidget->AddToViewport(ZOrder);

				CurrentFullScreenWidget = FullScreenWidget;

				return FullScreenWidget;
			}
		}

		return nullptr;
	}

	void OnConstructedFullScreenWidget(UUserWidget* ConstructedWidget);
	void OnDestructedFullScreenWidget(UUserWidget* DestructedWidget);

	void OnCalledRespawnTimer();

	UFUNCTION()
	void OnRep_RespawnTime();

	void OnSetRestartTime(float RestartTime);

	UFUNCTION(Exec)
	void ReadyMatch();

	UFUNCTION(Server, Reliable)
	void ServerReadyMatch();
	void ServerReadyMatch_Implementation();

	UFUNCTION(Exec)
	void FinishMatch();

	UFUNCTION(Server, Reliable)
	void ServerFinishMatch();
	void ServerFinishMatch_Implementation();

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

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASTimerCaptionUserWidget> TimerCaptionUserWidgetClass;

	UPROPERTY()
	UASTimerCaptionUserWidget* RespawnTimerWidget;

	UPROPERTY()
	UASTimerCaptionUserWidget* RestartTimerWidget;

	FInputModeGameOnly GameInputMode;
	FInputModeGameAndUI UIInputMode;

	UPROPERTY(ReplicatedUsing = OnRep_RespawnTime)
	float RespawnTime;

	FTimerHandle RespawnTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASNotificationUserWidget> NotificationWidgetClass;
};
