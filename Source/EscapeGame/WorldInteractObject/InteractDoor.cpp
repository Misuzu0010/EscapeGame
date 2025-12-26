// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldInteractObject/InteractDoor.h"
#include"InventoryComponent.h"

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
	if (!bIsInteractable)return FText::GetEmpty();

	if (!RequireKeyID.IsNone()) 
	{
		return FText::FromString("Use Key to Open Door");
	}

	return FText::FromString(TEXT("Open Door"));
}

bool AInteractDoor::Interact_Implementation(APawn* InstigatorPawn) 
{
	if (!InstigatorPawn || !bIsInteractable)return false;

	if (RequireKeyID.IsNone()) 
	{
		bIsOpen = true;
		OnDoorOpen();
		return true;
	}

	UInventoryComponent* InventoryComp = InstigatorPawn->FindComponentByClass<UInventoryComponent>();

	if (InventoryComp) 
	{
		if (InventoryComp->GetTotalCountOfItem(RequireKeyID) > 0) 
		{
			bIsOpen = true;
			OnDoorOpen();
			//可以加一个打开门的文本提示

			if (bConsumeKey) 
			{
				
			}
			return true;

		}
		else 
		{
			OnDoorLocked();
			return false;


		}

	}
	return false;
}
bool AInteractDoor::CanInteract_Implementation(AActor* Interactor) const
{
	return bIsInteractable && !bIsOpen;
}