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
#ifndef __ANTARES_TOOLBOX_COMPONENT_DATAGRID_RENDERER_AREA_RESERVOIRLEVELS_H__
#define __ANTARES_TOOLBOX_COMPONENT_DATAGRID_RENDERER_AREA_RESERVOIRLEVELS_H__

#include "../area.h"
#include "../matrix.h"
#include <antares/date/date.h>

namespace Antares
{
namespace Component
{
namespace Datagrid
{
namespace Renderer
{

class ATimeSeriesLevels : public Renderer::Matrix<double>, public Renderer::ARendererArea
{
public:
    using AncestorType = Renderer::Matrix<double>;

public:
    ATimeSeriesLevels(wxWindow* control, Toolbox::InputSelector::Area* notifier);

    virtual ~ATimeSeriesLevels();

    virtual int width() const
    {
        return AncestorType::width();
    }

    virtual int height() const
    {
        return AncestorType::height();
    }

    virtual int internalWidth() const
    {
        return AncestorType::width();
    }
    virtual int internalHeight() const
    {
        return AncestorType::height();
    }

    virtual wxString columnCaption(int colIndx) const;

    virtual wxString rowCaption(int rowIndx) const
    {
        return AncestorType::rowCaption(rowIndx);
    }

    virtual wxString cellValue(int x, int y) const;

    virtual double cellNumericValue(int x, int y) const;

    virtual bool cellValue(int x, int y, const Yuni::String& value);

    virtual void resetColors(int, int, wxColour&, wxColour&) const
    { /*Do nothing*/
    }

    virtual bool isTriplet() const override
    {
        return true;
    }

    virtual wxColour verticalBorderColor(int x, int y) const;
    virtual wxColour horizontalBorderColor(int x, int y) const;

    virtual IRenderer::CellStyle cellStyle(int col, int row) const;

    virtual bool valid() const
    {
        return AncestorType::valid();
    }

    virtual Date::Precision precision()
    {
        return Date::hourly;
    }

protected:
    //! Event: the study has been closed
    virtual void onStudyClosed() override;
    //! Event: the study has been loaded
    virtual void onStudyLoaded() override;

}; // class ATimeSeriesLevels

class TimeSeriesReservoirLevels final : public ATimeSeriesLevels
{
public:
    using AncestorType = Renderer::Matrix<double>;

public:
    TimeSeriesReservoirLevels(wxWindow* control, Toolbox::InputSelector::Area* notifier) :
     ATimeSeriesLevels(control, notifier)
    {
    }
    virtual ~TimeSeriesReservoirLevels()
    {
        destroyBoundEvents();
    }

    virtual Date::Precision precision()
    {
        return Date::daily;
    }

    virtual wxString rowCaption(int rowIndx) const
    {
        return wxStringFromUTF8(study->calendar.text.daysYear[rowIndx]);
    }

    virtual uint maxHeightResize() const
    {
        return DAYS_PER_YEAR;
    }

protected:
    virtual void internalAreaChanged(Antares::Data::Area* area)
    {
        AncestorType::matrix((area && CurrentStudyIsValid()) ? &(area->hydro.series->reservoirLevels) : NULL);
        Renderer::ARendererArea::internalAreaChanged(area);
    }
};  // class TimeSeriesReservoirLevels

} // namespace Renderer
} // namespace Datagrid
} // namespace Component
} // namespace Antares

#endif // __ANTARES_TOOLBOX_COMPONENT_DATAGRID_RENDERER_AREA_RESERVOIRLEVELS_H__
