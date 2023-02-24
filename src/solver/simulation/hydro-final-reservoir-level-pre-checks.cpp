/*
** Copyright 2007-2018 RTE
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

#include "hydro-final-reservoir-level-pre-checks.h"
#include <antares/emergency.h>
// #include "../solver/simulation/sim_extern_variables_globales.h"

namespace Antares
{
namespace Solver
{
void FinalReservoirLevelPreChecks(Data::Study& study)
{
    bool preChecksPasses = true;
    uint simEndDay = study.parameters.simulationDays.end;
    for (uint tsIndex = 0; tsIndex != study.scenarioFinalHydroLevels.height; ++tsIndex)
    {
        study.areas.each(
          [&](Data::Area& area)
          {
              // TODO CR25:
              /*at this point the pre-checks are done for all MC before running the simulation
              and the simulation is ended immediately not waisting user time!
              However, at this point "tsIndex = (uint)ptchro.Hydraulique" is not assigned yet
              so the pre-checks are done for all MC years (whether they are used or not in hydro
              scenario-builder) this can lead to error reporting for MC years that are not used (not
              the case when pre-checks done im management.cpp)
              */
              auto& inflowsmatrix = area.hydro.series->storage;
              auto const& srcinflows = inflowsmatrix[tsIndex < inflowsmatrix.width ? tsIndex : 0];
              double initialReservoirLevel = study.scenarioHydroLevels[area.index][tsIndex];
              double finalReservoirLevel = study.scenarioFinalHydroLevels[area.index][tsIndex];
              double deltaReservoirLevel = 0.0;
              int simEndRealMonth = 0;
              // FinalReservoirLevelRuntimeData
              auto& finLevData = area.hydro.finalReservoirLevelRuntimeData;
              finLevData.includeFinalReservoirLevel.push_back(false);
              finLevData.finResLevelMode.push_back(
                Antares::Data::FinalReservoirLevelMode::completeYear);
              finLevData.deltaLevel.push_back(deltaReservoirLevel);
              finLevData.endLevel.push_back(deltaReservoirLevel);
              finLevData.endMonthIndex.push_back(simEndRealMonth);

              if (area.hydro.reservoirManagement && !area.hydro.useWaterValue
                  && !isnan(finalReservoirLevel) && !isnan(initialReservoirLevel))
              {
                  // simEndDayReal
                  uint simEndDayReal = simEndDay;
                  // deltaReservoirLevel
                  deltaReservoirLevel = initialReservoirLevel - finalReservoirLevel;
                  // collect data for pre-checks
                  int initReservoirLvlMonth
                    = area.hydro.initializeReservoirLevelDate; // month [0-11]
                  int initReservoirLvlDay
                    = study.calendar.months[initReservoirLvlMonth].daysYear.first;
                  double reservoirCapacity = area.hydro.reservoirCapacity;
                  double totalYearInflows = 0.0;
                  // FinalReservoirLevelRuntimeData
                  finLevData.includeFinalReservoirLevel.at(tsIndex) = true;
                  finLevData.endLevel.at(tsIndex) = finalReservoirLevel;
                  finLevData.deltaLevel.at(tsIndex) = deltaReservoirLevel;
                  if (initReservoirLvlDay == 0 && simEndDay == DAYS_PER_YEAR)
                  {
                      finLevData.finResLevelMode.at(tsIndex)
                        = Antares::Data::FinalReservoirLevelMode::completeYear;
                  }
                  else if (initReservoirLvlDay != 0 && simEndDay == DAYS_PER_YEAR)
                  {
                      simEndRealMonth = 0;
                      finLevData.finResLevelMode.at(tsIndex)
                        = Antares::Data::FinalReservoirLevelMode::incompleteYear;
                  }
                  else
                  {
                      uint simEndMonth = study.calendar.days[simEndDay].month;
                      uint simEnd_MonthFirstDay = study.calendar.months[simEndMonth].daysYear.first;
                      uint simEnd_MonthLastDay = study.calendar.months[simEndMonth].daysYear.end;

                      simEndRealMonth
                        = (simEndDay - simEnd_MonthFirstDay) <= (simEnd_MonthLastDay - simEndDay)
                            ? simEndMonth
                            : simEndMonth + 1;
                      finLevData.finResLevelMode.at(tsIndex)
                        = Antares::Data::FinalReservoirLevelMode::incompleteYear;

                      if (simEndRealMonth == 12 && initReservoirLvlDay == 0)
                      {
                          simEndRealMonth = 0;
                          finLevData.finResLevelMode.at(tsIndex)
                            = Antares::Data::FinalReservoirLevelMode::completeYear;
                      }
                      // E.g. End Date = 21.Dec && InitReservoirLevelDate = 1.Jan ->
                      // - > go back to first case
                      else if (simEndRealMonth == 12 && initReservoirLvlDay != 0)
                          simEndRealMonth = 0;
                      // End Date = 21.Dec && InitReservoirLevelDate = 1.Mar
                      // Reach FinalReservoirLevel at 1.Jan
                      else if (simEndRealMonth == initReservoirLvlMonth
                               && simEndDay >= initReservoirLvlDay)
                          simEndRealMonth = (simEndRealMonth + 1) % 12;
                      // E.g. End Date = 10.Jan && InitReservoirLevelDate = 1.Jan ->
                      // we need to move FinalReservoirLevel to 1.Feb.
                      // Cannot do both init and final on the same day
                      else if (simEndRealMonth == initReservoirLvlMonth
                               && simEndDay < initReservoirLvlDay)
                          simEndRealMonth = (simEndRealMonth - 1) % 12;
                      // E.g. End Date = 25.Nov && InitReservoirLevelDate = 1.Dec ->
                      // we need to move FinalReservoirLevel to 1.Nov.
                      // Cannot do both init and final on the same day

                      // log out that the final reservoir level will be reached on some other day
                      if (simEndDay != study.calendar.months[simEndRealMonth].daysYear.first)
                      {
                          simEndDayReal = study.calendar.months[simEndRealMonth].daysYear.first;
                          logs.info()
                            << "Year: " << tsIndex + 1 << ". Area: " << area.name
                            << ". Final reservoir level will be reached on day: " << simEndDayReal;
                      }
                  }
                  // Now convert to month if initialization is not done in January
                  int h20_solver_sim_end_month = (simEndRealMonth - initReservoirLvlMonth) >= 0
                                                   ? simEndRealMonth - initReservoirLvlMonth
                                                   : simEndRealMonth - initReservoirLvlMonth + 12;
                  finLevData.endMonthIndex.at(tsIndex) = h20_solver_sim_end_month;

                  logs.debug() << "tsIndex: " << tsIndex;
                  logs.debug() << "includeFinalReservoirLevel: "
                               << to_string(finLevData.includeFinalReservoirLevel.at(tsIndex));
                  logs.debug() << "finResLevelMode: "
                               << to_string(finLevData.finResLevelMode.at(tsIndex));
                  logs.debug() << "deltaLevel: " << finLevData.deltaLevel.at(tsIndex);
                  logs.debug() << "endLevel: " << finLevData.endLevel.at(tsIndex);
                  logs.debug() << "realMonth-SimEnd: " << simEndRealMonth;
                  logs.debug() << "endMonthIndex_h20_solver: " << finLevData.endMonthIndex.at(tsIndex);
                  logs.debug() << "simEndDayReal: " << simEndDayReal;
                  logs.debug() << "initReservoirLvlDay: " << initReservoirLvlDay;

                  // rule curve values for simEndDayReal
                  double lowLevelLastDay
                    = area.hydro.reservoirLevel[Data::PartHydro::minimum][simEndDayReal - 1];
                  double highLevelLastDay
                    = area.hydro.reservoirLevel[Data::PartHydro::maximum][simEndDayReal - 1];
                  // calculate (partial)yearly inflows
                  if (initReservoirLvlDay <= simEndDayReal)
                  {
                      for (uint day = initReservoirLvlDay; day < simEndDayReal; ++day)
                          totalYearInflows += srcinflows[day];
                  }
                  else
                  {
                      for (uint day = initReservoirLvlDay; day < DAYS_PER_YEAR; ++day)
                          totalYearInflows += srcinflows[day];
                      for (uint day = 0; day < simEndDayReal; ++day)
                          totalYearInflows += srcinflows[day];
                  }
                  // pre-check 1 -> reservoir_levelDay_365 – reservoir_levelDay_1 ≤
                  // yearly_inflows
                  if ((finalReservoirLevel - initialReservoirLevel) * reservoirCapacity
                      > totalYearInflows) // ROR time-series in MW (power), SP time-series in MWh
                                          // (energy)
                  {
                      logs.error() << "Year: " << tsIndex + 1 << ". Area: " << area.name
                                   << ". Incompatible total inflows: " << totalYearInflows
                                   << " with initial: " << initialReservoirLevel
                                   << " and final: " << finalReservoirLevel << " reservoir levels.";
                      preChecksPasses = false;
                  }
                  // pre-check 2 -> final reservoir level set by the user is within the
                  // rule curves for the final day
                  if (finalReservoirLevel < lowLevelLastDay
                      || finalReservoirLevel > highLevelLastDay)
                  {
                      logs.error() << "Year: " << tsIndex + 1 << ". Area: " << area.name
                                   << ". Specifed final reservoir level: " << finalReservoirLevel
                                   << " is incompatible with reservoir level rule curve ["
                                   << lowLevelLastDay << " , " << highLevelLastDay << "]";
                      preChecksPasses = false;
                  }
              }
          });
    }
    if (!preChecksPasses)
    {
        logs.fatal() << "At least one year has failed final reservoir level pre-checks.";
        AntaresSolverEmergencyShutdown();
    }
}

} // namespace Solver
} // namespace Antares