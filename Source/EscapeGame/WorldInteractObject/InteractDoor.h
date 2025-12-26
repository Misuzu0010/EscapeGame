// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
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
	//模型组件
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Components")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	FName RequireKeyID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bConsumeKey = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bIsOpen = false;

	// 1. 这扇门能不能交互？(如果是装饰门，这就设为 false)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bIsInteractable = true;

	//接口实现
	virtual bool Interact_Implementation(APawn* InstigatorPawn) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	virtual FText GetInteractText_Implementation(AActor* Interactor) const override;	

	// --- 蓝图事件 ---
	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorOpen(); // 蓝图负责转动门板

	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorLocked(); // 蓝图负责播放锁住的声音


};
