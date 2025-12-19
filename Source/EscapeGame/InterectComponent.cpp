// Fill out your copyright notice in the Description page of Project Settings.


#include "InterectComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h" // 必须加！不然不认识 PC
#include "Blueprint/UserWidget.h"           // 必须加！不然不认识 CreateWidget
#include "Kismet/GameplayStatics.h"
#include "Interface/PickupInterface.h"
#include "InventoryMenuWidget.h"            // 你的自定义UI头文件
#include "GameFramework/Controller.h"
#include"Collision.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "InventoryComponent.h"

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

	// 【关键补丁】
	// 组件出生时，去问问主人：“嘿，你身上有没有挂着背包组件？”
	// 如果不找，InventoryComp 就是空指针，一会打开 UI 就会崩溃！
	AActor* Owner = GetOwner();
	if (Owner)
	{
		InventoryComp = Owner->FindComponentByClass<UInventoryComponent>();
		if (!InventoryComp)
		{
			UE_LOG(LogTemp, Error, TEXT("香子兰警告：%s 身上忘了挂 InventoryComponent！"), *Owner->GetName());
		}
	}
}


// Called every frame
void UInterectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInterectComponent::ToggleInventory()
{

	// 【错误修正 1】组件没有 GetController()！
	// 必须先找到 Owner (Pawn)，再从 Pawn 找 Controller
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());

	UE_LOG(LogTemp, Warning, TEXT("香子兰正在监视：按下了 I 键！"));

	if (!PC || !InventoryMenuClass)
	{
		UE_LOG(LogTemp, Error, TEXT("缺少 PC 或者 未设置 UI Class！"));
		return;
	}

	// 如果窗口不存在，就创建它
	if (!InventoryMenuInstance)
	{
		InventoryMenuInstance = CreateWidget<UUserWidget>(PC, InventoryMenuClass);
		if (InventoryMenuInstance)
		{
			UInventoryMenuWidget* MenuWidget = Cast<UInventoryMenuWidget>(InventoryMenuInstance);

			// 这里用到了 InventoryComp，所以在 BeginPlay 里必须获取到它！
			if (MenuWidget && InventoryComp)
			{
				MenuWidget->InitializeInventory(InventoryComp);
			}
		}
	}
	// --- 下面是必须补上的逻辑 ---

	if (InventoryMenuInstance) // 再次确认一下有东西
	{

		// 判断当前是在屏幕上显示着，还是藏着
		if (InventoryMenuInstance->IsInViewport())
		{
			// === 如果开着，就关掉 ===
			InventoryMenuInstance->RemoveFromParent(); // 从屏幕移除

			// 把鼠标藏起来，控制权还给游戏角色
			FInputModeGameOnly GameMode;
			PC->SetInputMode(GameMode);
			PC->bShowMouseCursor = false;

			PC->SetIgnoreLookInput(false);
			PC->SetIgnoreMoveInput(false);

		}
		else
		{
			// === 如果关着，就打开 ===
			InventoryMenuInstance->AddToViewport(); // 贴到屏幕上

			// 把鼠标显示出来，控制权交给 UI
			FInputModeGameAndUI UIMode;
			UIMode.SetWidgetToFocus(InventoryMenuInstance->TakeWidget()); // 让UI获得焦点
			UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(UIMode);
			PC->bShowMouseCursor = true;

			// 这里的重点！禁止行动！
			PC->SetIgnoreMoveInput(true); // 禁止 WASD
			PC->SetIgnoreLookInput(true); // 禁止鼠标旋转镜头
		}
	}


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
		}
	}

	// --- 调试画线 (可选) ---
	// 如果你想看见那个球，把下面这行取消注释 (需要 #include "DrawDebugHelpers.h")
	// DrawDebugSphere(GetWorld(), Start, 150.0f, 12, FColor::Red, false, 2.0f);
}
