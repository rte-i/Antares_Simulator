/*
** Copyright 2007-2024, RTE (https://www.rte-france.com)
** See AUTHORS.txt
** SPDX-License-Identifier: MPL-2.0
** This file is part of Antares-Simulator,
** Adequacy and Performance assessment for interconnected energy networks.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the Mozilla Public Licence 2.0 as published by
** the Mozilla Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** Mozilla Public Licence 2.0 for more details.
**
** You should have received a copy of the Mozilla Public Licence 2.0
** along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
*/

#include <action/settings/suffix-for-area-names.h>

namespace Antares
{
namespace Action
{
namespace Settings
{
void SuffixAreaName::prepareSkipWL(Context& ctx)
{
    // reset the property
    ctx.property["area.name.suffix"].clear();
}

bool SuffixAreaName::prepareWL(Context& ctx)
{
    // reset the property
    ctx.property["area.name.suffix"] = pValue;

    if (pValue.empty())
    {
        pInfos.caption = "Suffix for area names";
        pInfos.state = stDisabled;
    }
    else
    {
        pInfos.message.clear();
        pInfos.state = stReady;
        if (pValue == "<auto>")
        {
            pInfos.caption = "Suffix for area names (auto)";
            pInfos.message << "Make sure there is no collision between area names";
        }
        else
        {
            pInfos.caption = "Suffix for area names (custom)";
            pInfos.message << "All area names will be suffixed with \"" << pValue << '"';
        }
    }
    return true;
}

void SuffixAreaName::behaviorToText(Behavior behavior, Yuni::String& out)
{
    switch (behavior)
    {
    case bhOverwrite:
        out = "enabled";
        break;
    case bhMerge:
        out = "enabled";
        break;
    case bhSkip:
        out = "skip";
        break;
    case bhMax:
        out.clear();
        break;
    }
}

} // namespace Settings
} // namespace Action
} // namespace Antares
