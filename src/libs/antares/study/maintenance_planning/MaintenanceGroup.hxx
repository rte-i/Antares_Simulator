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
#pragma once

#include <antares/utils/utils.h>
#include "MaintenanceGroup.h"

namespace Antares::Data
{
inline const MaintenanceGroup::MaintenanceGroupName& MaintenanceGroup::name() const
{
    return name_;
}

inline const MaintenanceGroup::MaintenanceGroupName& MaintenanceGroup::id() const
{
    return ID_;
}

inline uint MaintenanceGroup::areaCount() const
{
    return (uint)weights_.size();
}

inline bool MaintenanceGroup::enabled() const
{
    return enabled_;
}

inline MaintenanceGroup::ResidualLoadDefinitionType MaintenanceGroup::type() const
{
    return type_;
}

inline bool MaintenanceGroup::skipped() const
{
    return areaCount() == 0;
}

inline bool MaintenanceGroup::isActive() const
{
    return enabled() && !skipped();
}

inline MaintenanceGroup::iterator MaintenanceGroup::begin()
{
    return weights_.begin();
}

inline MaintenanceGroup::iterator MaintenanceGroup::end()
{
    return weights_.end();
}

inline MaintenanceGroup::const_iterator MaintenanceGroup::begin() const
{
    return weights_.begin();
}

inline MaintenanceGroup::const_iterator MaintenanceGroup::end() const
{
    return weights_.end();
}

} // namespace Antares::Data