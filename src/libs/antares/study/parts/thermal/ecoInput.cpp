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

#include <yuni/yuni.h>
#include <yuni/io/file.h>
#include <yuni/io/directory.h>
#include <yuni/core/math.h>
#include "../../study.h"
#include "../../memory-usage.h"
#include "ecoInput.h"
#include "../../../logs.h"
#include "../../../array/array1d.h"

using namespace Yuni;

#define SEP IO::Separator

namespace Antares::Data
{
EconomicInputData::EconomicInputData(std::weak_ptr<const ThermalCluster> cluster) :
 itsThermalCluster(cluster)
{
}

void EconomicInputData::copyFrom(const EconomicInputData& rhs)
{
    itsThermalCluster = rhs.itsThermalCluster;
    fuelcost = rhs.fuelcost;
    rhs.fuelcost.unloadFromMemory();
    co2cost = rhs.co2cost;
    rhs.co2cost.unloadFromMemory();
}

bool EconomicInputData::saveToFolder(const AnyString& folder) const
{
    bool ret = true;
    if (IO::Directory::Create(folder))
    {
        String buffer;
        buffer.clear() << folder << SEP << "fuelCost.txt";
        ret = fuelcost.saveToCSVFile(buffer) && ret;
        buffer.clear() << folder << SEP << "CO2Cost.txt";
        ret = co2cost.saveToCSVFile(buffer) && ret;
        return ret;
    }
    return false;
}

bool EconomicInputData::loadFromFolder(Study& study, const AnyString& folder)
{
    bool ret = true;
    auto& buffer = study.bufferLoadingTS;

    auto cluster = itsThermalCluster.lock();
    if (!cluster)
        return false;

    if (study.header.version >= 860)
    {
        buffer.clear() << folder << SEP << "fuelCost.txt";
        if (IO::File::Exists(buffer))
        {
            ret = fuelcost.loadFromCSVFile(
                    buffer, 1, HOURS_PER_YEAR, Matrix<>::optImmediate, &study.dataBuffer)
                  && ret;
            if (study.usedByTheSolver && study.parameters.derated)
                fuelcost.averageTimeseries();
        }

        buffer.clear() << folder << SEP << "CO2Cost.txt";
        if (IO::File::Exists(buffer))
        {
            ret = co2cost.loadFromCSVFile(
                    buffer, 1, HOURS_PER_YEAR, Matrix<>::optImmediate, &study.dataBuffer)
                  && ret;
            if (study.usedByTheSolver && study.parameters.derated)
                co2cost.averageTimeseries();
        }
    }

    return ret;
}

bool EconomicInputData::forceReload(bool reload) const
{
    bool ret = true;
    ret = fuelcost.forceReload(reload) && ret;
    ret = co2cost.forceReload(reload) && ret;
    return ret;
}

void EconomicInputData::markAsModified() const
{
    fuelcost.markAsModified();
    co2cost.markAsModified();
}

void EconomicInputData::estimateMemoryUsage(StudyMemoryUsage& u) const
{
    if (timeSeriesThermal & u.study.parameters.timeSeriesToGenerate)
    {
        fuelcost.estimateMemoryUsage(u, true, fuelcost.width, HOURS_PER_YEAR);
        u.requiredMemoryForInput += sizeof(EconomicInputData);
        co2cost.estimateMemoryUsage(u, true, co2cost.width, HOURS_PER_YEAR);
        u.requiredMemoryForInput += sizeof(EconomicInputData);
    }
}

void EconomicInputData::reset()
{
    fuelcost.reset(1, HOURS_PER_YEAR, true);
    co2cost.reset(1, HOURS_PER_YEAR, true);
}

} // namespace Antares::Data