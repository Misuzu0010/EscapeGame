# Combat Refactor Notes

The following notes were moved out of Source/EscapeGame/Temp_Refactor_Code so UnrealBuildTool does not treat them as runtime source.

```cpp

/*
UENUM(BlueprintType)
enum class ECombatBufferedInput : uint8
{
	None,
	Light,
	Heavy
};

UENUM(BlueprintType)
enum class ECombatWindowType : uint8
{
	Combo,
	HeavyCancel
};	
UPROPERTY(Transient)
ECombatBufferedInput CombatBufferedInput = ECombatBufferedInput::None;
	
UPROPERTY(Transient)
TSet<ECombatWindowType> ActiveWindows;

void HandleAttackHoldThresholdReached();

void BufferCombatInput(ECombatBufferedInput Input);
void ClearBufferedInput();
void TryConsumeBufferedInput();
bool HasCombatWindow(ECombatWindowType WindowType) const
{
	return RuntimeState.ActiveWindows.Contains(WindowType);
};
*/
```
