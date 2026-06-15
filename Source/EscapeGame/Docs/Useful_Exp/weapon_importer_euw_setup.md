# EUW_WeaponImporter Setup

本文档说明如何在 UE Editor 中创建 `EUW_WeaponImporter`，并通过 Editor Utility Widget 调用 C++ 后端函数 `ImportWeaponFromObjFolder`。该 Widget 是一次性的手动编辑器资产设置；`.uasset` 不适合用文本补丁生成。

## 1. Create Widget

1. Open UE Editor。
2. 在 Content Browser 中创建文件夹 `/Game/EditorTools`。
3. 在 `/Game/EditorTools` 中 Right click -> Editor Utilities -> Editor Utility Widget。
4. 将新建的 Editor Utility Widget 命名为 `EUW_WeaponImporter`。
5. 打开 `EUW_WeaponImporter`，切换到 Designer / Graph，准备添加输入控件和按钮逻辑。

## 2. Add Inputs

在 `EUW_WeaponImporter` 中添加以下 Widget variables。建议每个变量都暴露到界面输入控件，方便导入不同武器时修改参数。

| Variable | Type | Default |
| --- | --- | --- |
| `SourceFolder` | String | 空，填写包含 OBJ/材质/贴图源文件的本地文件夹路径 |
| `DestinationPath` | String | `/Game/Weapons/Sword_Darker` |
| `AssetBaseName` | String | `Sword_Darker` |
| `BaseDamage` | Float | `20` |
| `TraceRadius` | Float | `12` |
| `AttachSocketName` | Name | `WeaponSocket` |
| `TraceStartSocketName` | Name | `TraceStart` |
| `TraceEndSocketName` | Name | `TraceEnd` |

推荐界面布局：

1. 使用 `Vertical Box` 作为主容器。
2. 为每个 String / Name 参数添加 Label 和 TextBox。
3. 为 `BaseDamage`、`TraceRadius` 添加 SpinBox 或 Numeric Entry。
4. 确保每个输入控件的值会同步到对应变量，或者在点击按钮时从控件读取并赋值给对应变量。

## 3. Add Button Logic

1. 添加一个 Button，命名为 `Import`。
2. 为 `Import` Button 创建 `On Clicked` 事件。
3. 在 `On Clicked` 中调用 C++ 后端函数 `ImportWeaponFromObjFolder`。
4. 将 Widget variables 全部传入函数：
   - `SourceFolder`
   - `DestinationPath`
   - `AssetBaseName`
   - `BaseDamage`
   - `TraceRadius`
   - `AttachSocketName`
   - `TraceStartSocketName`
   - `TraceEndSocketName`
5. 读取函数返回结果中的 `Messages`，遍历每一个 entry，并用 `Print String` 或日志节点输出到 Output Log。
6. 如果返回结果中的 `bSucceeded` 为 true，额外打印 `WeaponDefinitionObjectPath`，用于确认生成的 `DA_Weapon_*` 资产路径。

Blueprint 逻辑建议：

1. `Import.OnClicked`
2. 从输入控件读取当前值，更新 Widget variables。
3. 调用 `ImportWeaponFromObjFolder`。
4. `For Each Loop` 遍历 returned `Messages`。
5. 每条 `Messages` entry 调用 `Print String`，并启用 Print to Log。
6. `Branch` 判断 `bSucceeded`。
7. true 分支调用 `Print String` 输出 `WeaponDefinitionObjectPath`。

## 4. Use Result

导入完成后，继续在 UE Editor 中检查和接入生成资产：

1. 打开生成的 Static Mesh。
2. 检查并根据需要添加或调整 `TraceStart` 和 `TraceEnd` sockets。
3. 打开 player character blueprint。
4. 将 `DefaultWeaponDefinition` 设置为生成的 `DA_Weapon_*`。
5. 运行 PIE。
6. 测试攻击，确认 weapon attack trace 可以从 `TraceStart` 到 `TraceEnd` 正确生效，并且 `TraceRadius` 符合预期。

## Quick Checklist

- `EUW_WeaponImporter` 位于 `/Game/EditorTools`。
- `SourceFolder` 指向正确的本地 OBJ 源文件夹。
- `DestinationPath` 使用 `/Game/...` 形式的 Content Browser 路径。
- `AssetBaseName` 与武器命名一致，例如 `Sword_Darker`。
- `Import` Button 的 `On Clicked` 会调用 `ImportWeaponFromObjFolder`。
- `Messages` 会逐条打印到 Output Log。
- `bSucceeded` 为 true 时会打印 `WeaponDefinitionObjectPath`。
- 生成的 Static Mesh 已设置 `TraceStart` / `TraceEnd` sockets。
- player character blueprint 的 `DefaultWeaponDefinition` 已指向生成的 `DA_Weapon_*`。

## 5. Lessons From First Manual Setup

### 5.1 Input path rules

`SourceFolder` 是 Windows 本地文件夹路径，例如：

```text
D:\各种游戏的项目资源\weapon\单手剑（OBJ模型）\剑_Sashimi
```

`DestinationPath` 是 UE Content Browser 路径，必须使用 `/Game/...`，例如：

```text
/Game/ThirdPerson/Weapon/Sword_Sashimi
```

建议每把武器使用独立子目录，避免多次导入时资产混在一起。`AssetBaseName` 尽量使用英文、数字和下划线，例如 `Sword_Sashimi`。

### 5.2 EditableTextBox conversion

如果界面全部使用 `EditableTextBox`，点击导入按钮时需要做类型转换：

```text
EditableTextBox -> GetText -> ToString -> String To Name
```

用于：

```text
AttachSocketName
TraceStartSocketName
TraceEndSocketName
```

Float 参数使用：

