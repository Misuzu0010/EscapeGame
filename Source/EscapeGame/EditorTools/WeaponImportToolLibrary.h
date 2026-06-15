#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WeaponImportToolLibrary.generated.h"

USTRUCT(BlueprintType)
struct ESCAPEGAME_API FWeaponImportResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	bool bSucceeded = false;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	FString StaticMeshObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	FString MaterialObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	FString WeaponDefinitionObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	TArray<FString> Messages;

	void AddMessage(const FString& Message)
	{
		Messages.Add(Message);
	}
};

UCLASS()
class ESCAPEGAME_API UWeaponImportToolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "TATools|Weapon Import", meta = (DevelopmentOnly))
	static FWeaponImportResult ImportWeaponFromObjFolder(
		const FString& SourceFolder,
		const FString& DestinationPath,
		const FString& AssetBaseName,
		float BaseDamage = 20.0f,
		float TraceRadius = 12.0f,
		FName AttachSocketName = TEXT("WeaponSocket"),
		FName TraceStartSocketName = TEXT("TraceStart"),
		FName TraceEndSocketName = TEXT("TraceEnd")
	);
};
