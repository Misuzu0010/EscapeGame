// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupData.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "InventoryComponent.h" // 引用背包组件
#include "Kismet/GameplayStatics.h"

// Sets default values
APickupData::APickupData()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//初始化mesh
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	//这里的写法涉及到AActor的源码
	/*
	
	    // AActor.cpp (引擎源码)

    void AActor::SetActorLocation(FVector NewLocation)
    {
        // 引擎心里想：我要移动这个 Actor
        // 可是 Actor 是个虚无的概念，我到底要移动谁呢？
        // 对了！我要移动它的 RootComponent！
    
        if (RootComponent != nullptr)
        {
            RootComponent->SetWorldLocation(NewLocation);
        }
        else
        {
            // 如果 RootComponent 是空的，我就不知道该移动谁了
            // 于是我什么都不做，或者报错
            UE_LOG(..., Warning, "移动失败！这个 Actor 没有根组件！");
        }
    }
	*/
	RootComponent = MeshComp;

	//默认设置 没有物理模拟，只有碰撞
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //初始化检测球
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetSphereRadius(80.0f);

	//设置为QueryOnly，只有查询碰撞，没有物理响应

    SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

// Called when the game starts or when spawned
void APickupData::BeginPlay()
{
	Super::BeginPlay();
	
}

bool APickupData::AttemptPickUp_Implementation(APawn* InstigatorPawn)
{
    if (!InstigatorPawn)return false;


    //尝试获取背包组件
    UInventoryComponent* Inventory = InstigatorPawn->FindComponentByClass<UInventoryComponent>();

    if (Inventory) 
    {
        int32 Leftover = Inventory->AddItem(ItemContent, ItemCount);

        if (Leftover < ItemCount) 
        {
            UE_LOG(LogTemp, Log, TEXT("Picked up: %s"), *ItemContent.ID.ToString());

            if (Leftover <= 0) 
            {
				Destroy();
                return true;
            }
            else
            {
                ItemCount = Leftover;
                return true;
            }

        }
        else 
        {
            //背包满了
            UE_LOG(LogTemp, Warning, TEXT("Inventory Full!!!"));
            return false;
        }


    }
    return false;
}

// === 编辑器可视化逻辑 ===
// 只要你在编辑器里修改了 ItemData，或者拖拽了这个 Actor，这个函数就会跑
#if WITH_EDITOR
void APickupData::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 如果 ItemContent 里配置了模型，就自动设置给 MeshComp
    if (ItemContent.WorldMesh && MeshComp)
    {
        MeshComp->SetStaticMesh(ItemContent.WorldMesh);
    }
}
#endif