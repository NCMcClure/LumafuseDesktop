// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FMultiTask2Module : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    void StartupModule() override;
    void ShutdownModule() override;
};
