// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupData.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "InventoryComponent.h" // 引用背包组件
#include "Engine/DataTable.h" // 必须引用这个才能用 FindRow
#include "Kismet/GameplayStatics.h"

// PickupData.cpp 构造函数

APickupData::APickupData()
{
    PrimaryActorTick.bCanEverTick = false;

    // 1. 网格体设置
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    // 关掉 Mesh 的物理碰撞，我们只用 SphereComp 来做检测
    // 这样就算你的 Key 模型自带了 BlockAll，这里也会把它关掉，防止绊倒玩家
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 2. 检测球设置
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->SetupAttachment(RootComponent);
    SphereComp->SetSphereRadius(80.0f);

    // --- 核心碰撞逻辑 (The Holy Grail) ---

    // 开启查询 (Query)，禁用物理模拟 (Physics)
    SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 第一步：先清空所有，把所有频道都设为 Ignore (忽略)
    SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);

    // 第二步：允许 玩家 (Pawn) 踩上去 -> 触发重叠事件 (Overlap)
    // 这样你就能穿过它，不会被卡住脚
    SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 第三步：允许 射线 (WorldDynamic) 打中它 -> 触发阻挡 (Block)
    // 这样你的交互射线 (SweepMultiByChannel) 就能检测到它！
    SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

    // (额外保险) 如果你的射线用的是 Visibility 频道，把这个也加上
    SphereComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}
// Called when the game starts or when spawned
void APickupData::BeginPlay()
{
	Super::BeginPlay();
	
}

bool APickupData::AttemptPickUp_Implementation(APawn* InstigatorPawn) 
{
    if (!InstigatorPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("拾取失败：InstigatorPawn 为空，Pickup=%s, ItemID=%s。"),
            *GetNameSafe(this),
            *ItemID.ToString());
        return false;
    }

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
    UE_LOG(LogTemp, Warning, TEXT("拾取失败：Pawn=%s 没有 InventoryComponent，ItemID=%s。"),
        *GetNameSafe(InstigatorPawn),
        *ItemID.ToString());
    return false;

}

// === 可视化逻辑：自动换模型 ===
// 只要你在编辑器里修改 ItemID，或者拖动 Actor，这个函数就会跑
void APickupData::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // ... 你的判断 ...
    if (ItemDataTable && !ItemID.IsNone())
    {
        // 加上这一句！看看是不是真的找到了！
        FItemData* RowData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("Debug"));

        if (RowData)
        {
            UE_LOG(LogTemp, Warning, TEXT("喵！查表成功！模型应该是: %s"),
                RowData->WorldMesh ? *RowData->WorldMesh->GetName() : TEXT("空"));

            if (RowData->WorldMesh)
            {
                MeshComp->SetStaticMesh(RowData->WorldMesh);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("喵！查表失败！找不到 ID: %s"), *ItemID.ToString());
        }
    }
}
