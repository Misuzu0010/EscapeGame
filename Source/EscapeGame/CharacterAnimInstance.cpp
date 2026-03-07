// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WindSimulationComponent.h" // 用于获取风力数据
#include "Kismet/KismetMathLibrary.h" // 用于数学计算

void UCharacterAnimInstance::NativeInitializeAnimation()
{
    UAnimInstance::NativeInitializeAnimation();

    // 缓存所有者，只会执行一次
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    UAnimInstance::NativeUpdateAnimation(DeltaSeconds);

    if (WindComponent)
    {
        // 直接从风力组件获取算好的柏林噪声风力 + 移动风力！
        KawaiiWind = WindComponent->GetCurrentWind();
    }
}

void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
    UAnimInstance::NativeThreadSafeUpdateAnimation(DeltaTime);
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
	}
    FVector Velocity = MovementComponent->Velocity;
    float Speed = Velocity.Size();
    // 3. 计算动态阻尼 (Dynamic Damping)
    // 逻辑：站立时软一点(0.1)，跑起来硬一点(0.5)防止穿模
    float TargetDamping = (Speed > 10.0f) ? 0.4f : 0.1f;
    DynamicDamping = FMath::FInterpTo(DynamicDamping, TargetDamping, DeltaTime, 5.0f);

    // 4. (可选) 如果角色瞬移或死亡，重置 Alpha
    // 这里简单演示：如果速度极快（可能是传送），暂时关闭物理
    if (Speed > 2000.0f)
    {
        PhysicsAlpha = 0.0f;
    }
    else
    {
        PhysicsAlpha = FMath::FInterpTo(PhysicsAlpha, 1.0f, DeltaTime, 2.0f);
    }
}