// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/EscapeCombatDamageable.h"
#include "AICharacter_1.generated.h"

UCLASS()
class ESCAPEGAME_API AAICharacter_1 : public ACharacter, public IEscapeCombatDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter_1();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual FCombatDamageResult ApplyDamage_Implementation(const FCombatDamageContext& DamageContext) override;
	void HandleDeath();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<class UAttributeComponent> AttributeComp;

};
