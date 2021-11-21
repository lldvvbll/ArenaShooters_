// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Inventory/ASInventoryUserWidget.h"
#include "Character/ASCharacter.h"
#include "Controller/ASPlayerController.h"
#include "GUI/Inventory/ASWeaponSlotUserWidget.h"
#include "GUI/Inventory/ASArmorSlotUserWidget.h"
#include "GUI/Inventory/ASItemScrollBoxWrapperUserWidget.h"
#include "GUI/Inventory/ASItemDragDropOperation.h"
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

	AASCharacter* ASChar = GetASCharacter();
	if (ensure(IsValid(ASChar)))
	{
		ASChar->OnGroundItemAdd.RemoveAll(this);
		ASChar->OnGroundItemRemove.RemoveAll(this);

		ASInventoryComp = ASChar->GetInventoryComponent();
		if (ensure(IsValid(ASInventoryComp)))
		{
			ASInventoryComp->OnAddInventoryItem.RemoveAll(this);
			ASInventoryComp->OnRemoveInventoryItem.RemoveAll(this);
			ASInventoryComp->OnInsertWeapon.RemoveAll(this);
			ASInventoryComp->OnInsertArmor.RemoveAll(this);
		}
	}

	OnDestructed.Broadcast(this);
}

FReply UASInventoryUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return Reply;
}

bool UASInventoryUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation))
		return true;

	auto DragDropOp = Cast<UASItemDragDropOperation>(InOperation);
	if (ensure(DragDropOp != nullptr))
	{
		const TWeakObjectPtr<UASItem>& Item = DragDropOp->GetItem();
		if (ensure(Item.IsValid()))
		{
			UWidget* SuitableWidget = FindSuitableToDropInventoryWidget(Item);
			if (ensure(SuitableWidget != nullptr))
			{
				UWidget* ParentWidget = DragDropOp->GetParentWidget();
				if (ParentWidget != SuitableWidget)
				{
					auto ASChar = GetOwningPlayerPawn<AASCharacter>();
					if (ensure(IsValid(ASChar)))
					{
						ASChar->PickUpItem(Item.Get());
					}
				}
			}
		}
	}

	return false;
}

AASCharacter* UASInventoryUserWidget::GetASCharacter() const
{
	auto PlayerController = Cast<AASPlayerController>(GetOwningPlayer());
	if (!ensure(PlayerController != nullptr))
		return nullptr;

	return Cast<AASCharacter>(PlayerController->GetCharacter());
}

void UASInventoryUserWidget::AddItemsToGroundScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (!ensure(GroundItemScrollBoxWrapperWidget != nullptr))
		return;

	GroundItemScrollBoxWrapperWidget->AddItemsToScrollBox(Items);
}

void UASInventoryUserWidget::RemoveItemsFromGroundScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (!ensure(GroundItemScrollBoxWrapperWidget != nullptr))
		return;

	GroundItemScrollBoxWrapperWidget->RemoveItemsFromScrollBox(Items);
}

void UASInventoryUserWidget::AddItemsToInventoryScrollBox(const TArray<TWeakObjectPtr<UASItem>>& Items)
{
	if (!ensure(InventoryItemScrollBoxWrapperWidget != nullptr))
		return;

	InventoryItemScrollBoxWrapperWidget->AddItemsToScrollBox(Items);
}

void UASInventoryUserWidget::OnChangedWeapon(EWeaponSlotType SlotType, UASWeapon* RemovedWeapon)
{
	if (!ensure(ASInventoryComp != nullptr))
		return;

	switch (SlotType)
	{
	case EWeaponSlotType::Main:
		if (ensure(MainWeaponSlotWidget != nullptr))
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromWeaponSlot(SlotType);
			if (ensure(ItemPair.Value))
			{
				MainWeaponSlotWidget->SetASItem(ItemPair.Key);
			}
		}
		break;
	case EWeaponSlotType::Sub:
		if (ensure(SubWeaponSlotWidget != nullptr))
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromWeaponSlot(SlotType);
			if (ensure(ItemPair.Value))
			{
				SubWeaponSlotWidget->SetASItem(ItemPair.Key);
			}
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
		if (ensure(HelmetSlotWidget != nullptr))
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromArmorSlot(SlotType);
			if (ensure(ItemPair.Value))
			{
				HelmetSlotWidget->SetASItem(ItemPair.Key);
			}
		}
		break;
	case EArmorSlotType::Jacket:
		if (ensure(JacketSlotWidget != nullptr))
		{
			ItemPtrBoolPair ItemPair = ASInventoryComp->FindItemFromArmorSlot(SlotType);
			if (ensure(ItemPair.Value))
			{
				JacketSlotWidget->SetASItem(ItemPair.Key);
			}
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASInventoryUserWidget::OnAddInventoryItem(const TWeakObjectPtr<UASItem>& NewItem)
{
	if (!ensure(InventoryItemScrollBoxWrapperWidget != nullptr))
		return;

	InventoryItemScrollBoxWrapperWidget->AddItemsToScrollBox({ NewItem });
}

void UASInventoryUserWidget::OnRemoveInventoryItem(const TWeakObjectPtr<UASItem>& InItem)
{
	if (!ensure(InventoryItemScrollBoxWrapperWidget != nullptr))
		return;

	InventoryItemScrollBoxWrapperWidget->RemoveItemsFromScrollBox({ InItem });
}

void UASInventoryUserWidget::BackToGame()
{
	RemoveFromParent();
}

UUserWidget* UASInventoryUserWidget::FindSuitableToDropInventoryWidget(const TWeakObjectPtr<UASItem>& DopItem) const
{
	if (!ensure(DopItem.IsValid()))
		return nullptr;
	
	UUserWidget* ReturnWidget = nullptr;

	EItemType ItemType = DopItem->GetItemType();
	switch (ItemType)
	{
	case EItemType::Weapon:
		{
			auto Weapon = Cast<UASWeapon>(DopItem);
			if (ensure(Weapon != nullptr))
			{
				EWeaponSlotType SlotType = UASInventoryComponent::GetSuitableWeaponSlotType(Weapon->GetWeaponType());
				switch (SlotType)
				{
				case EWeaponSlotType::Main:
					ReturnWidget = MainWeaponSlotWidget;
					break;
				case EWeaponSlotType::Sub:
					ReturnWidget = SubWeaponSlotWidget;
					break;
				default:
					checkNoEntry();
					break;
				}
			}
		}		
		break;
	case EItemType::Armor:
		{
			auto Armor = Cast<UASArmor>(DopItem);
			if (ensure(Armor != nullptr))
			{
				EArmorSlotType SlotType = UASInventoryComponent::GetSuitableArmorSlotType(Armor->GetArmorType());
				switch (SlotType)
				{
				case EArmorSlotType::Helmet:
					ReturnWidget = HelmetSlotWidget;
					break;
				case EArmorSlotType::Jacket:
					ReturnWidget = JacketSlotWidget;
					break;
				default:
					checkNoEntry();
					break;
				}
			}
		}
		break;
	case EItemType::Ammo:			// fallthrough
	case EItemType::HealingKit:
		{
			ReturnWidget = InventoryItemScrollBoxWrapperWidget;
		}
		break;
	default:
		checkNoEntry();
		break;
	}

	return ReturnWidget;
}
