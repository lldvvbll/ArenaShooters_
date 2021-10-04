// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Inventory/ASInventoryUserWidget.h"
#include "Character/ASCharacter.h"
#include "Controller/ASPlayerController.h"
#include "GUI/Inventory/ASWeaponSlotUserWidget.h"
#include "GUI/Inventory/ASArmorSlotUserWidget.h"
#include "GUI/Inventory/ASItemScrollBoxWrapperUserWidget.h"
#include "Character/ASInventoryComponent.h"
#include "Item/ASItem.h"
#include "Item/ASWeapon.h"
#include "Item/ASArmor.h"
#include "ItemActor/ASDroppedItemActor.h"

void UASInventoryUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MainWeaponSlotWidget = Cast<UASWeaponSlotUserWidget>(GetWidgetFromName(TEXT("MainWeaponSlot")));
	SubWeaponSlotWidget = Cast<UASWeaponSlotUserWidget>(GetWidgetFromName(TEXT("SubWeaponSlot")));
	HelmetSlotWidget = Cast<UASArmorSlotUserWidget>(GetWidgetFromName(TEXT("HelmetSlot")));
	JacketSlotWidget = Cast<UASArmorSlotUserWidget>(GetWidgetFromName(TEXT("JacketSlot")));
	InventoryItemScrollBoxWrapperWidget = Cast<UASItemScrollBoxWrapperUserWidget>(GetWidgetFromName(TEXT("InventoryItemScrollBoxWrapper")));
	GroundItemScrollBoxWrapperWidget = Cast<UASItemScrollBoxWrapperUserWidget>(GetWidgetFromName(TEXT("GroundItemScrollBoxWrapper")));

	if (AASCharacter* ASChar = GetASCharacter())
	{
		ASInventoryComp = ASChar->GetInventoryComponent();

		ASChar->OnGroundItemAdd.AddUObject(this, &UASInventoryUserWidget::AddItemsToGroundScrollBox);
		ASChar->OnGroundItemRemove.AddUObject(this, &UASInventoryUserWidget::RemoveItemsFromGroundScrollBox);

		OnChangedWeapon(EWeaponSlotType::Main, nullptr);
		OnChangedWeapon(EWeaponSlotType::Sub, nullptr);
		OnChangedArmor(EArmorSlotType::Helmet, nullptr);
		OnChangedArmor(EArmorSlotType::Jacket, nullptr);

		AddItemsToGroundScrollBox(ASChar->GetGroundItems());

		if (ASInventoryComp != nullptr)
		{
			ASInventoryComp->OnAddInventoryItem.AddUObject(this, &UASInventoryUserWidget::OnAddInventoryItem);
			ASInventoryComp->OnRemoveInventoryItem.AddUObject(this, &UASInventoryUserWidget::OnRemoveInventoryItem);
			ASInventoryComp->OnInsertWeapon.AddUObject(this, &UASInventoryUserWidget::OnChangedWeapon);
			ASInventoryComp->OnInsertArmor.AddUObject(this, &UASInventoryUserWidget::OnChangedArmor);

			AddItemsToInventoryScrollBox(ASInventoryComp->GetInventoryItems());
		}
	}

	// Tab, ESC
	FOnInputAction BackToGameAction;
	BackToGameAction.BindDynamic(this, &UASInventoryUserWidget::BackToGame);
	ListenForInputAction(FName(TEXT("ShowInventory")), EInputEvent::IE_Pressed, true, BackToGameAction);
	ListenForInputAction(FName(TEXT("ShowGameMenu")), EInputEvent::IE_Pressed, true, BackToGameAction);

	OnConstructed.Broadcast(this);
}

void UASInventoryUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnDestructed.Broadcast(this);
}

FReply UASInventoryUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return Reply;
}

AASCharacter* UASInventoryUserWidget::GetASCharacter() const
{
	auto PlayerController = Cast<AASPlayerController>(GetOwningPlayer());
	if (PlayerController == nullptr)
	{
		AS_LOG_S(Error);
		return nullptr;
	}

	return Cast<AASCharacter>(PlayerController->GetCharacter());
}

void UASInventoryUserWidget::AddItemsToGroundScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (GroundItemScrollBoxWrapperWidget == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	GroundItemScrollBoxWrapperWidget->AddItemsToScrollBox(Items);
}

void UASInventoryUserWidget::RemoveItemsFromGroundScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (GroundItemScrollBoxWrapperWidget == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	GroundItemScrollBoxWrapperWidget->RemoveItemsFromScrollBox(Items);
}

void UASInventoryUserWidget::AddItemsToInventoryScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (InventoryItemScrollBoxWrapperWidget == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	InventoryItemScrollBoxWrapperWidget->AddItemsToScrollBox(Items);
}

void UASInventoryUserWidget::OnChangedWeapon(EWeaponSlotType SlotType, UASWeapon* RemovedWeapon)
{
	if (ASInventoryComp == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	switch (SlotType)
	{
	case EWeaponSlotType::Main:
		if (MainWeaponSlotWidget != nullptr)
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromWeaponSlot(SlotType);
			if (ItemPair.Value)
			{
				MainWeaponSlotWidget->SetASItem(ItemPair.Key);
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
		break;
	case EWeaponSlotType::Sub:
		if (SubWeaponSlotWidget != nullptr)
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromWeaponSlot(SlotType);
			if (ItemPair.Value)
			{
				SubWeaponSlotWidget->SetASItem(ItemPair.Key);
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
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASInventoryUserWidget::OnChangedArmor(EArmorSlotType SlotType, UASArmor* RemovedArmor)
{
	switch (SlotType)
	{
	case EArmorSlotType::Helmet:
		if (HelmetSlotWidget != nullptr)
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromArmorSlot(SlotType);
			if (ItemPair.Value)
			{
				HelmetSlotWidget->SetASItem(ItemPair.Key);
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
		break;
	case EArmorSlotType::Jacket:
		if (JacketSlotWidget != nullptr)
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromArmorSlot(SlotType);
			if (ItemPair.Value)
			{
				JacketSlotWidget->SetASItem(ItemPair.Key);
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
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASInventoryUserWidget::OnAddInventoryItem(const TWeakObjectPtr<UASItem>& NewItem)
{
	if (InventoryItemScrollBoxWrapperWidget == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	InventoryItemScrollBoxWrapperWidget->AddItemsToScrollBox({ NewItem });
}

void UASInventoryUserWidget::OnRemoveInventoryItem(const TWeakObjectPtr<UASItem>& InItem)
{
	if (InventoryItemScrollBoxWrapperWidget == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	InventoryItemScrollBoxWrapperWidget->RemoveItemsFromScrollBox({ InItem });
}

void UASInventoryUserWidget::BackToGame()
{
	RemoveFromParent();
}
