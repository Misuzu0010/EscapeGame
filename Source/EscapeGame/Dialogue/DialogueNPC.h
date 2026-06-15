// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include"GameplayTagContainer.h"
#include "DialogueNPC.generated.h"

class UDialogueParticipantComponent;

UCLASS()
class ESCAPEGAME_API ADialogueNPC : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADialogueNPC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//数据层使用的id
	UPROPERTY(BlueprintReadWrite,Category="NPC")
	FGameplayTag NPC_ID;
	
	//对外展示名称
	UPROPERTY(BlueprintReadWrite,Category="NPC")
	FText NPCDisplayName;
	
	//头像
	UPROPERTY(BlueprintReadWrite,Category="NPC")
	TSoftObjectPtr<UTexture2D> NPCPortraitTexture;
	
	//对话逻辑
	UPROPERTY(BlueprintReadWrite, Category="NPC")
	TObjectPtr<UDialogueParticipantComponent> DialogueParticipantComp;
	
private:
	
	

};
