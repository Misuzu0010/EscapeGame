#include "WeaponImportToolLibrary.h"

#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

namespace WeaponImportTool
{
	bool IsValidGameDestinationPath(const FString& DestinationPath)
	{
		return DestinationPath.StartsWith(TEXT("/Game/")) &&
			FPackageName::IsValidLongPackageName(DestinationPath);
	}
}

FWeaponImportResult UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
	const FString& SourceFolder,
	const FString& DestinationPath,
	const FString& AssetBaseName,
	float BaseDamage,
	float TraceRadius,
	FName AttachSocketName,
	FName TraceStartSocketName,
	FName TraceEndSocketName
)
{
	FWeaponImportResult Result;

	(void)AttachSocketName;
	(void)TraceStartSocketName;
	(void)TraceEndSocketName;

	if (SourceFolder.IsEmpty() || !IFileManager::Get().DirectoryExists(*SourceFolder))
	{
		Result.AddMessage(FString::Printf(TEXT("\u6E90\u6587\u4EF6\u5939\u4E0D\u5B58\u5728\uFF1A%s"), *SourceFolder));
		return Result;
	}

	if (!WeaponImportTool::IsValidGameDestinationPath(DestinationPath))
	{
		Result.AddMessage(FString::Printf(TEXT("\u76EE\u6807\u8DEF\u5F84\u5FC5\u987B\u662F\u6709\u6548\u7684 /Game/... \u8DEF\u5F84\uFF1A%s"), *DestinationPath));
		return Result;
	}

	if (AssetBaseName.TrimStartAndEnd().IsEmpty())
	{
		Result.AddMessage(TEXT("\u8D44\u4EA7\u57FA\u7840\u540D\u4E0D\u80FD\u4E3A\u7A7A\u3002"));
		return Result;
	}

	if (BaseDamage < 0.0f)
	{
		Result.AddMessage(TEXT("BaseDamage \u4E0D\u80FD\u5C0F\u4E8E 0\u3002"));
		return Result;
	}

	if (TraceRadius <= 0.0f)
	{
		Result.AddMessage(TEXT("TraceRadius \u5FC5\u987B\u5927\u4E8E 0\u3002"));
		return Result;
	}

#if WITH_EDITOR
	Result.bSucceeded = true;
	Result.AddMessage(TEXT("\u53C2\u6570\u6821\u9A8C\u901A\u8FC7\uFF0C\u5BFC\u5165\u6D41\u7A0B\u5C06\u5728\u540E\u7EED\u4EFB\u52A1\u4E2D\u63A5\u5165\u3002"));
#else
	Result.AddMessage(TEXT("\u6B66\u5668\u5BFC\u5165\u5DE5\u5177\u53EA\u80FD\u5728 UE Editor \u4E2D\u6267\u884C\u3002"));
#endif

	return Result;
}
