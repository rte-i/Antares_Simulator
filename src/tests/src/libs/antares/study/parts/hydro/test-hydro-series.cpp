/*
 * Copyright 2007-2024, RTE (https://www.rte-france.com)
 * See AUTHORS.txt
 * SPDX-License-Identifier: MPL-2.0
 * This file is part of Antares-Simulator,
 * Adequacy and Performance assessment for interconnected energy networks.
 *
 * Antares_Simulator is free software: you can redistribute it and/or modify
 * it under the terms of the Mozilla Public Licence 2.0 as published by
 * the Mozilla Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Antares_Simulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Mozilla Public Licence 2.0 for more details.
 *
 * You should have received a copy of the Mozilla Public Licence 2.0
 * along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
 */

#define BOOST_TEST_MODULE test hydro series

#define WIN32_LEAN_AND_MEAN

#include <files-system.h>

#include <boost/test/unit_test.hpp>

#include <antares/array/matrix.h>
#include <antares/study/study.h>

#define SEP "/"

using namespace Antares::Data;
namespace fs = std::filesystem;

void fillTimeSeriesWithSpecialEnds(Matrix<double>& timeSeries, double start, double end)
{
    for (uint ts = 0; ts < timeSeries.width; ts++)
    {
        timeSeries[ts][0] = start;
        timeSeries[ts][timeSeries.height - 1] = end;
    }
}

struct Fixture
{
    Fixture()
    {
        // Create studies
        study = std::make_shared<Study>(true);

        // Add areas to studies
        area_1 = study->areaAdd("Area1");
        study->areas.rebuildIndexes();

        // Create necessary folders and files for these two areas
        createFoldersAndFiles();

        // Instantiating neccessary studies parameters
        study->header.version = Antares::Data::StudyVersion(9, 1);
        study->parameters.derated = false;

        //  Setting necessary paths
        pathToMaxHourlyGenPower_file.clear();
        pathToMaxHourlyGenPower_file = base_folder + SEP + series_folder + SEP + area_1->id.c_str()
                                       + SEP + maxHourlyGenPower_file;

        pathToMaxHourlyPumpPower_file.clear();
        pathToMaxHourlyPumpPower_file = base_folder + SEP + series_folder + SEP + area_1->id.c_str()
                                        + SEP + maxHourlyPumpPower_file;

        pathToMaxDailyReservoirLevels_file.clear();
        pathToMaxDailyReservoirLevels_file = base_folder + SEP + series_folder + SEP
                                             + area_1->id.c_str() + SEP
                                             + maxDailyReservoirLevels_file;

        pathToMinDailyReservoirLevels_file.clear();
        pathToMinDailyReservoirLevels_file = base_folder + SEP + series_folder + SEP
                                             + area_1->id.c_str() + SEP
                                             + minDailyReservoirLevels_file;

        pathToAvgDailyReservoirLevels_file.clear();
        pathToAvgDailyReservoirLevels_file = base_folder + SEP + series_folder + SEP
                                             + area_1->id.c_str() + SEP
                                             + avgDailyReservoirLevels_file;
        pathToReservoirLevels_file.clear();
        pathToReservoirLevels_file = base_folder + SEP + common_capacity_folder + SEP + "reservoir_"
                                     + area_1->id + ".txt";

        pathToSeriesFolder.clear();
        pathToSeriesFolder = base_folder + SEP + series_folder;

        pathToCommonCapacityFolder.clear();
        pathToCommonCapacityFolder = base_folder + SEP + common_capacity_folder;
    }

    void createFoldersAndFiles()
    {
        // series folder
        std::string buffer;
        createFolder(base_folder, series_folder);

        // common/capacity folder
        createFolder(base_folder, common_capacity_folder);

        // area folder
        std::string area1_folder = area_1->id.c_str();
        buffer.clear();
        buffer = base_folder + SEP + series_folder;
        createFolder(buffer, area1_folder);

        // maxHourlyGenPower and maxHourlyPumpPower files
        buffer.clear();
        buffer = base_folder + SEP + series_folder + SEP + area1_folder;
        createFile(buffer, maxHourlyGenPower_file);
        createFile(buffer, maxHourlyPumpPower_file);
        createFile(buffer, maxDailyReservoirLevels_file);
        createFile(buffer, minDailyReservoirLevels_file);
        createFile(buffer, avgDailyReservoirLevels_file);

        buffer.clear();
        buffer = base_folder + SEP + common_capacity_folder;
        std::string file_name = "reservoir_" + area_1->id + ".txt";
        createFile(buffer, file_name);
    }

