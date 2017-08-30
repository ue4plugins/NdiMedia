// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "NdiMediaSourceFactoryNew.generated.h"


/**
 * Implements a factory for UNdiMediaSource objects.
 */
UCLASS(hidecategories=Object)
class UNdiMediaSourceFactoryNew
	: public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	//~ UFactory Interface

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual bool ShouldShowInNewMenu() const override;
};
