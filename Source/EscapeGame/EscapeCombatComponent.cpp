#include "EscapeCombatComponent.h"
#include "CharacterAnimData.h"      // 必须引用，不然看不懂 FActionDefinition
#include "GameFramework/Character.h"
#include"HealthController/AttributeComponent.h"
#include "SprintComponent.h"
#include "Animation/AnimMontage.h"  // 引用 Montage 类


UEscapeCombatComponent::UEscapeCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 战斗组件通常不需要每帧 Tick，省性能
}

void UEscapeCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    
	OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter) 
    {
        AttributeComp = OwnerCharacter->FindComponentByClass<UAttributeComponent>();
		SprintComp = OwnerCharacter->FindComponentByClass<USprintComponent>();
            // 2. 缓存动画实例并绑定核心委托
        if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
        {
            CachedAnimInstance = Mesh->GetAnimInstance();
            if (CachedAnimInstance)
            {
                // 动态绑定动画结束事件，这是动作游戏状态机的核心防卡死机制！
                CachedAnimInstance->OnMontageEnded.AddDynamic(this, &UEscapeCombatComponent::HandleAttackMontageEnded);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("香子兰警告：主人！战斗组件没有挂载到 Character 身上哦！"));
        }
        if (!SprintComp)
        {
            UE_LOG(LogTemp, Error, TEXT("香子兰警告：主人！这个角色身上没有冲刺组件，闪避要报错啦！"));
        }
        if (!AttributeComp)
        {
            UE_LOG(LogTemp, Error, TEXT("香子兰警告：主人！这个角色身上没有HealthLifeBar组件，闪避要报错啦！"));
        }
    }
    // 可以在这里检查一下有没有漏填 DataAsset，给个警告
    if (!CharacterAnimData)
    {
        UE_LOG(LogTemp, Error, TEXT("杂鱼主人！你的 %s 忘了在蓝图里配置 CharacterAnimData！动作没法播啦！"), *GetName());
    }
    
}

