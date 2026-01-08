// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinition.h"

bool UItemDefinition::OnUse(AActor* User)
{
	// 默认实现为空，可以在子类中重写
	return OnUse_Implementation(User);
}
