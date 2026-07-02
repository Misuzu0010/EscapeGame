# 战斗系统完善清单

生成日期：2026-05-25

本文档记录当前 `EscapeGame` 项目战斗系统的后续完善路线。当前战斗系统已有轻击、连击、蓄力、重击释放、Montage 播放、攻击 Trace 和伤害接口雏形；后续目标是先稳定战斗状态框架，再补齐受击、闪避、反馈和敌人闭环。

## 1. 稳定当前战斗状态框架

- [x] 将分散的运行时字段收拢到 `FCombatRuntimeState`。
- [x] 保持轻击、连击、蓄力、重击释放流程不退化。
- [x] 精准管理 Action Tag 生命周期，避免 `ActiveTags.Reset()` 粗暴清理。
- [x] 将 Tag 修改入口统一收口，减少直接操作 `ActiveTags.AddTag/RemoveTag`。
- [ ] 战斗状态与 `UStateMachineComponent` 同步：攻击开始进入 `Attacking`，动作结束恢复 `Idle/Moving`。
- [ ] 明确攻击期间移动、冲刺、蹲伏、跳跃的优先级规则。

## 2. 完善输入与动作规则

- [ ] 将长按阈值 `0.55f` 改为可配置参数。
- [ ] 将 Montage Section 名称 `"Attack"` 改为数据配置或常量。
- [ ] 明确轻击、连击、蓄力、重击、空中攻击、闪避之间的优先级。
- [ ] 建立统一的 `CanStartCombatAction(ActionTag)` 判断入口。
- [ ] 做标准化输入缓存：提前输入是否缓存、缓存多久、在哪个 Combo Window 消耗。
- [ ] 补完整 `Action.Combat.AirAttack` 执行路径。
- [ ] 补完整 `Input.Action.Dodge` 执行路径。

## 3. 规范动画 Notify 流程

- [ ] 建立主项目内正式的 `AnimNotify_DoAttackTrace`。
- [ ] 建立 `AnimNotify_CheckCombo` 或 `ComboWindow_Open/Close`。
- [ ] 明确每个攻击 Montage 的 Trace 帧、连击窗口、释放 Section 和结束清理点。
- [ ] 决定动作结束清理主要依赖 `OnMontageEnded`，还是额外增加 `AnimNotify_EndAction`。
- [ ] 对异常情况做保护：Montage 被打断、Notify 残留、当前动作 Tag 已失效。

## 4. 完善命中与伤害系统

- [ ] 将当前 `ApplyDamage` 参数升级为 `DamageContext` 或类似结构。
- [ ] `DamageContext` 建议包含：伤害值、攻击者、命中目标、命中点、冲击力、动作 Tag、伤害类型、`FHitResult`。
- [ ] 防止同一攻击窗口跨多个 Notify 重复命中同一个目标。
- [ ] 让受击方返回命中结果，便于攻击方确认是否真正造成伤害。
- [ ] 接入 `IEscapeCombatAttacker::NotifyHitConfirmed`，用于 HitStop、吸血、连击奖励等反馈。

## 5. 完善受击、硬直与死亡

- [ ] 给 `AEscapeGameCharacter::ApplyDamage_Implementation` 增加组件判空和死亡保护。
- [ ] 接入 `State.Status.Invincible`、`State.Status.HyperArmor`、`State.Status.Blocking`。
- [ ] 增加轻受击、重受击、击飞、击倒、眩晕等受击反应。
- [ ] 让死亡流程播放 Montage，禁用输入和移动，必要时禁用碰撞。
- [ ] 死亡时清理战斗运行时状态，避免死亡后残留攻击 Tag。

## 6. 实现闪避系统

- [ ] 使用 `GeneralActionMap` 或独立 `DodgeActionDefinition` 驱动闪避 Montage。
- [ ] 闪避消耗体力，并使用 `Cooldown.Dodge` 控制冷却。
- [ ] 闪避期间添加 `State.Status.Invincible`。
- [ ] 闪避结束后清理无敌和闪避 Tag。
- [ ] 明确闪避能否打断轻击、蓄力、重击、受击。

## 7. 补敌人与训练假人闭环

- [ ] 创建主项目内的 `ACombatDummy` 或 `AEnemyCharacter`。
- [ ] 实现 `UEscapeCombatDamageable`，支持扣血、受击反馈和死亡。
- [ ] 先用训练假人验证玩家攻击闭环，不急于接复杂 AI。
- [ ] 后续再接入追击、攻击、巡逻和 StateTree。
- [ ] 敌人攻击也应复用同一套伤害接口和受击接口。

## 8. 打击反馈与表现

- [ ] 使用 `FCombatActionDefinition` 中的 `HitStopDuration`。
- [ ] 使用 `CameraShakeScale` 驱动命中 Camera Shake。
- [ ] 使用 `HitImpactAffect` 生成 Niagara 命中特效。
- [ ] 区分命中敌人、命中盾牌、命中环境、未命中。
- [ ] 增加 Debug Trace 显示开关，用于调攻击距离和半径。
- [ ] 后续可加入命中音效、材质闪白、受击方向反馈。

## 9. 数据资产整理

- [ ] 清理 `FCombatActionDefinition` 字段命名，例如 `DodgeStaminaCost` 不应放在所有攻击动作里。
- [ ] 补充攻击体力消耗、可否移动、可否转向、可否被打断、Root Motion 规则。
- [ ] 为 Player 和 Enemy 分别建立 `UCharacterAnimData` 配置。
- [ ] 增加 DataAsset 校验：Montage 是否为空、NextComboTag 是否存在、Section 是否存在。
- [ ] 给 `HitStopDuration`、`CameraShakeScale` 等字段补默认值，消除 UE 启动时的未初始化警告。

## 10. 调试与测试

- [ ] 建立战斗测试地图：玩家、训练假人、可视化 Trace。
- [ ] 测试轻击四连、蓄力释放、攻击中跳跃/冲刺拦截、体力不足、死亡状态。
- [ ] 增加战斗日志分类，逐步替换大量 `LogTemp`。
- [ ] 给 `FCombatRuntimeState` 提供调试输出，能打印当前动作 Tag、Combo、缓存输入和 ActiveTags。
- [ ] 做一份手动 QA 表，避免每次调动画时漏测核心流程。

## 推荐实施顺序

1. 完成 Action Tag 精准清理。
2. 同步战斗状态到 `UStateMachineComponent`。
3. 建立 `CanStartCombatAction()` 统一动作入口规则。
4. 规范攻击 Notify。
5. 完成受击和训练假人闭环。
6. 实现闪避系统。
7. 加入 HitStop、Camera Shake、Niagara 等打击反馈。

## 当前阶段验收重点

- 快速点击攻击：第一段轻击能正常播放。
- 连续点击攻击：输入缓存能触发下一段连击。
- 长按攻击：能进入蓄力。
- 松开长按：能跳到攻击 Section 并释放重击。
- Montage 结束后：能重新起手。
- 当前动作 Tag：切段时旧动作 Tag 被清理，新动作 Tag 被记录。
- 攻击会话 Tag：`Action.State.Attacking` 在连击期间保留，动作彻底结束后清理。
- `DoAttackTrace()`：仍能根据当前动作 Tag 找到 `FCombatActionDefinition`。