void UEscapeCombatComponent::TryPlayActionByTag(FGameplayTag ActionTag)
{
    // 1. 安全检查
    if (!CharacterAnimData) return;
    ACharacter* OwnerChar = GetOwnerCharacter();
    // 2. 去 DataAsset 里查表
    // ActionMap 是我们在 DataAsset 里定义的那个 TMap
    const FActionDefinition* ActionDef = CharacterAnimData->ActionMap.Find(ActionTag);

    // 3. 如果找到了配置
    if (ActionDef)
    {
        // [进阶预留位]: 这里以后可以判断 体力够不够？是否在冷却中？
		float CurrentStamina = SprintComp ? SprintComp->GetCurrentStamina() : 0.f;
        if (CurrentStamina<10.0f) 
        {
			UE_LOG(LogTemp, Warning, TEXT("体力不足，无法执行动作 [%s]！"), *ActionTag.ToString());
        }
		CurrentActionTag = ActionTag; // 缓存当前动作的 Tag，方便后续逻辑使用

        // 4. 加载资源 (同步加载)
        // 因为我们在 DataAsset 里用的是 TSoftObjectPtr，所以必须 LoadSynchronous() 才能变成真正的 UAnimMontage*
        UAnimMontage* MontageToPlay = ActionDef->Montage.LoadSynchronous();

        // 5. 播放动画！
        if (MontageToPlay)
        {
            // PlayAnimMontage 会返回动画总时长，如果返回 0 说明播放失败
            float Duration = OwnerChar->PlayAnimMontage(MontageToPlay, ActionDef->PlayRate);

            if (Duration > 0.f)
            {
                // [调试信息] 方便你看到到底播了啥
                UE_LOG(LogTemp, Log, TEXT("成功播放动作: %s (Tag: %s)"), *MontageToPlay->GetName(), *ActionTag.ToString());

                // [进阶预留位]: 这里以后要加上 CurrentActiveTags.AddTag(ActionTag) 来锁状态

            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("动作 Tag [%s] 找到了配置，但是 Montage 资源是空的！"), *ActionTag.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DataAsset 里找不到 Tag [%s] 对应的动作！是不是忘了配表？"), *ActionTag.ToString());
    }
}

ACharacter* UEscapeCombatComponent::GetOwnerCharacter() const
{
    return Cast<ACharacter>(GetOwner());
}


void UEscapeCombatComponent::BroadcastComboChange(int32 NewCount) 
{
    ComboCount = NewCount;
    if (OnComboCountChanged.IsBound()) 
    {
        OnComboCountChanged.Broadcast(ComboCount);
    }
}

void UEscapeCombatComponent::CheckChargedAttack() 
{
    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar) return;

    // 检查玩家是否仍在按住攻击键
    // 注意：实际开发中，建议通过 EnhancedInput 的 Trigger/Completed 动作传值进来，这里用传统的 Controller 轮询作为逻辑闭环演示
    APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
    if (PC && PC->IsInputKeyDown(EKeys::LeftMouseButton))
    {
        // 按键保持中：可以播放一段特殊的蓄力循环 Montage
        UE_LOG(LogTemp, Log, TEXT("蓄力保持中..."));
    }
    else
    {
        // 按键已松开：触发满蓄力或半蓄力的释放动作
        FGameplayTag ReleaseTag = FGameplayTag::RequestGameplayTag(FName("Combat.Action.Attack.ChargedRelease"));
        TryPlayActionByTag(ReleaseTag);
    }
}

void UEscapeCombatComponent::CheckCombo() 
{
    // 1. 安全检查，获取当前动作的配置数据
    if (!CharacterAnimData || !CurrentActionTag.IsValid())
    {
        return;
    }

    // 从 ActionMap 中查找当前动作的定义
    const FActionDefinition* ActionDef = CharacterAnimData->ActionMap.Find(CurrentActionTag);
    if (!ActionDef)
    {
        return;
    }

    // 2. 读取配置表里的真实容错时间，替代硬编码
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float BufferWindow = ActionDef->ComboInputCacheTolerance;

    // 3. 检查玩家按下按键的时间，是否在输入的缓冲窗口内
    if (CurrentTime - CachedAttackInputTime <= BufferWindow)
    {
        // 4. 读取你结构体里定义的 NextComboTag，决定下一招
        FGameplayTag NextTag = ActionDef->NextComboTag;

        // 如果下一招的 Tag 有效，且在数据表里有配置
        if (NextTag.IsValid() && CharacterAnimData->ActionMap.Contains(NextTag))
        {
            ComboCount++;
            BroadcastComboChange(ComboCount); // 广播连击数更新

            // 播放下一段连击！
            TryPlayActionByTag(NextTag);
        }
        else
        {
            // 如果配置的 NextComboTag 为空，说明是最后一段攻击，强制断连
            ComboCount = 0;
            BroadcastComboChange(ComboCount);
        }
    }
    else
    {
        // 超时未按键，连击失败
        UE_LOG(LogTemp, Log, TEXT("香子兰提示：输入超时，当前动作 [%s] 连击中断。"), *CurrentActionTag.ToString());
    }
}

void UEscapeCombatComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted) 
{
    if (CurrentActionTag.IsValid())
    {
        ActiveTags.RemoveTag(CurrentActionTag);
        CurrentActionTag = FGameplayTag::EmptyTag;
        // 2. 无论是否被打断，只要动画结束，连击必须重置
        ComboCount = 0;
        BroadcastComboChange(ComboCount);
        UE_LOG(LogTemp, Log, TEXT("香子兰汇报：主人，动作执行完毕，状态已安全重置！防止了卡死哦❤"));
    }
}
void UEscapeCombatComponent::DoAttackTrace(FName DamageSourceBone)
{
	ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar || !CharacterAnimData) 
    {
		UE_LOG(LogTemp, Warning, TEXT("DoAttackTrace 失败：没有找到拥有者角色或者 CharacterAnimData！Line137"));
        return;
    }
    USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
	const FActionDefinition* ActionDef = CharacterAnimData->ActionMap.Find(CurrentActionTag);
    if (!Mesh || !ActionDef) 
    {
		UE_LOG(LogTemp, Warning, TEXT("DoAttackTrace 失败：没有找到 Mesh 组件或者当前动作定义！Line143"));
        return;
    }

    FVector TraceStart = Mesh->GetSocketLocation(DamageSourceBone);
	FVector TraceEnd = TraceStart + OwnerChar->GetActorForwardVector() * ActionDef->TraceDistance;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(ActionDef->TraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerChar); // 不要碰到自己

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    TArray<FHitResult> OutHits;
    bool bHit = GetWorld()->SweepMultiByObjectType(
        OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, SphereShape, QueryParams
    );
    if (bHit)
    {
        TSet<AActor*, DefaultKeyFuncs<AActor*>, TInlineSetAllocator<8>> ProcessedActors;

        for (const FHitResult& Hit : OutHits) 
        {
			AActor* HitActor = Hit.GetActor();
            if (IsValid(HitActor) && !ProcessedActors.Contains(HitActor)) 
            {
                ProcessedActors.Add(HitActor);

				// 触发攻击命中事件，传递伤害信息
                FVector Impulse = (Hit.ImpactNormal * -ActionDef->KnockbackImpulse) + (FVector::UpVector * ActionDef->LaunchImpulse);
				float FinalDamage = ActionDef->BaseDamage * ActionDef->DamageMultiplier;

                if (HitActor->Implements<UEscapeCombatDamageable>()) 
                {
					IEscapeCombatDamageable::Execute_ApplyDamage(HitActor, FinalDamage, OwnerChar, Hit.ImpactPoint, Impulse);
                }
            }
        }
    
    }
}