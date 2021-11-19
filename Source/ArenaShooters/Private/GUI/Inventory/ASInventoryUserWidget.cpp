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
	if (IsValid(ASChar))
	{
		ASChar->OnGroundItemAdd.RemoveAll(this);
		ASChar->OnGroundItemRemove.RemoveAll(this);

		ASInventoryComp = ASChar->GetInventoryComponent();
		if (IsValid(ASInventoryComp))
		{
			ASInventoryComp->OnAddInventoryItem.RemoveAll(this);
			ASInventoryComp->OnRemoveInventoryItem.RemoveAll(this);
			ASInventoryComp->OnInsertWeapon.RemoveAll(this);
			ASInventoryComp->OnInsertArmor.RemoveAll(this);
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

	if (auto DragDropOp = Cast<UASItemDragDropOperation>(InOperation))
	{
		const TWeakObjectPtr<UASItem>& Item = DragDropOp->GetItem();
		if (Item.IsValid())
		{
			UWidget* SuitableWidget = FindSuitableToDropInventoryWidget(Item);
			if (SuitableWidget != nullptr)
			{
				UWidget* ParentWidget = DragDropOp->GetParentWidget();
				if (ParentWidget != SuitableWidget)
				{
					auto ASChar = GetOwningPlayerPawn<AASCharacter>();
					if (IsValid(ASChar))
					{
						ASChar->PickUpItem(Item.Get());
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
		else
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}

	return false;
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

UUserWidget* UASInventoryUserWidget::FindSuitableToDropInventoryWidget(const TWeakObjectPtr<UASItem>& DopItem) const
{
	if (!DopItem.IsValid())
	{
		AS_LOG_S(Error);
		return nullptr;
	}
	
	UUserWidget* ReturnWidget = nullptr;

	EItemType ItemType = DopItem->GetItemType();
	switch (ItemType)
	{
	case EItemType::Weapon:
		if (auto Weapon = Cast<UASWeapon>(DopItem))
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
		else
		{
			AS_LOG_S(Error);
		}
		break;
	case EItemType::Armor:
		if (auto Armor = Cast<UASArmor>(DopItem))
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
		else
		{
			AS_LOG_S(Error);
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
