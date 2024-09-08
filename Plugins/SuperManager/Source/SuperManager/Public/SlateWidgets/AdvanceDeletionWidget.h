// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}

	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetsDataToStore)

	SLATE_ARGUMENT(FString, CurrentSelectedFolder)

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& Ina);

private:

	TArray<TSharedPtr<FAssetData>> StoredAssetsData;

	TSharedPtr< SListView< TSharedPtr <FAssetData> > > ConstructedAssetListView;

	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;

	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;

	TArray<TSharedPtr<FAssetData>> DisplayedAssetData;

	FSlateFontInfo GetEmboseedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

	TSharedRef< SListView< TSharedPtr <FAssetData> > > ConstructAssetListView();
	
	void RefreshAssetListView();

#pragma region ComboBoxForListingCondition

	TArray<TSharedPtr <FString>> ComboSourceItems;

	TSharedPtr<STextBlock> ComboDisplayTextBlock;


	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();

	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption,ESelectInfo::Type InSelectInfo);

	TSharedRef<STextBlock> ConstructComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify);

#pragma endregion


#pragma region RowWidgetForAssetListView

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData>AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable);

	void OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData);

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>&AssetDataToDisplay);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

#pragma endregion

#pragma region TabButtons




	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();

	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();


	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

#pragma endregion
};