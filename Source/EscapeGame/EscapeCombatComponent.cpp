#include "EscapeCombatComponent.h"
#include "CharacterAnimData.h"      // 必须引用，不然看不懂 FCombatActionDefinition
#include "GameFramework/Character.h"
#include"HealthController/AttributeComponent.h"
#include "SprintComponent.h"
#include "EscapeGameplayTags.h"
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
    TryPlayActionByTagInternal(ActionTag);
}

bool UEscapeCombatComponent::TryPlayActionByTagInternal(FGameplayTag ActionTag)
{
    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!CharacterAnimData)
    {
        UE_LOG(LogTemp, Warning, TEXT("无法执行动作 [%s]：CharacterAnimData 未配置！"), *ActionTag.ToString());
        return false;
    }

    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("无法执行动作 [%s]：没有找到拥有者 Character！"), *ActionTag.ToString());
        return false;
    }

    // 2. 去 DataAsset 里查表
    // ActionMap 是我们在 DataAsset 里定义的那个 TMap
    const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(ActionTag);

    // 3. 如果找到了配置
    if (!ActionDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("DataAsset 里找不到 Tag [%s] 对应的动作！是不是忘了配表？"), *ActionTag.ToString());
        return false;
    }

    // [进阶预留位]: 这里以后可以判断 体力够不够？是否在冷却中？
	float CurrentStamina = SprintComp ? SprintComp->GetCurrentStamina() : 0.f;
    if (CurrentStamina < 10.0f)
    {
		UE_LOG(LogTemp, Warning, TEXT("体力不足，无法执行动作 [%s]！"), *ActionTag.ToString());
        return false;
    }

    // 4. 加载资源 (同步加载)
    // 因为我们在 DataAsset 里用的是 TSoftObjectPtr，所以必须 LoadSynchronous() 才能变成真正的 UAnimMontage*
    UAnimMontage* MontageToPlay = ActionDef->Montage.LoadSynchronous();
    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("动作 Tag [%s] 找到了配置，但是 Montage 资源是空的！"), *ActionTag.ToString());
        return false;
    }

    // 5. 播放动画！
    // PlayAnimMontage 会返回动画总时长，如果返回 0 说明播放失败
    const float Duration = OwnerChar->PlayAnimMontage(MontageToPlay, ActionDef->PlayRate);
    if (Duration <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("动作 Montage [%s] 播放失败！Tag: %s"), *MontageToPlay->GetName(), *ActionTag.ToString());
        return false;
    }

    // 只有 Montage 真正播放成功后，才提交当前动作状态。
    CurrentActionTag = ActionTag;
    CurrentPlayingMontage = MontageToPlay;
    ActiveTags.AddTag(ActionTag);
    UE_LOG(LogTemp, Log, TEXT("成功播放动作: %s (Tag: %s)"), *MontageToPlay->GetName(), *ActionTag.ToString());

    return true;
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
/*
	以下两个函数是用于处理蓄力攻击的，蓝图 Enhanced Input 的 Started/Triggered 引脚调用 BeginOrUpdateChargedAttack，Completed/Canceled 引脚调用 ReleaseChargedAttack
*/
void UEscapeCombatComponent::BeginOrUpdateChargedAttack() 
{
    // 【核心排雷】：如果身上已经有“蓄力”或“蓄力释放”的 Tag，直接拦截！
    if (ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge) || 
        ActiveTags.HasTag(EscapeGameplayTags::Action_ChargedAttack_Release))
    {
        return; 
    }

    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar) return;
    
    if (TryPlayActionByTagInternal(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        // 蓄力 Montage 确认播放成功后，再正式切换状态。
        ActiveTags.RemoveTag(EscapeGameplayTags::Action_State_Attacking);
        bHasSavedComboInput = false;
        UE_LOG(LogTemp, Log, TEXT("香子兰汇报：成功拦截每帧调用，开始单次蓄力！"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("蓄力动作播放失败，未进入蓄力状态。"));
    }
}
void UEscapeCombatComponent::ReleaseChargedAttack()
{
    // 【换锁】：检查身上是否有 蓄力中 的 Tag。如果没有，说明根本没在蓄力，无视松开按键的指令。
    if (!ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge)) 
    {
        return; 
    }

    // 【状态切换】：撕掉蓄力的标签，贴上释放重击的标签！
    ActiveTags.RemoveTag(EscapeGameplayTags::Action_Combat_Heavy_Charge);
    ActiveTags.AddTag(EscapeGameplayTags::Action_ChargedAttack_Release); 

    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar || !CharacterAnimData || !CurrentActionTag.IsValid()) return;

    UAnimInstance* AnimInstance = OwnerChar->GetMesh() ? OwnerChar->GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance) return;

    const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(CurrentActionTag);
    if (ActionDef)
    {
        UAnimMontage* CurrentMontage = ActionDef->Montage.Get(); 
        if (CurrentMontage && AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            // 跳转到 Montage 里名为 "Attack" 的 Section
            AnimInstance->Montage_JumpToSection(FName("Attack"), CurrentMontage);
            UE_LOG(LogTemp, Log, TEXT("香子兰汇报：主人松手了，蓄力结束，正在狠狠地斩向敌人！"));
        }
    }
}
void UEscapeCombatComponent::CheckCombo() 
{
    // 【换锁】：用 Tag 替换 Enum！如果没有 Attacking 标签，说明被打断了，直接拦截！
    if (!ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking))
    {
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：检测到幽灵 Notify 或未处于攻击状态，已成功拦截连击判定！"));
        return;
    }

    if (!CharacterAnimData || !CurrentActionTag.IsValid()) return;
    const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(CurrentActionTag);
    if (!ActionDef) return;

    if (bHasSavedComboInput)
    {
        bHasSavedComboInput = false; 
        
        FGameplayTag NextTag = ActionDef->NextComboTag;
        if (NextTag.IsValid() && CharacterAnimData->CombatActionMap.Contains(NextTag))
        {
            if (TryPlayActionByTagInternal(NextTag))
            {
                ComboCount++;
                BroadcastComboChange(ComboCount);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("下一段连击播放失败，等待当前 Montage 正常结束。"));
            }
        }
        else
        {
            // 已经是最后一段了，重置连击计数
            ComboCount = 0;
            BroadcastComboChange(ComboCount);
            
            // 注意：不要在这里直接 RemoveTag(Attacking)！
            // 让 HandleAttackMontageEnded 在动画彻底播完后去清理，这样收刀动作才有防打断保护！
        }
    }
}

void UEscapeCombatComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted) 
{
    if (Montage != CurrentPlayingMontage) return; 

    // 【解锁】：动画结束，无脑撕掉所有战斗相关的 Tag！
    ActiveTags.RemoveTag(EscapeGameplayTags::Action_State_Attacking);
    ActiveTags.RemoveTag(EscapeGameplayTags::Action_Combat_Heavy_Charge);
    ActiveTags.RemoveTag(EscapeGameplayTags::Action_ChargedAttack_Release);
    
    bHasSavedComboInput = false;
    CurrentActionTag = FGameplayTag::EmptyTag;
    CurrentPlayingMontage = nullptr;
    ComboCount = 0;
    BroadcastComboChange(ComboCount);
}
void UEscapeCombatComponent::DoAttackTrace(FName DamageSourceBone)
{
	ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar || !CharacterAnimData) 
    {
		UE_LOG(LogTemp, Warning, TEXT("DoAttackTrace 失败：没有找到拥有者角色或者 CharacterAnimData！"));
        return;
    }
    USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
	const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(CurrentActionTag);
    if (!Mesh || !ActionDef) 
    {
		UE_LOG(LogTemp, Warning, TEXT("DoAttackTrace 失败：没有找到 Mesh 组件或者当前动作定义！"));
        return;
    }
	// 1. 根据当前动作的配置，计算追踪的起点和终点
    FVector TraceStart = Mesh->GetSocketLocation(DamageSourceBone);
	FVector TraceEnd = TraceStart + OwnerChar->GetActorForwardVector() * ActionDef->TraceDistance;
	// 2. 使用 Sphere Trace 来检测攻击范围内的敌人，半径也从配置表里读
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(ActionDef->TraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerChar); // 不要碰到自己
    
	// 3. 只检测 Pawn 和 WorldDynamic 两种类型的物体，效率更高
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    
	// 4. 执行 SweepMultiByObjectType，获取所有被击中的对象
    TArray<FHitResult> OutHits;
    bool bHit = GetWorld()->SweepMultiByObjectType(
        OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, SphereShape, QueryParams
    );
    
	// 5. 遍历所有被击中的对象，应用伤害和击退效果
    if (bHit)
    {
        TSet<AActor*, DefaultKeyFuncs<AActor*>, TInlineSetAllocator<8>> ProcessedActors;

        for (const FHitResult& Hit : OutHits) 
        {
			AActor* HitActor = Hit.GetActor();
            if (IsValid(HitActor) && !ProcessedActors.Contains(HitActor)) 
            {
                ProcessedActors.Add(HitActor);

				//1,触发攻击命中事件，传递伤害信息
                FVector Impulse = (Hit.ImpactNormal * -ActionDef->KnockbackImpulse) + (FVector::UpVector * ActionDef->LaunchImpulse);
				float FinalDamage = ActionDef->BaseDamage * ActionDef->DamageMultiplier;

                if (HitActor->Implements<UEscapeCombatDamageable>()) 
                {
					IEscapeCombatDamageable::Execute_ApplyDamage(HitActor, FinalDamage, OwnerChar, Hit.ImpactPoint, Impulse);
                }
                // 2. 组装打击情报包 (Payload)
                FAttackHitPayload Payload;
                Payload.DamageCauser = HitActor;         // 砍到了谁
                Payload.DamageLocation = Hit.ImpactPoint; // 砍到了哪里 (用于在这里生成飙血特效 Niagara)
                Payload.DamageImpulse = Impulse;          // 冲击力方向

                // 3. 告诉【玩家自己】：我砍中啦！快播放打击反馈！
                if (OnAttackHit.IsBound())
                {
                    OnAttackHit.Broadcast(Payload);
                    UE_LOG(LogTemp, Log, TEXT("香子兰汇报：武器命中目标！已发送 OnAttackHit 广播！"));
                }
            }
        }
    
    }
}
void UEscapeCombatComponent::RequestLightAttack()
{
    // 如果没有任何攻击和蓄力状态，允许起手
    if (!ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking) && 
        !ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        // 【上锁】：给自己贴上“正在攻击”的标签
        
        FGameplayTag FirstAttackTag = EscapeGameplayTags::Action_Combat_Light_1;
        if (TryPlayActionByTagInternal(FirstAttackTag))
        {
            ActiveTags.AddTag(EscapeGameplayTags::Action_State_Attacking);
            UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：贴上了攻击Tag，成功打出第一段轻击！"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("不好，现在并没有进入攻击状态，请检查DataAsset是否配置正确"));
        }
    }
    // 如果已经在轻击了，开启输入缓冲
    else if (ActiveTags.HasTagExact(EscapeGameplayTags::Action_State_Attacking))
    {
        bHasSavedComboInput = true;
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：动作中检测到输入，锁存开启！"));
    }
}


