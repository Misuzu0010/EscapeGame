// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include"Interface/PickupInterface.h"
#include"ItemData.h"
#include "PickupData.generated.h"

class USphereComponent;

UCLASS()
class ESCAPEGAME_API APickupData : public AActor,public IPickupInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupData();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,Category="Components")
	TObjectPtr<UStaticMeshComponent>MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp; // 用于检测碰撞

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Pickup",meta = (ExposeOnSpawn = "true"))
	FItemData ItemContent;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Pickup")
	int32 ItemCount = 1; // 拾取物品的数量

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	// --- 接口实现 ---
	// 这是核心！当角色按 E 或者踩上去时，调用这个
	virtual bool AttemptPickUp_Implementation(APawn* InstigatorPawn) override;

	// 当我们在编辑器里修改 ItemContent 时，自动更新模型！(这是个超级好用的功能)
#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
	// OnConstruction 比 PostEditChangeProperty 更通用，拖拽时也能生效
#endif


};
