#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "EditorTools/WeaponImportToolLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponImportToolInvalidSourceTest,
	"EscapeGame.Editor.WeaponImport.InvalidSourceFolder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FWeaponImportToolInvalidSourceTest::RunTest(const FString& Parameters)
{
	const FWeaponImportResult Result = UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
		TEXT("Z:/Path/That/Does/Not/Exist"),
		TEXT("/Game/Weapons/TestSword"),
		TEXT("TestSword"),
		20.0f,
		12.0f,
		TEXT("WeaponSocket"),
		TEXT("TraceStart"),
		TEXT("TraceEnd")
	);

	TestFalse(TEXT("Invalid source folder must fail"), Result.bSucceeded);
	TestTrue(TEXT("Invalid source folder returns a message"), Result.Messages.Num() > 0);
	if (Result.Messages.Num() > 0)
	{
		TestTrue(
			TEXT("Message mentions source folder"),
			Result.Messages[0].Contains(TEXT("\u6E90\u6587\u4EF6\u5939\u4E0D\u5B58\u5728"))
		);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponImportToolInvalidDestinationTest,
	"EscapeGame.Editor.WeaponImport.InvalidDestinationPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FWeaponImportToolInvalidDestinationTest::RunTest(const FString& Parameters)
{
	const FString ExistingFolder = FPaths::ProjectDir();
	const FWeaponImportResult Result = UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
		ExistingFolder,
		TEXT("Weapons/TestSword"),
		TEXT("TestSword"),
		20.0f,
		12.0f,
		TEXT("WeaponSocket"),
		TEXT("TraceStart"),
		TEXT("TraceEnd")
	);

	TestFalse(TEXT("Destination path outside /Game must fail"), Result.bSucceeded);
	TestTrue(TEXT("Invalid destination returns a message"), Result.Messages.Num() > 0);
	if (Result.Messages.Num() > 0)
	{
		TestTrue(
			TEXT("Message mentions invalid destination path"),
			Result.Messages[0].Contains(TEXT("\u76EE\u6807\u8DEF\u5F84\u5FC5\u987B\u662F\u6709\u6548\u7684 /Game"))
		);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponImportToolInvalidAssetNameTest,
	"EscapeGame.Editor.WeaponImport.InvalidAssetName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FWeaponImportToolInvalidAssetNameTest::RunTest(const FString& Parameters)
{
	const FString ExistingFolder = FPaths::ProjectDir();
	const FWeaponImportResult Result = UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
		ExistingFolder,
		TEXT("/Game/Weapons/TestSword"),
		TEXT(""),
		20.0f,
		12.0f,
		TEXT("WeaponSocket"),
		TEXT("TraceStart"),
		TEXT("TraceEnd")
	);

	TestFalse(TEXT("Empty asset name must fail"), Result.bSucceeded);
	TestTrue(TEXT("Invalid asset name returns a message"), Result.Messages.Num() > 0);
	if (Result.Messages.Num() > 0)
	{
		TestTrue(
			TEXT("Message mentions asset name"),
			Result.Messages[0].Contains(TEXT("\u8D44\u4EA7\u57FA\u7840\u540D\u4E0D\u80FD\u4E3A\u7A7A"))
		);
	}
	return true;
}

#endif
