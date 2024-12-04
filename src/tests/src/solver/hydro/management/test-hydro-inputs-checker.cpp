//
// Created by Nikola Ilic on 23/06/23.
//

#define BOOST_TEST_MODULE hydro - inputs - checker
#define WIN32_LEAN_AND_MEAN
#include <boost/test/unit_test.hpp>

#include <antares/study/study.h>
#include "antares/solver/hydro/management/HydroErrorsCollector.h"
#include "antares/solver/hydro/management/HydroInputsChecker.h"
#include "fatal-error.h"


using namespace Antares::Solver;
using namespace Antares::Data;

struct Fixture
{
    Fixture(const Fixture& f) = delete;
    Fixture(const Fixture&& f) = delete;
    Fixture& operator=(const Fixture& f) = delete;
    Fixture& operator=(const Fixture&& f) = delete;

    Fixture()
    {
        // Simulation last day must be 365 so that final level checks succeeds
        study->parameters.simulationDays.end = 365;
        study->parameters.firstMonthInYear = january;
        uint nbYears = study->parameters.nbYears = 1;

        area_1 = study->areaAdd("Area1");

        area_1->hydro.reservoirManagement = false;
        area_1->hydro.followLoadModulations = false;
        area_1->hydro.useHeuristicTarget = false;

        area_1->hydro.series->timeseriesNumbers.reset(nbYears);

        area_1->hydro.series->timeseriesNumbers[0] = 0;

        area_1->hydro.deltaBetweenFinalAndInitialLevels.resize(nbYears);

        area_1->hydro.series->storage.timeSeries.resize(1, DAYS_PER_YEAR);
        area_1->hydro.series->ror.timeSeries.resize(1, HOURS_PER_YEAR);
        area_1->hydro.series->mingen.timeSeries.resize(1, HOURS_PER_YEAR);
        area_1->hydro.series->maxHourlyGenPower.timeSeries.resize(1, HOURS_PER_YEAR);
        area_1->hydro.series->maxHourlyPumpPower.timeSeries.resize(1, HOURS_PER_YEAR);
        area_1->hydro.series->maxDailyReservoirLevels.timeSeries.resize(1, DAYS_PER_YEAR);
        area_1->hydro.series->avgDailyReservoirLevels.timeSeries.resize(1, DAYS_PER_YEAR);
        area_1->hydro.series->minDailyReservoirLevels.timeSeries.resize(1, DAYS_PER_YEAR);

        area_1->hydro.series->storage.timeSeries.fill(400.);
        area_1->hydro.series->ror.timeSeries.fill(100.);
        area_1->hydro.series->mingen.timeSeries.fill(50.);
        area_1->hydro.series->maxHourlyGenPower.timeSeries.fill(300.);
        area_1->hydro.series->maxHourlyPumpPower.timeSeries.fill(200.);
        area_1->hydro.series->maxDailyReservoirLevels.timeSeries.fill(0.8);
        area_1->hydro.series->minDailyReservoirLevels.timeSeries.fill(0.4);
        area_1->hydro.series->avgDailyReservoirLevels.timeSeries.fill(0.6);
    }

    ~Fixture() = default;

    Study::Ptr study = std::make_shared<Study>();
    Area* area_1;
    std::shared_ptr<HydroInputsChecker> hydroInputsChecker = std::make_shared<HydroInputsChecker>(
      *study);
};

BOOST_FIXTURE_TEST_SUITE(hydro_inputs_checker, Fixture)

BOOST_AUTO_TEST_CASE(reservoir_levels_are_valid)
{
    uint year = 0;
    hydroInputsChecker->Execute(0);
    BOOST_CHECK_NO_THROW(hydroInputsChecker->CheckForErrors());
}

BOOST_AUTO_TEST_CASE(reservoir_levels_are_invalid_case_1)
{
    uint year = 0;
    area_1->hydro.series->minDailyReservoirLevels.timeSeries[0][4] = 0.9;
    hydroInputsChecker->Execute(0);
    BOOST_CHECK_THROW(hydroInputsChecker->CheckForErrors(), FatalError);
}

BOOST_AUTO_TEST_CASE(reservoir_levels_are_invalid_case_2)
{
    uint year = 0;
    area_1->hydro.series->maxDailyReservoirLevels.timeSeries[0][4] = 1.1;
    hydroInputsChecker->Execute(0);
    BOOST_CHECK_THROW(hydroInputsChecker->CheckForErrors(), FatalError);
}

BOOST_AUTO_TEST_CASE(reservoir_levels_are_invalid_case_3)
{
    uint year = 0;
    area_1->hydro.series->avgDailyReservoirLevels.timeSeries[0][4] = 1.1;
    hydroInputsChecker->Execute(0);
    BOOST_CHECK_THROW(hydroInputsChecker->CheckForErrors(), FatalError);
}

BOOST_AUTO_TEST_CASE(reservoir_levels_are_invalid_case_4)
{
    uint year = 0;
    area_1->hydro.series->minDailyReservoirLevels.timeSeries[0][4] = -0.1;
    hydroInputsChecker->Execute(0);
    BOOST_CHECK_THROW(hydroInputsChecker->CheckForErrors(), FatalError);
}

BOOST_AUTO_TEST_SUITE_END()
