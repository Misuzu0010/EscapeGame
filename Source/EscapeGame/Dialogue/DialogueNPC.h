// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interfaces/InteractableInterface.h"
#include "DialogueNPC.generated.h"

class UDialogueParticipantComponent;
class UArrowComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;

// 普通 NPC 的 Actor 包装。
// 这个类只负责“能被玩家交互”和“把交互请求转给对话系统”，不承担完整对话内容存储。
UCLASS()
class ESCAPEGAME_API ADialogueNPC : public AActor,public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// 构造时关闭 Tick，普通对话 NPC 不需要每帧更新。
	ADialogueNPC();

protected:
	// 游戏开始或 Actor 生成时调用。
	virtual void BeginPlay() override;

public:	
	// 每帧调用。当前骨架里没有行为，保留是为了后续扩展。
	virtual void Tick(float DeltaTime) override;

	// 玩家按交互键时由 UInteractComponent 调用。
	virtual bool Interact_Implementation(APawn* InstigatorPawn) override;

	// 返回当前 NPC 是否允许被交互。
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	// 返回交互提示文本。
	virtual FText GetInteractText_Implementation(AActor* Interactor) const override;

	// 普通 NPC 默认拥有的对话参与者组件。
	// 显示名、头像、对话资产、起始节点等对话身份配置都放在这里。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dialogue")
	TObjectPtr<UDialogueParticipantComponent> DialogueParticipantComp;

	// NPC 的基础碰撞体，负责交互命中和站位碰撞。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC|Collision")
	TObjectPtr<UCapsuleComponent> CollisionCylinder;

	// 编辑器中显示 NPC 朝向。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC|Visual")
	TObjectPtr<UArrowComponent> ArrowComponent;

	// NPC 的可视人物模型。蓝图子类可以直接替换 Skeletal Mesh 和 AnimBP。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC|Visual")
	TObjectPtr<USkeletalMeshComponent> NPCMesh;
	
	// 玩家靠近时显示的交互提示文本。例：“交谈”“按 E 对话”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "NPC")
	FText InteractText;
	
	// true：当前允许玩家交互；false：这个 NPC 暂时不响应交互。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bCanInteract = true;
	
	// true：玩家交互后立刻进入对话；false：只触发提示或其他逻辑，由脚本再决定是否开对话。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bAutoStartDialogueOnInteract = true;
	
	// true：允许 UI 显示交互提示。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bShowInteractionPrompt = true;
};