// ==========================================
// 核心逻辑实现：
// ==========================================
void UEscapeCombatComponent::Input_AttackStarted()
{
    // 【核心机密】：按下的瞬间，我们什么攻击都不做！
    // 而是定一个 0.25 秒的闹钟。如果 0.25 秒后主人还没松手，闹钟就会自动触发“重击蓄力”！
    GetWorld()->GetTimerManager().SetTimer(
        InputBufferTimer,
        this,
        &UEscapeCombatComponent::BeginOrUpdateChargedAttack,
        0.55f, // 这个时间就是你的“长按判定阈值”，可以随便微调
        false
    );
    UE_LOG(LogTemp, Log, TEXT("香子兰汇报：检测到按下！已开启 0.55秒的输入缓冲闹钟..."));
}

void UEscapeCombatComponent::Input_AttackCompleted()
{
    // 玩家松开左键了！我们来看看闹钟的情况：

    // 【情况 1：闹钟还在滴答滴答走！】
    // 说明从按下到松手，连 0.25 秒都没到。这绝对是一个快速的【轻击 (Tap)】！
    if (GetWorld()->GetTimerManager().IsTimerActive(InputBufferTimer))
    {
        // 杂鱼！想骗我出重击？没门！掐死准备触发重击的闹钟！
        GetWorld()->GetTimerManager().ClearTimer(InputBufferTimer);

        // 立刻判定为轻击请求！(这里会自然走进你之前写好的查 Tag 第一段攻击，或 bHasSavedComboInput 的连击逻辑)
        RequestLightAttack();
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：判定为【轻击】，闹钟已掐死！"));
    }
    // 【情况 2：闹钟已经不在走了】
    // 说明 0.25 秒前，闹钟已经响过了（已经自动执行了 BeginOrUpdateChargedAttack）。
    // 此时玩家松手，说明这是【蓄力结束，狠狠释放重击】！
    else if (ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        ReleaseChargedAttack();
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：判定为【蓄力松手】，重击斩出！"));
    }
}
