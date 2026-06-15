// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "Inventory/ItemData.h"
#include "InteractDoor.generated.h"

UCLASS()
class ESCAPEGAME_API AInteractDoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//ģ�����
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Components")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	FItemData RequireKeyID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bConsumeKey = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bIsOpen = false;

	// 1. �������ܲ��ܽ�����(�����װ���ţ������Ϊ false)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bIsInteractable = true;

	//�ӿ�ʵ��
	virtual bool Interact_Implementation(APawn* InstigatorPawn) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	virtual FText GetInteractText_Implementation(AActor* Interactor) const override;	

	// --- ��ͼ�¼� ---
	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorOpen(); // ��ͼ����ת���Ű�

	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorLocked(); // ��ͼ���𲥷���ס������


};
