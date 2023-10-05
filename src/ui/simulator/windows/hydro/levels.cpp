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

#include "levels.h"
#include <wx/stattext.h>
#include "../../toolbox/components/datagrid/renderer/area/reservoirlevels.h"
#include "../../toolbox/components/datagrid/renderer/area/watervalues.h"
#include "../../toolbox/components/button.h"
#include "../../toolbox/validator.h"
#include "../../toolbox/create.h"
#include "../../application/menus.h"
#include <wx/statline.h>

using namespace Yuni;

namespace Antares
{
namespace Window
{
namespace Hydro
{
Levels::Levels(wxWindow* parent, Toolbox::InputSelector::Area* notifier) :
 Component::Panel(parent),
 pInputAreaSelector(notifier),
 pArea(nullptr),
 pComponentsAreReady(false),
 pSupport(nullptr)
{
    OnStudyClosed.connect(this, &Levels::onStudyClosed);
    if (notifier)
        notifier->onAreaChanged.connect(this, &Levels::onAreaChanged);
}

void Levels::createComponents()
{
    if (pComponentsAreReady)
        return;
    pComponentsAreReady = true;

    {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        SetSizer(sizer);
        pSupport = new Component::Panel(this);
        sizer->Add(pSupport, 1, wxALL | wxEXPAND);
    }

    wxBoxSizer* ssGrids = new wxBoxSizer(wxHORIZONTAL);
    pSupport->SetSizer(ssGrids);

    ssGrids->Add(new Component::Datagrid::Component(
                   pSupport,
                   new Component::Datagrid::Renderer::ReservoirLevels(this, pInputAreaSelector),
                   wxT("Reservoir levels")),
                 2,
                 wxALL | wxEXPAND | wxFIXED_MINSIZE,
                 5);
    ssGrids->Add(
      new wxStaticLine(pSupport, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL),
      0,
      wxALL | wxEXPAND);
    ssGrids->Layout();
}

Levels::~Levels()
{
    destroyBoundEvents();
    // destroy all children as soon as possible to prevent against corrupt vtable
    DestroyChildren();
}

void Levels::onAreaChanged(Data::Area* area)
{
    pArea = area;
    if (area and area->hydro.prepro)
    {
        // create components on-demand
        if (!pComponentsAreReady)
            createComponents();
        else
            GetSizer()->Show(pSupport, true);
    }
}

void Levels::onStudyClosed()
{
    pArea = nullptr;

    if (GetSizer())
        GetSizer()->Show(pSupport, false);
}

} // namespace Hydro
} // namespace Window
} // namespace Antares
