// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASMatchItemSetSelectUserWidget.h"
#include "GUI/ASMatchItemSetSlotUserWidget.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "Components/ScrollBox.h"
#include "Controller/ASPlayerState.h"
#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"

void UASMatchItemSetSelectUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SlotsScrollBox = Cast<UScrollBox>(GetWidgetFromName(TEXT("SB_Slots")));

	if (SlotsScrollBox != nullptr)
	{
		SlotsScrollBox->ClearChildren();

		UASItemSetDataAsset* CurItemSetDataAsset = nullptr;
		auto PlayerState = GetOwningPlayer()->GetPlayerState<AASPlayerState>();
		if (IsValid(PlayerState))
		{
			CurItemSetDataAsset = PlayerState->GetItemSetDataAsset();
		}

		auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
		if (IsValid(GameState))
		{
			TArray<UASItemSetDataAsset*> DataAssets = GameState->GetItemSetDataAssets();
			for (auto& DataAsset : DataAssets)
			{
				auto MatchItemSetSlotWidget = CreateWidget<UASMatchItemSetSlotUserWidget>(this, MatchItemSetSlotWidgetClass);
				if (MatchItemSetSlotWidget != nullptr)
				{
					SlotsScrollBox->AddChild(MatchItemSetSlotWidget);

					MatchItemSetSlotWidget->SetDataAsset(DataAsset);

					if (CurItemSetDataAsset != nullptr && DataAsset != nullptr)
					{
						MatchItemSetSlotWidget->ChangeButtonState(DataAsset == CurItemSetDataAsset);
					}
					else
					{
						AS_LOG_S(Error);
					}

					MatchItemSetSlotWidget->OnClickedSlot.AddUObject(this, &UASMatchItemSetSelectUserWidget::OnClickedSlot);
				}
				else
				{
					AS_LOG_S(Error);
				}
			}			
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
}

void UASMatchItemSetSelectUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto PlayerState = GetOwningPlayerState<AASPlayerState>();
	if (IsValid(PlayerState))
	{
		PlayerState->OnSetItemSetDataAsset.RemoveAll(this);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASMatchItemSetSelectUserWidget::OnClickedSlot(UASMatchItemSetSlotUserWidget* ClickedSlot)
{
	if (ClickedSlot == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (SlotsScrollBox != nullptr)
	{
		for (auto& SlotWidget : SlotsScrollBox->GetAllChildren())
		{
			auto ItemSetSlotWidget = Cast<UASMatchItemSetSlotUserWidget>(SlotWidget);
			if (ItemSetSlotWidget == nullptr)
			{
				AS_LOG_S(Error);
				continue;
			}

			ItemSetSlotWidget->ChangeButtonState(ItemSetSlotWidget == ClickedSlot);
		}
	}
}