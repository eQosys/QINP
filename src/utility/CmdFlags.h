#pragma once

#include "libQrawlr.h"

enum class CmdFlag
{
    Help,
    Verbose,
    Keep_Intermediate_Files,
    Run_Generated_Executable,
    Export_Symbol_Info,
    Export_Comments,
    Render_Qrawlr_Tree,
    Render_Symbol_Tree
};

typedef qrawlr::Flags<CmdFlag> CmdFlags;