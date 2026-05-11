// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WindSimulationComponent.h" // 用于获取风力数据
#include "Kismet/KismetMathLibrary.h" // 用于数学计算
#include"ClothLODControllerComponent.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
    UAnimInstance::NativeInitializeAnimation();
    // 缓存所有者，只会执行一次
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();

        WindComponent = OwnerCharacter->FindComponentByClass<UWindSimulationComponent>();

        // 【新增】获取我们的布料LOD控制器
		ClothLODComponent = OwnerCharacter->FindComponentByClass<UClothLODControllerComponent>();
    }
}

// ==========================================
// 基础写法：在游戏线程更新 (Game Thread)
// 逻辑：【绝对安全】在这里只做一件事：从组件提取数据并存入 Cached 变量！绝不做复杂运算！
// ==========================================

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    UAnimInstance::NativeUpdateAnimation(DeltaSeconds);

    // 1. 安全提取速度，存入普通变量
    if (MovementComponent)
    {
        CachedVelocity = MovementComponent->Velocity;
		VerticalVelocity = CachedVelocity.Z;
		GroundSpeed = CachedVelocity.Size2D();
        bIsFalling = MovementComponent->IsFalling();
		bShouldMove = (MovementComponent->GetCurrentAcceleration().Size2D() > 0.f) && (GroundSpeed > 3.0f);
		bIsRunning = bShouldMove && (GroundSpeed > 300.0f); // 300 是一个经验值，可以根据需要调整
        CachedRotation = OwnerCharacter->GetActorRotation();
    }

    // 2. 安全提取风力，弱指针必须用 IsValid() 判断
    if (WindComponent.IsValid())
    {
		float KawaiiMultiplier = 5.0f;// 这个值可以根据需要调整，增加风力的影响程度
        KawaiiWind = WindComponent->GetCurrentWind()*KawaiiMultiplier;
    }
    if (ClothLODComponent.IsValid())
    {
        CachedClothLODFactor = ClothLODComponent->GetLODFactor();
    }
}

void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
    UAnimInstance::NativeThreadSafeUpdateAnimation(DeltaTime);
    
    float Speed = CachedVelocity.Size2D();
    float TargetDamping = (Speed > 10.0f) ? DynamicDampingForIdle : DynamicDampingForMoving;
    DynamicDamping = FMath::FInterpTo(DynamicDamping, TargetDamping, DeltaTime, 5.0f);
    float TargetPhysicsAlpha = 1.0f;
	//传送检测：如果速度过快，直接关闭物理模拟，防止布料乱飞
    if (Speed > 2000.0f)
    {
        PhysicsAlpha = 0.0f;
    }
    else
    {
        // 使用缓存的 LOD Factor，不再调用外部组件的方法
        float LODMultiplier = 1.0f - CachedClothLODFactor;
        TargetPhysicsAlpha *= LODMultiplier;
    }
    PhysicsAlpha = FMath::FInterpTo(PhysicsAlpha, TargetPhysicsAlpha, DeltaTime, 2.0f);
    
    if (GroundSpeed > 3.0f)
    {
        // 限制倍率范围，防止动画抽搐 (0.5倍到2.0倍之间)
        LocomotionPlayRate = FMath::Clamp(GroundSpeed / AuthoredRunSpeed, 0.5f, 2.0f);
    }
    else
    {
        LocomotionPlayRate = 1.0f;
    }
    // 2. 线程安全的局部移动角度计算 (替代 CalculateDirection)
    // 逻辑：使用四元数逆变换，将世界空间的 Velocity 转换到角色的局部空间
    if (GroundSpeed > 3.0f)
    {
        FVector LocalVelocity = CachedRotation.UnrotateVector(CachedVelocity);
        
        // 利用 Atan2 计算出 X 和 Y 的夹角，并转换为角度 (-180 到 180)
        // 这样计算完全脱离了引擎的 Actor 引用，完美线程安全！
        LocomotionAngle = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
    }
};