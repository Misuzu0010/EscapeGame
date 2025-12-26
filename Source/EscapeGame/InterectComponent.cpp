// Fill out your copyright notice in the Description page of Project Settings.


#include "InterectComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h" // 必须加！不然不认识 PC
#include "Blueprint/UserWidget.h"           // 必须加！不然不认识 CreateWidget
#include "Kismet/GameplayStatics.h"
#include "Interface/PickupInterface.h"
#include "GameFramework/Controller.h"
#include"Collision.h"
#include "Engine/World.h"
#include"Interface/InteractableInterface.h"
#include "Engine/EngineTypes.h"

// Sets default values for this component's properties
UInterectComponent::UInterectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UInterectComponent::BeginPlay()
{
	Super::BeginPlay();


}


// Called every frame
void UInterectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UInterectComponent::OnInteract(const FInputActionValue& Value)
{
	// --- 1. 准备参数 ---

	UE_LOG(LogTemp, Warning, TEXT("香子兰收到指令：正在尝试交互！"));


	//必须问 Owner 要坐标！
	AActor* MyOwner = GetOwner();
	if (!MyOwner) return;

	FVector Start = MyOwner->GetActorLocation();
	FVector End = Start; // 起点和终点一样，就相当于原地生成一个球

	// 创建一个半径 150 的球形
	FCollisionShape Shape = FCollisionShape::MakeSphere(150.0f);

	// 忽略参数：忽略自己
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(MyOwner);

	// 用来存结果的数组（注意这里变成了 HitResult，更常用！）
	TArray<FHitResult> OutHits;

	// --- 2. 执行扫描 (Sweep) ---
	// ECC_WorldDynamic 包含了大多数交互物，也可以改成 ECC_Pawn 或 ECC_Visibility 看你的设置
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		ECC_WorldDynamic,
		Shape,
		Params
	);

	// --- 3. 处理结果 ---
	if (bHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			// 【重点】FHitResult 的 GetActor() 非常稳定，不容易报错
			AActor* HitActor = Hit.GetActor();

			// 防御性编程：先判空
			if (!HitActor) continue;
			UE_LOG(LogTemp, Warning, TEXT("香子兰看到了: %s"), *HitActor->GetName());

			// 检查是否实现了接口
			if (HitActor->Implements<UPickupInterface>())
			{
				// 找到了！执行交互
				APawn* PawnOwner = Cast<APawn>(MyOwner);
				if (PawnOwner)
				{
					// 现在传进去的就是 Pawn* 类型了，报错就会消失！
					IPickupInterface::Execute_AttemptPickUp(HitActor, PawnOwner);

					UE_LOG(LogTemp, Warning, TEXT("香子兰帮你摸到了: %s"), *HitActor->GetName());
					break;
				}
			}
			else if (HitActor->Implements<UInteractableInterface>())
			{
				// 这里调用新的通用交互逻辑
				// 注意：要传入 Pawn，方便门知道是谁开了它
				APawn* PawnOwner = Cast<APawn>(GetOwner());
				if(PawnOwner)
				{
					IInteractableInterface::Execute_Interact(HitActor, PawnOwner);
				}

				break; // 交互通常一次只触发一个
			}
			else 
			{
				UE_LOG(LogTemp, Error, TEXT("哎哟我草，啥也没有呀"));
			}
		}
	}

	// --- 调试画线 (可选) ---
	// 如果你想看见那个球，把下面这行取消注释 (需要 #include "DrawDebugHelpers.h")
	// DrawDebugSphere(GetWorld(), Start, 150.0f, 12, FColor::Red, false, 2.0f);
}

void UInterectComponent::RequestToggleInventory()
{
	// 我不管UI怎么开，我只管喊一声
	if (OnRequestToggleInventory.IsBound()) 
	{
		OnRequestToggleInventory.Broadcast();
	}
}