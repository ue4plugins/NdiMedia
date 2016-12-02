// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaSourceCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
#include "IDetailPropertyRow.h"
#include "NdiMediaFinder.h"
#include "NdiMediaSource.h"
#include "Widgets/Input/SComboButton.h"


#define LOCTEXT_NAMESPACE "FNdiMediaSourceCustomization"


/* IDetailCustomization interface
 *****************************************************************************/

void FNdiMediaSourceCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// customize 'NDI' category
	IDetailCategoryBuilder& NdiCategory = DetailBuilder.EditCategory("NDI");
	{
		// SourceName
		SourceNameProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UNdiMediaSource, SourceName));
		{
			IDetailPropertyRow& SourceNameRow = NdiCategory.AddProperty(SourceNameProperty);

			SourceNameRow.CustomWidget()
				.NameContent()
				[
					SourceNameProperty->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MaxDesiredWidth(0.0f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SourceNameProperty->CreatePropertyValueWidget(false)
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SComboButton)
								.OnGetMenuContent(this, &FNdiMediaSourceCustomization::HandleSourceComboButtonMenuContent, EProperty::SourceName)
								.ContentPadding(FMargin(4.0, 2.0))
						]
				];
		}

		// SourceEndpoint
		SourceEndpointProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UNdiMediaSource, SourceEndpoint));
		{
			IDetailPropertyRow& SourceEndpointRow = NdiCategory.AddProperty(SourceEndpointProperty);

			SourceEndpointRow.CustomWidget()
				.NameContent()
				[
					SourceEndpointProperty->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MaxDesiredWidth(0.0f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SourceEndpointProperty->CreatePropertyValueWidget(false)
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SComboButton)
								.OnGetMenuContent(this, &FNdiMediaSourceCustomization::HandleSourceComboButtonMenuContent, EProperty::SourceEndpoint)
								.ContentPadding(FMargin(4.0, 2.0))
						]
				];
		}
	}
}


/* FNdiMediaSourceCustomization callbacks
 *****************************************************************************/

TSharedRef<SWidget> FNdiMediaSourceCustomization::HandleSourceComboButtonMenuContent(EProperty Property) const
{
	// get default NDI source finder object
	auto DefaultFinder = GetDefault<UNdiMediaFinder>();

	if (DefaultFinder == nullptr)
	{
		return SNullWidget::NullWidget;
	}

	// fetch found NDI sources
	TArray<FNdiMediaSourceId> OutSources;

	if (!DefaultFinder->GetSources(OutSources))
	{
		return SNullWidget::NullWidget;
	}

	// generate menu
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("AllSources", LOCTEXT("AllSourcesSection", "All Sources"));
	{
		bool SourceAdded = false;

		for (auto Source : OutSources)
		{
			const TSharedPtr<IPropertyHandle> ResetProperty = (Property == EProperty::SourceName) ? SourceEndpointProperty : SourceNameProperty;
			const TSharedPtr<IPropertyHandle> ValueProperty = (Property == EProperty::SourceName) ? SourceNameProperty : SourceEndpointProperty;
			const FString ValueStr = (Property == EProperty::SourceName) ? Source.Name : Source.Endpoint;
			const FString UrlStr = FString(TEXT("ndi://")) + ValueStr;

			MenuBuilder.AddMenuEntry(
				FText::FromString(Source.ToString()),
				FText::FromString(UrlStr),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([=] {
						ValueProperty->SetValue(ValueStr);
						ResetProperty->SetValue(FString());
					}),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([=]{
						FString CurrentValue;
						return ((ValueProperty->GetValue(CurrentValue) == FPropertyAccess::Success) && CurrentValue == ValueStr);
					})
				),
				NAME_None,
				EUserInterfaceActionType::RadioButton
			);

			SourceAdded = true;
		}

		if (!SourceAdded)
		{
			MenuBuilder.AddWidget(SNullWidget::NullWidget, LOCTEXT("NoSourcesFound", "No sources found"), false, false);
		}
	}
	MenuBuilder.EndSection();

	const FString LocalhostPrefix = FString(FPlatformProcess::ComputerName()) + TEXT(" ");

	MenuBuilder.BeginSection("LocalSources", LOCTEXT("LocalSourcesSection", "Local Sources"));
	{
		bool SourceAdded = false;

		for (auto Source : OutSources)
		{
			FString Name = Source.Name;

			if (!Name.StartsWith(LocalhostPrefix))
			{
				continue;
			}

			const int32 ColonIdx = Source.Endpoint.Find(TEXT(":"));

			if (ColonIdx == INDEX_NONE)
			{
				continue;
			}

			const TSharedPtr<IPropertyHandle> ResetProperty = (Property == EProperty::SourceName) ? SourceEndpointProperty : SourceNameProperty;
			const TSharedPtr<IPropertyHandle> ValueProperty = (Property == EProperty::SourceName) ? SourceNameProperty : SourceEndpointProperty;

			const FString EndpointStr = FString(TEXT("127.0.0.1")) + Source.Endpoint.RightChop(ColonIdx);
			const FString NameStr = Name.Replace(*LocalhostPrefix, TEXT("localhost "));
			const FString SourceStr = NameStr + TEXT(" [") + EndpointStr + TEXT("]");
			const FString ValueStr = (Property == EProperty::SourceName) ? NameStr : EndpointStr;
			const FString UrlStr = FString(TEXT("ndi://")) + ValueStr;

			MenuBuilder.AddMenuEntry(
				FText::FromString(SourceStr),
				FText::FromString(UrlStr),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([=] {
						ValueProperty->SetValue(ValueStr);
						ResetProperty->SetValue(FString());
					}),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([=]{
						FString CurrentValue;
						return ((ValueProperty->GetValue(CurrentValue) == FPropertyAccess::Success) && CurrentValue == ValueStr);
					})
				),
				NAME_None,
				EUserInterfaceActionType::RadioButton
			);

			SourceAdded = true;
		}

		if (!SourceAdded)
		{
			MenuBuilder.AddWidget(SNullWidget::NullWidget, LOCTEXT("NoSourcesFound", "No sources found"), false, false);
		}
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}


#undef LOCTEXT_NAMESPACE
