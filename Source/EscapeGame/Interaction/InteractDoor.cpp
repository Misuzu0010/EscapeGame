// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractDoor.h"
#include "Inventory/InventoryComponent.h"

// Sets default values
AInteractDoor::AInteractDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AInteractDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractDoor::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);

}

FText AInteractDoor::GetInteractText_Implementation(AActor* Interactor) const
{
	if (!bIsInteractable)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("门交互文本为空：Door=%s 当前不可交互，Interactor=%s。"),
			*GetNameSafe(this),
			*GetNameSafe(Interactor));
		return FText::GetEmpty();
	}

	if (!RequireKeyID.ID.IsNone()) 
	{
		return FText::FromString("Use Key to Open Door");
	}

	return FText::FromString(TEXT("Open Door"));
}

bool AInteractDoor::Interact_Implementation(APawn* InstigatorPawn) 
{
	if (!InstigatorPawn || !bIsInteractable)
	{
		UE_LOG(LogTemp, Warning, TEXT("开门失败：Door=%s, InstigatorPawn=%s, bIsInteractable=%s。"),
			*GetNameSafe(this),
			*GetNameSafe(InstigatorPawn),
			bIsInteractable ? TEXT("true") : TEXT("false"));
		return false;
	}

	if (RequireKeyID.ID.IsNone()) 
	{
		bIsOpen = true;
		OnDoorOpen();
		return true;
	}

	UInventoryComponent* InventoryComp = InstigatorPawn->FindComponentByClass<UInventoryComponent>();

	if (InventoryComp) 
	{
		if (InventoryComp->GetTotalCountOfItem(RequireKeyID.ID) > 0) 
		{
			bIsOpen = true;
			OnDoorOpen();
			// 钥匙验证通过后先触发开门事件，再按配置决定是否消耗钥匙。

			if (bConsumeKey) 
			{
				InventoryComp->RemoveItem(RequireKeyID, 1);
			}
			return true;

		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("开门失败：Pawn=%s 缺少钥匙 ItemID=%s。"),
				*GetNameSafe(InstigatorPawn),
				*RequireKeyID.ID.ToString());
			OnDoorLocked();
			return false;


		}

	}
	UE_LOG(LogTemp, Warning, TEXT("开门失败：Pawn=%s 没有 InventoryComponent，无法检查钥匙 ItemID=%s。"),
		*GetNameSafe(InstigatorPawn),
		*RequireKeyID.ID.ToString());
	return false;
}
bool AInteractDoor::CanInteract_Implementation(AActor* Interactor) const
{
	return bIsInteractable && !bIsOpen;
}
