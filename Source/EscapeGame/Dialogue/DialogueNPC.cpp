// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialogue/DialogueNPC.h"
#include "Dialogue/DialogueParticipantComponent.h"
#include "Dialogue/DialogueQuestSubsystem.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

ADialogueNPC::ADialogueNPC()
{
	// 普通对话 NPC 当前不需要每帧逻辑，关闭 Tick 减少不必要开销。
	PrimaryActorTick.bCanEverTick = false;

	CollisionCylinder = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCylinder"));
	RootComponent = CollisionCylinder;
	CollisionCylinder->InitCapsuleSize(42.0f, 96.0f);
	CollisionCylinder->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionCylinder->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionCylinder->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionCylinder->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionCylinder->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(CollisionCylinder);

	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NPCMesh"));
	NPCMesh->SetupAttachment(CollisionCylinder);
	NPCMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DialogueParticipantComp = CreateDefaultSubobject<UDialogueParticipantComponent>(TEXT("DialogueParticipantComp"));
	InteractText = FText::FromString(TEXT("交谈"));
}

void ADialogueNPC::BeginPlay()
{
	// 目前没有初始化逻辑，保留 BeginPlay 方便后续接入交互组件或对话组件。
	Super::BeginPlay();
	
}

void ADialogueNPC::Tick(float DeltaTime)
{
	// Tick 当前不会执行，因为构造函数里关闭了 bCanEverTick。
	// 后续如果需要 NPC 面向玩家、范围检测等持续逻辑，再重新开启。
	Super::Tick(DeltaTime);

}

bool ADialogueNPC::Interact_Implementation(APawn* InstigatorPawn)
{
	if (!bCanInteract)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueNPC 交互失败：NPC 当前不可交互。Actor=%s"), *GetNameSafe(this));
		return false;
	}

	if (!IsValid(InstigatorPawn))
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueNPC 交互失败：InstigatorPawn 无效。Actor=%s"), *GetNameSafe(this));
		return false;
	}

	if (!bAutoStartDialogueOnInteract)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueNPC 交互未自动开启对话：bAutoStartDialogueOnInteract=false。Actor=%s"), *GetNameSafe(this));
		return true;
	}

	if (!IsValid(DialogueParticipantComp))
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueNPC 交互失败：DialogueParticipantComp 无效。Actor=%s"), *GetNameSafe(this));
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueNPC 交互失败：GameInstance 无效。Actor=%s"), *GetNameSafe(this));
		return false;
	}

	UDialogueQuestSubsystem* DialogueSubsystem = GameInstance->GetSubsystem<UDialogueQuestSubsystem>();
	if (!IsValid(DialogueSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueNPC 交互失败：找不到 UDialogueQuestSubsystem。Actor=%s"), *GetNameSafe(this));
		return false;
	}

	return DialogueSubsystem->StartConversation(InstigatorPawn, DialogueParticipantComp);
}

bool ADialogueNPC::CanInteract_Implementation(AActor* Interactor) const
{
	return bCanInteract && IsValid(DialogueParticipantComp);
}

FText ADialogueNPC::GetInteractText_Implementation(AActor* Interactor) const
{
	return InteractText;
}

