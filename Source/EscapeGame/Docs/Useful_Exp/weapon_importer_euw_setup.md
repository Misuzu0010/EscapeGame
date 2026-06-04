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
