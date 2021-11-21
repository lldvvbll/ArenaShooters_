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

			PlayerState->OnSetItemSetDataAsset.AddUObject(this, &UASMatchItemSetSelectUserWidget::OnChangedItemSetDataAsset);
		}

		auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
		if (ensure(IsValid(GameState)))
		{
			bool bEnable = GameState->GetInnerMatchState() != EInnerMatchState::Finish;

			TArray<UASItemSetDataAsset*> DataAssets = GameState->GetItemSetDataAssets();
			for (auto& DataAsset : DataAssets)
			{
				auto MatchItemSetSlotWidget = CreateWidget<UASMatchItemSetSlotUserWidget>(this, MatchItemSetSlotWidgetClass);
				if (ensure(MatchItemSetSlotWidget != nullptr))
				{
					SlotsScrollBox->AddChild(MatchItemSetSlotWidget);

					if (!bEnable)
					{
						MatchItemSetSlotWidget->SetIsEnabled(false);
					}
					
					MatchItemSetSlotWidget->SetDataAsset(DataAsset);

					if (ensure(CurItemSetDataAsset != nullptr && DataAsset != nullptr))
					{
						MatchItemSetSlotWidget->ChangeButtonState(DataAsset == CurItemSetDataAsset);
					}
				}
			}			
		}
	}
}

void UASMatchItemSetSelectUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto PlayerState = GetOwningPlayerState<AASPlayerState>();
	if (ensure(IsValid(PlayerState)))
	{
		PlayerState->OnSetItemSetDataAsset.RemoveAll(this);
	}
}

void UASMatchItemSetSelectUserWidget::OnChangedItemSetDataAsset(UASItemSetDataAsset* NewItemSetDataAsset)
{
	if (!ensure(IsValid(NewItemSetDataAsset)))
		return;

	FPrimaryAssetId ItemSetDataAssetID = NewItemSetDataAsset->GetPrimaryAssetId();
	if (!ensure(ItemSetDataAssetID.IsValid()))
		return;

	if (SlotsScrollBox != nullptr)
	{
		for (auto& SlotWidget : SlotsScrollBox->GetAllChildren())
		{
			auto ItemSetSlotWidget = Cast<UASMatchItemSetSlotUserWidget>(SlotWidget);
			if (!ensure(ItemSetSlotWidget != nullptr))
				continue;

			ItemSetSlotWidget->ChangeButtonState(ItemSetSlotWidget->GetItemSetDataAssetId() == ItemSetDataAssetID);
		}
	}
}