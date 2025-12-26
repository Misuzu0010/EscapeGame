// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InterectComponent.generated.h"
struct FInputActionValue;



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRequestToggleInventory);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API UInterectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInterectComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 2. 声明回调函数 (按下 E 时执行的逻辑)
	UFUNCTION()
	void OnInteract(const FInputActionValue& Value);

	UPROPERTY(BlueprintAssignable)
	FOnRequestToggleInventory OnRequestToggleInventory;

	UFUNCTION()
	void RequestToggleInventory();


		
};
