// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaEditorPCH.h"
#include "NdiMediaSourceFactoryNew.h"


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
