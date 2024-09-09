// Fill out your copyright notice in the Description page of Project Settings.


#include "AssestAction/QuickMaterialWidget.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"


#pragma region QuickMaterialCreationCore

void UQuickMaterialWidget::CreateMaterialFromSelectedTextures()
{
	if (bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid name"));
			return;
		}
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;


	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath))return;

	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName))return;

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath);

	if (!CreatedMaterial)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material"));
		return;
	}

	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if (!SelectedTexture) continue;

		Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
	}

}//CreateMaterialFromSelectedTextures.


#pragma endregion


#pragma region QuickMaterialCreation

//Process the selected data, will filter out textures,and return false if non-texture selected
bool UQuickMaterialWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess, 
	TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProcess.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No texture selected"));
		return false;
	}

	bool bMaterialNameSet = false;

	for (const FAssetData& SelectedData : SelectedDataToProcess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset();

		if (!SelectedAsset)continue;

		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);

		if (!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select only textures\n") +
			SelectedAsset->GetName()+ TEXT("is not a texture"));
			return false;
		}

		OutSelectedTexturesArray.Add(SelectedTexture);

		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();

		}
		if (!bCustomMaterialName && !bMaterialNameSet)
		{

			MaterialName = SelectedAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0, TEXT("M_"));

			bMaterialNameSet = true;

		}

	}
	return true;
}//ProcessSelectedData.


//Will return true if the material name is used by asset under the specified folder
bool UQuickMaterialWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{

	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);

	for (const FString& ExistingAssetPath : ExistingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath);

		if (ExistingAssetName.Equals(MaterialNameToCheck))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, MaterialNameToCheck +
				TEXT(" is already used by asset"));

			return true;
		}
	}

	return false;

	
}

UMaterial* UQuickMaterialWidget::CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterial, PathToPutMaterial,
		UMaterial::StaticClass(), MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}//CreateMaterialAsset.

void UQuickMaterialWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial,
	UTexture2D* SelectedTexture, uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode =
		NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);

	if (!TextureSampleNode) return;

	if (!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
}

#pragma endregion

#pragma region CreateMaterialNodes

bool UQuickMaterialWidget::TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			//Connect pins to base color socket here
			TextureSampleNode->Texture = SelectedTexture;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_BaseColor)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;

			return true;
		}
	}

	return false;
}

#pragma endregion