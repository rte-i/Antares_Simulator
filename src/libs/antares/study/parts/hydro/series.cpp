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
#include <antares/study/parts/hydro/series.h>
#include "antares/study/study.h"

using namespace Yuni;

namespace fs = std::filesystem;

#define SEP IO::Separator

namespace Antares::Data
{

static bool loadTSfromFile(Matrix<double>& ts,
                           const std::string& areaID,
                           const fs::path& folder,
                           const std::string& filename,
                           unsigned int height)
{
    fs::path filePath = folder / areaID / filename;
    Matrix<>::BufferType fileContent;
    return ts.loadFromCSVFile(filePath.string(), 1, height, &fileContent);
}

static void ConvertDailyTSintoHourlyTS(const Matrix<double>::ColumnType& dailyColumn,
                                       Matrix<double>::ColumnType& hourlyColumn)
{
    uint hour = 0;
    uint day = 0;

    while (hour < HOURS_PER_YEAR && day < DAYS_PER_YEAR)
    {
        for (uint i = 0; i < HOURS_PER_DAY; ++i)
        {
            hourlyColumn[hour] = dailyColumn[day];
            ++hour;
        }
        ++day;
    }
}

DataSeriesHydro::DataSeriesHydro():
    ror(timeseriesNumbers),
    storage(timeseriesNumbers),
    mingen(timeseriesNumbers),
    maxHourlyGenPower(timeseriesNumbers),
    maxHourlyPumpPower(timeseriesNumbers),
    maxDailyReservoirLevels(timeseriesNumbers),
    minDailyReservoirLevels(timeseriesNumbers),
    avgDailyReservoirLevels(timeseriesNumbers),
    reservoirLevels(timeseriesNumbers)
{
    timeseriesNumbers.registerSeries(&ror, "ror");
    timeseriesNumbers.registerSeries(&storage, "storage");
    timeseriesNumbers.registerSeries(&mingen, "mingen");
    timeseriesNumbers.registerSeries(&maxHourlyGenPower, "max-geneneration-power");
    timeseriesNumbers.registerSeries(&maxHourlyPumpPower, "max-pumping-power");
    timeseriesNumbers.registerSeries(&maxDailyReservoirLevels, "max-reservoir-level");
    timeseriesNumbers.registerSeries(&minDailyReservoirLevels, "min-reservoir-level");
    timeseriesNumbers.registerSeries(&avgDailyReservoirLevels, "avg-reservoir-level");

    // Pmin was introduced in v8.6
    // The previous behavior was Pmin=0
    // For compatibility reasons with existing studies, mingen, maxHourlyGenPower and
    // maxHourlyPumpPower are set to one column of zeros by default
    mingen.reset();
    maxHourlyGenPower.reset();
    maxHourlyPumpPower.reset();
    maxDailyReservoirLevels.reset(1L, DAYS_PER_YEAR);
    maxDailyReservoirLevels.fill(1.0);
    avgDailyReservoirLevels.reset(1L, DAYS_PER_YEAR);
    avgDailyReservoirLevels.fill(0.5);
    minDailyReservoirLevels.reset(1L, DAYS_PER_YEAR);
}

void DataSeriesHydro::copyGenerationTS(const DataSeriesHydro& source)
{
    ror.timeSeries = source.ror.timeSeries;
    storage.timeSeries = source.storage.timeSeries;
    mingen.timeSeries = source.mingen.timeSeries;
    reservoirLevels.reservoirLevelsUI = source.reservoirLevels.reservoirLevelsUI;

    source.ror.unloadFromMemory();
    source.storage.unloadFromMemory();
    source.mingen.unloadFromMemory();
    source.reservoirLevels.reservoirLevelsUI.unloadFromMemory();
}

void DataSeriesHydro::copyMaxPowerTS(const DataSeriesHydro& source)
{
    maxHourlyGenPower.timeSeries = source.maxHourlyGenPower.timeSeries;
    maxHourlyPumpPower.timeSeries = source.maxHourlyPumpPower.timeSeries;

    source.maxHourlyGenPower.unloadFromMemory();
    source.maxHourlyPumpPower.unloadFromMemory();
}

void DataSeriesHydro::reset()
{
    resizeTS(1);
}

void DataSeriesHydro::resizeTS(uint nbSeries)
{
    storage.reset(nbSeries, DAYS_PER_YEAR);
    ror.reset(nbSeries, HOURS_PER_YEAR);
}

bool DataSeriesHydro::forceReload(bool reload) const
{
    bool ret = true;
    ret = ror.forceReload(reload) && ret;
    ret = storage.forceReload(reload) && ret;
    ret = mingen.forceReload(reload) && ret;
    ret = maxHourlyGenPower.forceReload(reload) && ret;
    ret = maxHourlyPumpPower.forceReload(reload) && ret;
    ret = maxDailyReservoirLevels.forceReload(reload) && ret;
    ret = avgDailyReservoirLevels.forceReload(reload) && ret;
    ret = minDailyReservoirLevels.forceReload(reload) && ret;
    ret = reservoirLevels.forceReload(reload) && ret;
    return ret;
}

void DataSeriesHydro::markAsModified() const
{
    ror.markAsModified();
    storage.markAsModified();
    mingen.markAsModified();
    maxHourlyGenPower.markAsModified();
    maxHourlyPumpPower.markAsModified();
    maxDailyReservoirLevels.markAsModified();
    avgDailyReservoirLevels.markAsModified();
    minDailyReservoirLevels.markAsModified();
    reservoirLevels.markAsModified();
}

bool DataSeriesHydro::loadGenerationTS(const AreaName& areaID,
                                       const fs::path& folder,
                                       StudyVersion studyVersion)
{
    timeseriesNumbers.clear();

    bool ret = loadTSfromFile(ror.timeSeries, areaID, folder, "ror.txt", HOURS_PER_YEAR);
    ret = loadTSfromFile(storage.timeSeries, areaID, folder, "mod.txt", DAYS_PER_YEAR) && ret;
    if (studyVersion >= StudyVersion(8, 6))
    {
        ret = loadTSfromFile(mingen.timeSeries, areaID, folder, "mingen.txt", HOURS_PER_YEAR)
              && ret;
    }
    return ret;
}

bool DataSeriesHydro::LoadMaxPower(const std::string& areaID, const fs::path& folder)
{
    bool ret = true;
    Matrix<>::BufferType fileContent;

    fs::path filePath = folder / areaID / "maxHourlyGenPower.txt";
    ret = maxHourlyGenPower.timeSeries.loadFromCSVFile(filePath.string(),
                                                       1,
                                                       HOURS_PER_YEAR,
                                                       &fileContent)
          && ret;

    filePath = folder / areaID / "maxHourlyPumpPower.txt";
    ret = maxHourlyPumpPower.timeSeries.loadFromCSVFile(filePath.string(),
                                                        1,
                                                        HOURS_PER_YEAR,
                                                        &fileContent)
          && ret;

    return ret;
}

bool DataSeriesHydro::loadScenarizedReservoirLevels(const std::string& areaID,
                                                    const fs::path& folder,
                                                    bool usedBySolver)
{
    if (!usedBySolver)
    {
        return true;
    }

    bool ret = true;

    ret = loadTSfromFile(maxDailyReservoirLevels.timeSeries,
                         areaID,
                         folder,
                         "maxDailyReservoirLevels.txt",
                         DAYS_PER_YEAR)
          && ret;
    ret = loadTSfromFile(minDailyReservoirLevels.timeSeries,
                         areaID,
                         folder,
                         "minDailyReservoirLevels.txt",
                         DAYS_PER_YEAR)
          && ret;
    ret = loadTSfromFile(avgDailyReservoirLevels.timeSeries,
                         areaID,
                         folder,
                         "avgDailyReservoirLevels.txt",
                         DAYS_PER_YEAR)
          && ret;

    return ret;
}

bool DataSeriesHydro::loadReservoirLevels(const std::string& areaID,
                                          const std::filesystem::path& folder,
                                          bool usedBySolver)
{
    if (!usedBySolver)
    {
        return true;
    }

    bool ret = true;
    Matrix<>::BufferType fileContent;
    Matrix<double> reservoirLevelDataBuffer;
    fs::path filePath = folder / std::string("reservoir_" + areaID + ".txt");
    reservoirLevelDataBuffer.reset(3, DAYS_PER_YEAR, true);
    ret = reservoirLevelDataBuffer.loadFromCSVFile(filePath.string(),
                                                   3,
                                                   DAYS_PER_YEAR,
                                                   &fileContent);

    minDailyReservoirLevels.timeSeries.reset(1U, DAYS_PER_YEAR, true);
    minDailyReservoirLevels.timeSeries.pasteToColumn(0,
                                                     reservoirLevelDataBuffer[PartHydro::minimum]);
    avgDailyReservoirLevels.timeSeries.reset(1U, DAYS_PER_YEAR, true);
    avgDailyReservoirLevels.timeSeries.pasteToColumn(0,
                                                     reservoirLevelDataBuffer[PartHydro::average]);
    maxDailyReservoirLevels.timeSeries.reset(1U, DAYS_PER_YEAR, true);
    maxDailyReservoirLevels.timeSeries.pasteToColumn(0,
                                                     reservoirLevelDataBuffer[PartHydro::maximum]);

    return ret;
}

void DataSeriesHydro::buildHourlyMaxPowerFromDailyTS(
  const Matrix<double>::ColumnType& DailyMaxGenPower,
  const Matrix<double>::ColumnType& DailyMaxPumpPower)
{
    const uint count = 1;

    maxHourlyGenPower.reset(count, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(count, HOURS_PER_YEAR);

    ConvertDailyTSintoHourlyTS(DailyMaxGenPower, maxHourlyGenPower.timeSeries[0]);
    ConvertDailyTSintoHourlyTS(DailyMaxPumpPower, maxHourlyPumpPower.timeSeries[0]);
}

bool DataSeriesHydro::saveToFolder(const AreaName& areaID, const AnyString& folder) const
{
    String buffer;
    buffer.clear() << folder << SEP << areaID;
    /* Make sure the folder is created */
    if (IO::Directory::Create(buffer))
    {
        bool ret = true;

        // Saving data
        buffer.clear() << folder << SEP << areaID << SEP << "ror.txt";
        ret = ror.timeSeries.saveToCSVFile(buffer, 0) && ret;
        buffer.clear() << folder << SEP << areaID << SEP << "mod.txt";
        ret = storage.timeSeries.saveToCSVFile(buffer, 0) && ret;
        buffer.clear() << folder << SEP << areaID << SEP << "mingen.txt";
        ret = mingen.timeSeries.saveToCSVFile(buffer, 0) && ret;
        buffer.clear() << folder << SEP << areaID << SEP << "maxHourlyGenPower.txt";
        ret = maxHourlyGenPower.timeSeries.saveToCSVFile(buffer, 0) && ret;
        buffer.clear() << folder << SEP << areaID << SEP << "maxHourlyPumpPower.txt";
        ret = maxHourlyPumpPower.timeSeries.saveToCSVFile(buffer, 0) && ret;
        buffer.clear() << folder << SEP << areaID << SEP << "maxDailyReservoirLevels.txt";
        ret = maxDailyReservoirLevels.timeSeries.saveToCSVFile(buffer, 2) && ret;
        buffer.clear() << folder << SEP << areaID << SEP << "avgDailyReservoirLevels.txt";
        ret = avgDailyReservoirLevels.timeSeries.saveToCSVFile(buffer, 2) && ret;
        buffer.clear() << folder << SEP << areaID << SEP << "minDailyReservoirLevels.txt";
        ret = minDailyReservoirLevels.timeSeries.saveToCSVFile(buffer, 2, false, true) && ret;

        return ret;
    }
    return false;
}

uint DataSeriesHydro::TScount() const
{
    const std::vector<uint32_t> nbColumns({storage.numberOfColumns(),
                                           ror.numberOfColumns(),
                                           mingen.numberOfColumns(),
                                           maxHourlyGenPower.numberOfColumns(),
                                           maxHourlyPumpPower.numberOfColumns(),
                                           maxDailyReservoirLevels.numberOfColumns(),
                                           minDailyReservoirLevels.numberOfColumns(),
                                           avgDailyReservoirLevels.numberOfColumns(),
                                           reservoirLevels.max.numberOfColumns(),
                                           reservoirLevels.min.numberOfColumns(),
                                           reservoirLevels.avg.numberOfColumns()});

    return *std::max_element(nbColumns.begin(), nbColumns.end());
}

void DataSeriesHydro::resizeTSinDeratedMode(bool derated,
                                            StudyVersion studyVersion,
                                            bool usedBySolver)
{
    if (!(derated && usedBySolver))
    {
        return;
    }

    ror.averageTimeseries();
    storage.averageTimeseries();
    if (studyVersion >= StudyVersion(8, 6))
    {
        mingen.averageTimeseries();

        if (studyVersion >= StudyVersion(9, 1))
        { // Check: Maybe we don't need check for 9.1 version, since we have conversion
            // This two objects will be created regardless of the version
            maxHourlyGenPower.averageTimeseries();
            maxHourlyPumpPower.averageTimeseries();
        }
    }
    maxDailyReservoirLevels.averageTimeseries();
    avgDailyReservoirLevels.averageTimeseries();
    minDailyReservoirLevels.averageTimeseries();
    reservoirLevels.averageTimeSeries();
}
} // namespace Antares::Data
