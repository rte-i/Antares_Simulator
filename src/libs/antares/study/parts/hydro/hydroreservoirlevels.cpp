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

#include <algorithm>

#include <yuni/yuni.h>
#include <yuni/io/file.h>

#include <antares/exception/LoadingError.hpp>
#include <antares/inifile/inifile.h>
#include <antares/logs/logs.h>
#include <antares/study/parts/hydro/hydroreservoirlevels.h>
#include "antares/study/study.h"

namespace fs = std::filesystem;

namespace Antares::Data
{

ReservoirLevels::ReservoirLevels(TimeSeriesNumbers& timeseriesNumbers):
    max(timeseriesNumbers),
    min(timeseriesNumbers),
    avg(timeseriesNumbers),
    timeseriesNumbers(timeseriesNumbers)
{
    timeseriesNumbers.registerSeries(&max, "max-reservoir-level");
    timeseriesNumbers.registerSeries(&min, "min-reservoir-level");
    timeseriesNumbers.registerSeries(&avg, "avg-reservoir-level");

    max.reset(1L, DAYS_PER_YEAR);
    max.fill(1.0);
    avg.reset(1L, DAYS_PER_YEAR);
    avg.fill(0.5);
    min.reset(1L, DAYS_PER_YEAR);
}

bool ReservoirLevels::forceReload(bool reload) const
{
    bool ret = true;
    ret = max.forceReload(reload) && ret;
    ret = min.forceReload(reload) && ret;
    ret = avg.forceReload(reload) && ret;

    return ret;
}

void ReservoirLevels::markAsModified() const
{
    max.markAsModified();
    min.markAsModified();
    avg.markAsModified();
}

bool ReservoirLevels::loadScenarizedReservoirLevels(const std::string& areaID,
                                                    const fs::path& folder,
                                                    bool usedBySolver)
{
    if (!usedBySolver)
    {
        return true;
    }

    Matrix<>::BufferType fileContent;

    bool ret = true;

    fs::path filePath = folder / areaID / "maxDailyReservoirLevels.txt";
    ret = max.timeSeries.loadFromCSVFile(filePath.string(), 1, DAYS_PER_YEAR, &fileContent) && ret;
    filePath = folder / areaID / "minDailyReservoirLevels.txt";
    ret = min.timeSeries.loadFromCSVFile(filePath.string(), 1, DAYS_PER_YEAR, &fileContent) && ret;
    filePath = folder / areaID / "avgDailyReservoirLevels.txt";
    ret = avg.timeSeries.loadFromCSVFile(filePath.string(), 1, DAYS_PER_YEAR, &fileContent) && ret;

    return ret;
}

bool ReservoirLevels::loadReservoirLevels(const std::string& areaID,
                                          const std::filesystem::path& folder,
                                          bool usedBySolver)
{
    bool ret = true;
    Matrix<>::BufferType fileContent;

    fs::path filePath = folder / std::string("reservoir_" + areaID + ".txt");

    reservoirLevelsUI.reset(3, DAYS_PER_YEAR, true);

    if (!usedBySolver)
    {
        bool enabledModeIsChanged = false;
        if (JIT::enabled)
        {
            JIT::enabled = false; // Allowing to read the area's daily max power
            enabledModeIsChanged = true;
        }

        ret = reservoirLevelsUI.loadFromCSVFile(filePath.string(), 3, DAYS_PER_YEAR, &fileContent);

        if (enabledModeIsChanged)
        {
            JIT::enabled = true; // Back to the previous loading mode.
        }
    }
    else
    {
        ret = reservoirLevelsUI.loadFromCSVFile(filePath.string(), 3, DAYS_PER_YEAR, &fileContent);

        min.timeSeries.reset(1U, DAYS_PER_YEAR, true);
        min.timeSeries.pasteToColumn(0, reservoirLevelsUI[ReservoirLevels::minimum]);
        avg.timeSeries.reset(1U, DAYS_PER_YEAR, true);
        avg.timeSeries.pasteToColumn(0, reservoirLevelsUI[ReservoirLevels::average]);
        max.timeSeries.reset(1U, DAYS_PER_YEAR, true);
        max.timeSeries.pasteToColumn(0, reservoirLevelsUI[ReservoirLevels::maximum]);
    }

    return ret;
}

bool ReservoirLevels::saveToFolder(const std::string& areaID,
                                   const std::filesystem::path& folder) const
{
    bool ret = true;
    fs::path pathFile = folder / "common" / "capacity" / "reservoir_" / areaID / ".txt";
    ret = reservoirLevelsUI.saveToCSVFile(pathFile.string(), /*decimal*/ 3) && ret;

    return ret;
}

void ReservoirLevels::averageTimeSeries()
{
    max.averageTimeseries();
    min.averageTimeseries();
    avg.averageTimeseries();
}

} // namespace Antares::Data
