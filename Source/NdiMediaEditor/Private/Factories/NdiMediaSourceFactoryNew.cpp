// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaSourceFactoryNew.h"

#include "AssetTypeCategories.h"
#include "NdiMediaSource.h"


/* UNdiMediaSourceFactoryNew structors
 *****************************************************************************/

UNdiMediaSourceFactoryNew::UNdiMediaSourceFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UNdiMediaSource::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UNdiMediaSourceFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UNdiMediaSource>(InParent, InClass, InName, Flags);
}


uint32 UNdiMediaSourceFactoryNew::GetMenuCategories() const
{
	return EAssetTypeCategories::Media;
}


bool UNdiMediaSourceFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