```text
EditableTextBox -> GetText -> ToString -> String To Float
```

用于：

```text
BaseDamage
TraceRadius
```

注意：`Hint Text` 只是灰色提示，不是实际输入值。Socket 名称文本框最好设置真实默认 `Text`：

```text
WeaponSocket
TraceStart
TraceEnd
```

否则空字符串转成 `FName` 后可能得到 `None`，导致生成的 `UWeaponDefinition` 不使用预期默认 socket。

### 5.3 Reading FWeaponImportResult in Blueprint

`ImportWeaponFromObjFolder` 返回 `FWeaponImportResult`。Blueprint 中优先从 `Return Value` 引脚拖线，搜索：

```text
Break Weapon Import Result
```

拆出：

```text
bSucceeded
StaticMeshObjectPath
MaterialObjectPath
WeaponDefinitionObjectPath
Messages
```

推荐连接：

```text
Messages -> ForEachLoop -> Print String
ForEachLoop Completed -> Branch(bSucceeded)
Branch True -> Print String(WeaponDefinitionObjectPath)
```

如果找不到 Break 节点，先执行 `File -> Refresh All Nodes` 并 Compile Widget；仍然找不到时重启 UE Editor。

### 5.4 Widget layout

`Vertical Box` 会自动从上到下排列，不能自由拖动子控件。表单布局推荐：

```text
Vertical Box
  Horizontal Box
    TextBlock
    EditableTextBox
```

设置建议：

- `Horizontal Box` 的 `Vertical Box Slot.Padding` 设置为 `0, 4, 0, 4`。
- 左侧 `TextBlock.Min Desired Width` 设置为 `160` 或 `180`。
- 右侧 `EditableTextBox` 的 `Horizontal Box Slot.Size` 设置为 `Fill`，`Fill Width` 设置为 `1`。
- `ImportButton` 放到底部，`Padding` 设置为 `0, 12, 0, 0`。

### 5.5 Socket responsibilities

`AttachSocketName` 指角色 Skeletal Mesh / Skeleton 上的武器挂载 socket。默认：

```text
WeaponSocket
```

它通常加在右手主骨骼上，例如 `hand_r`、`右手首` 等，而不是加在手指骨骼上。

`TraceStartSocketName` 和 `TraceEndSocketName` 指武器 Static Mesh 上的伤害检测 sockets。默认：

```text
TraceStart
TraceEnd
```

推荐摆放：

```text
TraceStart: 刀刃靠近护手的位置
TraceEnd: 刀尖位置
```

Combat trace 会使用 `TraceStart -> TraceEnd` 这条线段，并结合 `TraceRadius` 做伤害 sweep。

### 5.6 Adding WeaponSocket to the character

1. 打开角色 Skeletal Mesh 或 Skeleton，例如 `SK_尤诺`。
2. 打开 `Skeleton Tree`。
3. 找到右手主骨骼，例如 `右手首`。
4. 右键骨骼，选择 `Add Socket`。
5. 命名为 `WeaponSocket`。
6. 给 socket 添加 Preview Asset，选择导入的武器 Static Mesh。
7. 调整 socket 的 Location / Rotation，让握把中心对准手掌。
8. Scale 保持 `1, 1, 1`，不要用 socket scale 修正武器资产尺寸。

保存 Skeleton / Skeletal Mesh 会修改对应 `.uasset`，这是添加角色挂点所需的资产变更。不要误保存没有实际改动的 Animation Blueprint。

### 5.7 Adding trace sockets to the weapon

1. 打开导入的武器 Static Mesh。
2. 打开 `Window -> Socket Manager`。
3. 添加 `TraceStart`，放到刀刃根部附近。
4. 添加 `TraceEnd`，放到刀尖附近。
5. 保存 Static Mesh。

如果视口中看不到武器本体，先按 `F` 聚焦。如果只能看到 socket 或 bounding shape，通常说明 OBJ 源模型尺寸过小。

### 5.8 OBJ scale issue

UE 使用厘米作为单位，OBJ 经常没有可靠单位信息。武器导入后过小是常见问题。推荐回 Blender 处理源资产：

1. 导入 OBJ。
2. 选中武器，看 `Item -> Dimensions`。
3. 单手剑长度建议约 `80 cm - 120 cm`。
4. 必要时使用 `S -> 100 -> Enter` 放大。
5. 执行 `Ctrl + A -> Scale`，让 Transform Scale 回到 `1, 1, 1`。
6. 重新导出 OBJ。
7. 在 UE 中用新目录重新导入，例如 `/Game/ThirdPerson/Weapon/Sword_Sashimi_v2`。

不要优先用角色 socket scale 或 Blueprint scale 修正源模型尺寸；那会让后续 trace socket、碰撞和预览都更难维护。

### 5.9 Texture import limitation

当前导入工具只会自动导入：

```text
第一张文件名包含 Diffuse 的 png
第一张文件名包含 Lightmap 的 png
```

如果源文件夹中有多组贴图，例如：

```text
Equip_Sword_Sashimi_01_Tex_Diffuse.png
Equip_Sword_Sashimi_01_Tex_Lightmap.png
Equip_Sword_Sashimi_02_Tex_Diffuse.png
Equip_Sword_Sashimi_02_Tex_Lightmap.png
```

工具只会取排序后的第一组。多材质槽、多贴图组自动导入需要后续升级工具。

### 5.10 Asset dirty-state caution

导入工具会生成或修改：

```text
Static Mesh
Texture
Material
DA_Weapon_*
```

添加 `WeaponSocket` 会修改角色 Skeleton / Skeletal Mesh。测试期间如果打开了 Animation Blueprint 或角色蓝图预览，UE 可能把它们标记为 dirty。确认没有实际改动时，不要保存这些无关 `.uasset`。
