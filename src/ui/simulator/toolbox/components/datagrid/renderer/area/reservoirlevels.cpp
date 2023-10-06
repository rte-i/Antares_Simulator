/*
** Copyright 2007-2023 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/

#include "reservoirlevels.h"

using namespace Yuni;

namespace Antares
{
namespace Component
{
namespace Datagrid
{
namespace Renderer
{
ATimeSeriesLevels::ATimeSeriesLevels(wxWindow* control, Toolbox::InputSelector::Area* notifier) :
 AncestorType(control), Renderer::ARendererArea(control, notifier)
{
}

ATimeSeriesLevels::~ATimeSeriesLevels()
{
    destroyBoundEvents();
}

void ATimeSeriesLevels::onStudyClosed()
{
    AncestorType::onStudyClosed();
    Renderer::ARendererArea::onStudyClosed();
}

void ATimeSeriesLevels::onStudyLoaded()
{
    AncestorType::onStudyLoaded();
    Renderer::ARendererArea::onStudyLoaded();
}

wxString ATimeSeriesLevels::cellValue(int x, int y) const
{
    if (!pArea)
        return wxString();
    auto& matrix = pArea->hydro.series->reservoirLevels;
    return ((uint)x < matrix.width && (uint)y < matrix.height)
             ? DoubleToWxString(100. * matrix[x][y])
             : wxString();
}

bool ATimeSeriesLevels::cellValue(int x, int y, const Yuni::String& value)
{
    if (!pArea)
        return false;
    auto& matrix = pArea->hydro.series->reservoirLevels;
    if ((uint)x < matrix.width && (uint)y < matrix.height)
    {
        double v;
        if (value.to(v))
        {
            v = Math::Round(v / 100., 3);
            if (v < 0.)
                v = 0.;
            if (v > 1.)
                v = 1;
            matrix[x][y] = v;
            matrix.markAsModified();
            return true;
        }
    }
    return false;
}

double ATimeSeriesLevels::cellNumericValue(int x, int y) const
{
    if (!pArea)
        return 0.;
    auto& matrix = pArea->hydro.series->reservoirLevels;
    return ((uint)x < matrix.width && (uint)y < matrix.height) ? matrix[x][y] * 100. : 0.;
}

wxString ATimeSeriesLevels::columnCaption(int colIndx) const
{
    switch ((1 + colIndx) % 3)
    {
    case 0:
    {
        return wxString() << wxT("     TS-") << ((colIndx / 3) + 1)
                          << wxT("     \n High Level (%)");
        break;
    }
    case 1:
    {
        return wxString() << wxT("     TS-") << ((colIndx / 3) + 1) << wxT("    \n Low Level (%)");
        break;
    }
    case 2:
    {
        return wxString() << wxT("     TS-") << ((colIndx / 3) + 1) << wxT("    \n Avg Level (%)");
        break;
    }
    default:
    {
        return wxT("0");
    }
    }
}

wxColour ATimeSeriesLevels::verticalBorderColor(int x, int y) const
{
    return (x == AncestorType::width() - 1) ? Default::BorderHighlightColor()
                                            : IRenderer::verticalBorderColor(x, y);
}

wxColour ATimeSeriesLevels::horizontalBorderColor(int x, int y) const
{
    // Getting informations about the next hour
    // (because the returned color is about the bottom border of the cell,
    // so the next hour for the user)
    if (!(!study) && y + 1 < Date::Calendar::maxHoursInYear)
    {
        auto& hourinfo = study->calendar.hours[y + 1];

        if (hourinfo.firstHourInMonth)
            return Default::BorderMonthSeparator();
        if (hourinfo.firstHourInDay)
            return Default::BorderDaySeparator();
    }
    return IRenderer::verticalBorderColor(x, y);
}

IRenderer::CellStyle ATimeSeriesLevels::cellStyle(int col, int row) const
{
    if (pArea)
    {
        auto& matrix = pArea->hydro.series->reservoirLevels;
        if ((uint)row < matrix.height)
        {
            int minIndex = 0;
            if (((col + 1) % 3) == 0)
            {
                minIndex = col - 2;
            }
            else if (((col + 1) % 3) == 2)
            {
                minIndex = col - 1;
            }
            else if (((col + 1) % 3) == 1)
            {
                minIndex = col;
            }

            double d = matrix[col][row];
            double min = matrix[minIndex][row];
            if (d < 0 || d > 1.)
                return IRenderer::cellStyleError;
            if (d < min)
                return IRenderer::cellStyleError;
            // We can use IRenderer::cellStyleWithNu... since this method
            // as no mean to know data from hydro.reservoirLevel
            if (Math::Zero(d))
            {
                return (row % 2) ? cellStyleDefaultAlternateDisabled : cellStyleDefaultDisabled;
            }
            else
            {
                return (row % 2) ? cellStyleDefaultAlternate : cellStyleDefault;
            }
        }
    }
    return IRenderer::cellStyleWithNumericCheck(col, row);
}

} // namespace Renderer
} // namespace Datagrid
} // namespace Component
} // namespace Antares
