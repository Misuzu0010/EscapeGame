// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupData.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "InventoryComponent.h" // 引用背包组件
#include "Engine/DataTable.h" // 必须引用这个才能用 FindRow
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
    if (!InstigatorPawn) return false;

    if (!ItemDataTable || ItemID.IsNone()) 
    {
        UE_LOG(LogTemp, Error, TEXT("PickupData: 缺少 DataTable 或 ItemID，无法捡起！"));
        return false;

    }

    FItemData* RowData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("Pickup Attempt"));
    
    if (!RowData) 
    {
        UE_LOG(LogTemp, Error, TEXT("PickUpData: ID %s 不存在"), *ItemID.ToString());
        return false;
    }

	UInventoryComponent* Inventory = InstigatorPawn->FindComponentByClass<UInventoryComponent>();

    if (Inventory) 
    {
        int32 LeftOver = Inventory->AddItem(*RowData, ItemCount);

        if (LeftOver < ItemCount) 
        {
            UE_LOG(LogTemp, Log, TEXT("Picked up: %s"), *ItemID.ToString());

            if (LeftOver <= 0)
            {
                Destroy();
                return true;
            }
            else
            {
                ItemCount = LeftOver;
                return true;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Inventory Full!!!"));
            return false;
        }
    }
    return false;

}

// === 可视化逻辑：自动换模型 ===
// 只要你在编辑器里修改 ItemID，或者拖动 Actor，这个函数就会跑
void APickupData::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 1. 检查是否有表和ID
    if (ItemDataTable && !ItemID.IsNone())
    {
        // 2. 查表 (OnConstruction里通常不报错，静默失败即可)
        FItemData* RowData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("Pickup OnConstruction"));

        // 3. 换模型
        if (RowData && RowData->WorldMesh && MeshComp)
        {
            MeshComp->SetStaticMesh(RowData->WorldMesh);
        }
    }
}