    std::shared_ptr<Study> study;
    Area* area_1;
    std::string base_folder = fs::temp_directory_path().string();
    std::string series_folder = "series";
    std::string common_capacity_folder = "common/capacity";
    std::string maxHourlyGenPower_file = "maxHourlyGenPower.txt";
    std::string maxHourlyPumpPower_file = "maxHourlyPumpPower.txt";
    std::string maxDailyReservoirLevels_file = "maxDailyReservoirLevels.txt";
    std::string minDailyReservoirLevels_file = "minDailyReservoirLevels.txt";
    std::string avgDailyReservoirLevels_file = "avgDailyReservoirLevels.txt";
    std::string pathToMaxHourlyGenPower_file;
    std::string pathToMaxHourlyPumpPower_file;
    std::string pathToMaxDailyReservoirLevels_file;
    std::string pathToMinDailyReservoirLevels_file;
    std::string pathToAvgDailyReservoirLevels_file;
    std::string pathToReservoirLevels_file;
    std::string pathToCommonCapacityFolder;
    std::string pathToSeriesFolder;

    ~Fixture()
    {
        removeFolder(base_folder, series_folder);
    }
};

BOOST_AUTO_TEST_SUITE(s)

BOOST_FIXTURE_TEST_CASE(Testing_load_power_credits_matrices_equal_width, Fixture)
{
    bool ret = true;

    auto& maxHourlyGenPower = area_1->hydro.series->maxHourlyGenPower.timeSeries;
    auto& maxHourlyPumpPower = area_1->hydro.series->maxHourlyPumpPower.timeSeries;
    maxHourlyGenPower.reset(3, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(3, HOURS_PER_YEAR);

    fillTimeSeriesWithSpecialEnds(maxHourlyGenPower, 401., 402.);
    fillTimeSeriesWithSpecialEnds(maxHourlyPumpPower, 201., 202.);

    ret = maxHourlyGenPower.saveToCSVFile(pathToMaxHourlyGenPower_file, 0) && ret;
    ret = maxHourlyPumpPower.saveToCSVFile(pathToMaxHourlyPumpPower_file, 0) && ret;

    maxHourlyGenPower.reset(3, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(3, HOURS_PER_YEAR);

    ret = area_1->hydro.series->LoadMaxPower(area_1->id, pathToSeriesFolder) && ret;
    BOOST_CHECK(ret);
}

BOOST_FIXTURE_TEST_CASE(Testing_load_power_credits_both_matrix_equal_width_and_derated, Fixture)
{
    bool ret = true;
    study->parameters.derated = true;
    StudyVersion studyVersion(9, 1);
    bool usedBySolver = true;

    auto& maxHourlyGenPower = area_1->hydro.series->maxHourlyGenPower.timeSeries;
    auto& maxHourlyPumpPower = area_1->hydro.series->maxHourlyPumpPower.timeSeries;
    maxHourlyGenPower.reset(3, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(3, HOURS_PER_YEAR);

    fillTimeSeriesWithSpecialEnds(maxHourlyGenPower, 401., 402.);
    fillTimeSeriesWithSpecialEnds(maxHourlyPumpPower, 201., 202.);

    ret = maxHourlyGenPower.saveToCSVFile(pathToMaxHourlyGenPower_file, 0) && ret;
    ret = maxHourlyPumpPower.saveToCSVFile(pathToMaxHourlyPumpPower_file, 0) && ret;

    maxHourlyGenPower.reset(3, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(3, HOURS_PER_YEAR);

    ret = area_1->hydro.series->LoadMaxPower(area_1->id, pathToSeriesFolder) && ret;
    BOOST_CHECK(ret);
}

BOOST_FIXTURE_TEST_CASE(Testing_load_power_credits_matrices_different_width_case_2, Fixture)
{
    bool ret = true;

    auto& maxHourlyGenPower = area_1->hydro.series->maxHourlyGenPower.timeSeries;
    auto& maxHourlyPumpPower = area_1->hydro.series->maxHourlyPumpPower.timeSeries;
    maxHourlyGenPower.reset(3, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(2, HOURS_PER_YEAR);

    fillTimeSeriesWithSpecialEnds(maxHourlyGenPower, 401., 402.);
    fillTimeSeriesWithSpecialEnds(maxHourlyPumpPower, 201., 202.);

    ret = maxHourlyGenPower.saveToCSVFile(pathToMaxHourlyGenPower_file, 0) && ret;
    ret = maxHourlyPumpPower.saveToCSVFile(pathToMaxHourlyPumpPower_file, 0) && ret;

    maxHourlyGenPower.reset(3, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(2, HOURS_PER_YEAR);

    ret = area_1->hydro.series->LoadMaxPower(area_1->id, pathToSeriesFolder) && ret;
    BOOST_CHECK(ret);
}

BOOST_FIXTURE_TEST_CASE(Testing_load_power_credits_different_width_case_1, Fixture)
{
    bool ret = true;

    auto& maxHourlyGenPower = area_1->hydro.series->maxHourlyGenPower.timeSeries;
    auto& maxHourlyPumpPower = area_1->hydro.series->maxHourlyPumpPower.timeSeries;
    maxHourlyGenPower.reset(1, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(3, HOURS_PER_YEAR);

    fillTimeSeriesWithSpecialEnds(maxHourlyGenPower, 401., 402.);
    fillTimeSeriesWithSpecialEnds(maxHourlyPumpPower, 201., 202.);

    ret = maxHourlyGenPower.saveToCSVFile(pathToMaxHourlyGenPower_file, 0) && ret;
    ret = maxHourlyPumpPower.saveToCSVFile(pathToMaxHourlyPumpPower_file, 0) && ret;

    maxHourlyGenPower.reset(1, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(3, HOURS_PER_YEAR);

    ret = area_1->hydro.series->LoadMaxPower(area_1->id, pathToSeriesFolder) && ret;
    BOOST_CHECK(ret);
}

BOOST_FIXTURE_TEST_CASE(Testing_load_power_credits_different_width_case_2, Fixture)
{
    bool ret = true;

    auto& maxHourlyGenPower = area_1->hydro.series->maxHourlyGenPower.timeSeries;
    auto& maxHourlyPumpPower = area_1->hydro.series->maxHourlyPumpPower.timeSeries;
    maxHourlyGenPower.reset(4, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(1, HOURS_PER_YEAR);

    fillTimeSeriesWithSpecialEnds(maxHourlyGenPower, 401., 402.);
    fillTimeSeriesWithSpecialEnds(maxHourlyPumpPower, 201., 202.);

    ret = maxHourlyGenPower.saveToCSVFile(pathToMaxHourlyGenPower_file, 0) && ret;
    ret = maxHourlyPumpPower.saveToCSVFile(pathToMaxHourlyPumpPower_file, 0) && ret;

    maxHourlyGenPower.reset(4, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(1, HOURS_PER_YEAR);

    ret = area_1->hydro.series->LoadMaxPower(area_1->id, pathToSeriesFolder) && ret;
    BOOST_CHECK(ret);
}

BOOST_FIXTURE_TEST_CASE(Testing_load_power_credits_both_zeros, Fixture)
{
    bool ret = true;

    auto& maxHourlyGenPower = area_1->hydro.series->maxHourlyGenPower.timeSeries;
    auto& maxHourlyPumpPower = area_1->hydro.series->maxHourlyPumpPower.timeSeries;
    maxHourlyGenPower.reset(4, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(1, HOURS_PER_YEAR);

    fillTimeSeriesWithSpecialEnds(maxHourlyGenPower, 401., 402.);
    fillTimeSeriesWithSpecialEnds(maxHourlyPumpPower, 201., 202.);

    ret = maxHourlyGenPower.saveToCSVFile(pathToMaxHourlyGenPower_file, 0) && ret;
    ret = maxHourlyPumpPower.saveToCSVFile(pathToMaxHourlyPumpPower_file, 0) && ret;

    maxHourlyGenPower.reset(4, HOURS_PER_YEAR);
    maxHourlyPumpPower.reset(1, HOURS_PER_YEAR);

    ret = area_1->hydro.series->LoadMaxPower(area_1->id, pathToSeriesFolder) && ret;
    BOOST_CHECK(ret);
}

BOOST_FIXTURE_TEST_CASE(Testing_load_reservoir_levels_matrices_equal_width, Fixture)
{
    bool ret = true;

    auto& maxDailyReservoirLevels = area_1->hydro.series->reservoirLevels.max.timeSeries;
    auto& minDailyReservoirLevels = area_1->hydro.series->reservoirLevels.min.timeSeries;
    auto& avgDailyReservoirLevels = area_1->hydro.series->reservoirLevels.avg.timeSeries;

    maxDailyReservoirLevels.reset(3, DAYS_PER_YEAR);
    minDailyReservoirLevels.reset(3, DAYS_PER_YEAR);
    avgDailyReservoirLevels.reset(3, DAYS_PER_YEAR);

    fillTimeSeriesWithSpecialEnds(maxDailyReservoirLevels, 0.8, 0.7);
    fillTimeSeriesWithSpecialEnds(minDailyReservoirLevels, 0.5, 0.6);
    fillTimeSeriesWithSpecialEnds(avgDailyReservoirLevels, 0.3, 0.4);

    ret = maxDailyReservoirLevels.saveToCSVFile(pathToMaxDailyReservoirLevels_file, 2) && ret;
    ret = minDailyReservoirLevels.saveToCSVFile(pathToMinDailyReservoirLevels_file, 2) && ret;
    ret = avgDailyReservoirLevels.saveToCSVFile(pathToAvgDailyReservoirLevels_file, 2) && ret;

    maxDailyReservoirLevels.reset(3, DAYS_PER_YEAR);
    minDailyReservoirLevels.reset(3, DAYS_PER_YEAR);
    avgDailyReservoirLevels.reset(3, DAYS_PER_YEAR);

    ret = area_1->hydro.series->reservoirLevels
            .loadScenarizedReservoirLevels(area_1->id, pathToSeriesFolder, study->usedByTheSolver)
          && ret;
    BOOST_CHECK(ret);
}

BOOST_FIXTURE_TEST_CASE(Testing_load_reservoir_levels_from_common_capacity_folder, Fixture)
{
    bool ret = true;

    auto& maxDailyReservoirLevels = area_1->hydro.series->reservoirLevels.max.timeSeries;
    auto& minDailyReservoirLevels = area_1->hydro.series->reservoirLevels.min.timeSeries;
    auto& avgDailyReservoirLevels = area_1->hydro.series->reservoirLevels.avg.timeSeries;
    auto& reservoirLevels = area_1->hydro.series->reservoirLevels.Buffer;

    reservoirLevels.reset(3, DAYS_PER_YEAR, true);

    reservoirLevels.fillColumn(ReservoirLevels::maximum, 1.);
    reservoirLevels.fillColumn(ReservoirLevels::average, 0.5);

    reservoirLevels[ReservoirLevels::maximum][0] = 0.9;
    reservoirLevels[ReservoirLevels::maximum][DAYS_PER_YEAR - 1] = 0.8;

    reservoirLevels[ReservoirLevels::average][0] = 0.5;
    reservoirLevels[ReservoirLevels::average][DAYS_PER_YEAR - 1] = 0.6;

    reservoirLevels[ReservoirLevels::minimum][0] = 0.1;
    reservoirLevels[ReservoirLevels::minimum][DAYS_PER_YEAR - 1] = 0.2;

    ret = reservoirLevels.saveToCSVFile(pathToReservoirLevels_file, 2) && ret;

    reservoirLevels.reset(3, DAYS_PER_YEAR, true);

    ret = area_1->hydro.series->reservoirLevels.loadReservoirLevels(area_1->id,
                                                                    pathToCommonCapacityFolder,
                                                                    study->usedByTheSolver)
          && ret;
    BOOST_CHECK(ret);
    BOOST_CHECK(maxDailyReservoirLevels[0][0] == 0.9
                && maxDailyReservoirLevels[0][DAYS_PER_YEAR - 1] == 0.8);
    BOOST_CHECK(avgDailyReservoirLevels[0][0] == 0.5
                && avgDailyReservoirLevels[0][DAYS_PER_YEAR - 1] == 0.6);
    BOOST_CHECK(minDailyReservoirLevels[0][0] == 0.1
                && minDailyReservoirLevels[0][DAYS_PER_YEAR - 1] == 0.2);
}

BOOST_AUTO_TEST_SUITE_END()
