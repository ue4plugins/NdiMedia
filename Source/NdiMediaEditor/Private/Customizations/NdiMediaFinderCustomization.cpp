// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaFinderCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "NdiMediaFinder.h"
#include "Widgets/Input/SEditableTextBox.h"


#define LOCTEXT_NAMESPACE "FNdiMediaFinderCustomization"


/* FNdiMediaFinderCustomization interface
 *****************************************************************************/

FNdiMediaFinderCustomization::~FNdiMediaFinderCustomization()
{
	if (Finder.IsValid())
	{
		Finder->Shutdown();
	}
}


/* FTickableEditorObject interface
 *****************************************************************************/

TStatId FNdiMediaFinderCustomization::GetStatId() const
{
	return TStatId();
}


bool FNdiMediaFinderCustomization::IsTickable() const
{
	return (Finder.IsValid() && PreviewTextBlock.IsValid());
}


void FNdiMediaFinderCustomization::Tick(float DeltaTime)
{
	TArray<FNdiMediaSourceId> OutSources;

	if (!Finder->GetSources(OutSources))
	{
		return;
	}

	FString PreviewString;

	for (auto Source : OutSources)
	{
		PreviewString += Source.ToString() + TEXT("\n");
	}

	FText PreviewText = PreviewString.IsEmpty()
		? LOCTEXT("Searching", "Searching...")
		: FText::FromString(PreviewString);

	if (PreviewText.CompareTo(PreviewTextBlock->GetText()) != 0)
	{
		PreviewTextBlock->SetText(PreviewText);
	}
}


/* IDetailCustomization interface
 *****************************************************************************/

void FNdiMediaFinderCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> OutObjects;
	DetailBuilder.GetObjectsBeingCustomized(OutObjects);

	if (OutObjects.Num() == 1)
	{
		Finder = Cast<UNdiMediaFinder>(OutObjects[0].Get());
		Finder->Initialize();
	}

	// customize 'Preview' category
	IDetailCategoryBuilder& PreviewCategory = DetailBuilder.EditCategory("Preview", LOCTEXT("PreviewCategoryName", "Preview"));
	{
		// add 'Preview' row
		FDetailWidgetRow& PreviewRow = PreviewCategory.AddCustomRow(LOCTEXT("PreviewRowFilterString", "Preview"));

		PreviewRow.WholeRowContent()
			[
				SAssignNew(PreviewTextBlock, SEditableTextBox)
					.IsReadOnly(true)
			];
	}
}


#undef LOCTEXT_NAMESPACE
