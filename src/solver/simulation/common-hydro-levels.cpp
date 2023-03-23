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

#include <yuni/yuni.h>
#include <yuni/core/math.h>
#include <antares/study/study.h>
#include <antares/study/memory-usage.h>
#include "common-eco-adq.h"
#include <antares/logs.h>
#include <cassert>
#include "simulation.h"
#include <antares/study/area/scratchpad.h>
#include <antares/study/parts/hydro/container.h>
#include <antares/study/parts/hydro/containerhydrocluster.h>

namespace Antares
{
namespace Solver
{
namespace Simulation
{
void computingHydroLevels(const Data::Study& study, // used as post-optimization process
                          PROBLEME_HEBDO& problem,
                          uint nbHoursInAWeek,
                          bool remixWasRun,
                          bool computeAnyway)
{
    assert(study.parameters.mode != Data::stdmAdequacyDraft);

    study.areas.each([&](const Data::Area& area) {
        if (!area.hydro.reservoirManagement)
            return;

        if (not computeAnyway)
        {
            if (area.hydro.useHeuristicTarget != remixWasRun)
                return;
        }

        uint index = area.index;

        double reservoirCapacity = area.hydro.reservoirCapacity;

        double* inflows = problem.CaracteristiquesHydrauliques[index]->ApportNaturelHoraire;

        RESULTATS_HORAIRES* weeklyResults = problem.ResultatsHoraires[index];

        double* turb = weeklyResults->TurbinageHoraire;

        double* pump = weeklyResults->PompageHoraire;
        double pumpingRatio = area.hydro.pumpingEfficiency;

        double nivInit = problem.CaracteristiquesHydrauliques[index]->NiveauInitialReservoir;
        double* niv = weeklyResults->niveauxHoraires;

        double* ovf = weeklyResults->debordementsHoraires;

        auto& computeLvlObj = problem.computeLvl_object;

        computeLvlObj.setParameters(
          nivInit, inflows, ovf, turb, pumpingRatio, pump, reservoirCapacity);

        for (uint h = 0; h < nbHoursInAWeek - 1; h++)
        {
            computeLvlObj.run();
            niv[h] = computeLvlObj.getLevel() * 100 / reservoirCapacity;
            computeLvlObj.prepareNextStep();
        }

        computeLvlObj.run();
        niv[nbHoursInAWeek - 1] = computeLvlObj.getLevel() * 100 / reservoirCapacity;
        // TODO Milos:
        // calling computeLvlObj method run with parameters:
        // nivInit, inflows, pumpingRatio, reservoirCapacity -> different for each cluster
        // while parameters:
        // ovf, turb, pump -> extracted from weekly results (from solver) -> so far one per area
        // is not applicable!  
    });
}

void interpolateWaterValue(const Data::Study& study, // used as post-optimization process
                           PROBLEME_HEBDO& problem,
                           Antares::Solver::Variable::State& state,
                           int firstHourOfTheWeek,
                           uint nbHoursInAWeek)
{
    uint daysOfWeek[7] = {0, 0, 0, 0, 0, 0, 0};

    const uint weekFirstDay = study.calendar.hours[firstHourOfTheWeek].dayYear;

    daysOfWeek[0] = weekFirstDay;
    for (int d = 1; d < 7; d++)
        daysOfWeek[d] = weekFirstDay + d;

    study.areas.each([&](const Data::Area& area) {
        uint index = area.index;

        RESULTATS_HORAIRES* weeklyResults = problem.ResultatsHoraires[index];

        double* waterVal = weeklyResults->valeurH2oHoraire;

        for (uint h = 0; h < nbHoursInAWeek; h++)
            waterVal[h] = 0.;

        if (!area.hydro.reservoirManagement || !area.hydro.useWaterValue)
            return;

        if (!area.hydro.useWaterValue)
            return;

        double reservoirCapacity = area.hydro.reservoirCapacity;

        double* niv = weeklyResults->niveauxHoraires;

        Antares::Data::getWaterValue(
          problem.previousSimulationFinalLevel[index] * 100 / reservoirCapacity,
          area.hydro.waterValues,
          weekFirstDay,
          state.h2oValueWorkVars,
          waterVal[0]);
        for (uint h = 1; h < nbHoursInAWeek; h++)
            Antares::Data::getWaterValue(niv[h - 1],
                                         area.hydro.waterValues,
                                         daysOfWeek[h / 24],
                                         state.h2oValueWorkVars,
                                         waterVal[h]);
    });
}

void updatingWeeklyFinalHydroLevel(const Data::Study& study, // used as post-optimization process
                                   PROBLEME_HEBDO& problem,
                                   uint nbHoursInAWeek)
{
    study.areas.each([&](const Data::Area& area) {
        if (!area.hydro.reservoirManagement)
            return;

        uint index = area.index;

        double reservoirCapacity = area.hydro.reservoirCapacity;

        RESULTATS_HORAIRES* weeklyResults = problem.ResultatsHoraires[index];

        double* niv = weeklyResults->niveauxHoraires;

        problem.previousSimulationFinalLevel[index]
          = niv[nbHoursInAWeek - 1] * reservoirCapacity / 100;
        // TODO Milos:
        // update final reservoir level at the end of the week -> stored in previousSimulationFinalLevel[area] (made another structure for clusters so this is ok, I have where to store it) 
        // with the final reservoir level value from the optimization-solver -> extracting this values from the weeklyResults (from solver output)!
        // this value is set as initial reservoir level at the beginning of the next week
        
        // this creates an ISSUE. We have ONE weekly results for area (for all the clusters inside one area) so far. 
        // E.g. one H.LEV [%] also one H.STOR[Mwh], H.PUMP[Mwh], H.INFL[MWh] and H.OVFL[%]
        // Do we create outputs for all the clusters? This is going to be difficult since number of clusters per area is different,
        // and we can also end up with too many columns!?
        // Or we just create structures to remember weekly results for all the hydro clusters
        // And for the final output we "sum up" cluster outputs somehow!?  
    });
}

void updatingAnnualFinalHydroLevel(const Data::Study& study, PROBLEME_HEBDO& problem) // used as post-optimization process
{
    if (!problem.hydroHotStart)
        return;

    study.areas.each([&](const Data::Area& area) {
        if (!area.hydro.reservoirManagement)
            return;

        uint index = area.index;

        double reservoirCapacity = area.hydro.reservoirCapacity;

        problem.previousYearFinalLevels[index]
          = problem.previousSimulationFinalLevel[index] / reservoirCapacity;
        // TODO Milos:
        // Only for HOT START.
        // update final reservoir level at the end of the YEAR -> stored inside previousYearFinalLevels[area] (made another structure for clusters so this is ok, I have where to store it)
        // with the final reservoir level at the end of the "last week" in that year. -> stored in previousSimulationFinalLevel[area] (made another structure for clusters so this is ok, I have where to store it)
        // this value is set as initial reservoir level at the beginning of the next YEAR

        // NO ISSUE here. Since both structures previousYearFinalLevels and previousSimulationFinalLevel are created for clusters!
    });
}

void computingHydroLevelsForCluster(const Data::Study& study,
                                    PROBLEME_HEBDO& problem,
                                    uint nbHoursInAWeek,
                                    bool remixWasRun,
                                    bool computeAnyway)
{
    assert(study.parameters.mode != Data::stdmAdequacyDraft);

    study.areas.each(
      [&](const Data::Area& area)
      {
          area.hydrocluster.list.each(
            [&](const Data::HydroclusterCluster& cluster)
            {
                if (!cluster.reservoirManagement)
                    return;

                if (not computeAnyway)
                {
                    if (cluster.useHeuristicTarget != remixWasRun)
                        return;
                }

                uint index = area.index;

                double reservoirCapacity = cluster.reservoirCapacity;

                double* inflows = problem.PaliersHydroclusterDuPays[index]
                                    .hydroClusterMap.at(cluster.index)
                                    .ApportNaturelHoraire;

                RESULTATS_HORAIRES* weeklyResults = problem.ResultatsHoraires[index];

                double* turb = weeklyResults->TurbinageHoraire;

                double* pump = weeklyResults->PompageHoraire;
                double pumpingRatio = cluster.pumpingEfficiency;

                double nivInit = problem.PaliersHydroclusterDuPays[index]
                                   .hydroClusterMap.at(cluster.index)
                                   .NiveauInitialReservoir;
                double* niv = weeklyResults->niveauxHoraires;

                double* ovf = weeklyResults->debordementsHoraires;

                auto& computeLvlObj = problem.computeLvl_object;

                computeLvlObj.setParameters(
                  nivInit, inflows, ovf, turb, pumpingRatio, pump, reservoirCapacity);

                for (uint h = 0; h < nbHoursInAWeek - 1; h++)
                {
                    computeLvlObj.run();
                    niv[h] = computeLvlObj.getLevel() * 100 / reservoirCapacity;
                    computeLvlObj.prepareNextStep();
                }

                computeLvlObj.run();
                niv[nbHoursInAWeek - 1] = computeLvlObj.getLevel() * 100 / reservoirCapacity;
            });
      });
}

void interpolateWaterValueForCluster(const Data::Study& study,
                                     PROBLEME_HEBDO& problem,
                                     Antares::Solver::Variable::State& state,
                                     int firstHourOfTheWeek,
                                     uint nbHoursInAWeek)
{
    uint daysOfWeek[7] = {0, 0, 0, 0, 0, 0, 0};

    const uint weekFirstDay = study.calendar.hours[firstHourOfTheWeek].dayYear;

    daysOfWeek[0] = weekFirstDay;
    for (int d = 1; d < 7; d++)
        daysOfWeek[d] = weekFirstDay + d;

    study.areas.each(
      [&](const Data::Area& area)
      {
          area.hydrocluster.list.each(
            [&](const Data::HydroclusterCluster& cluster)
            {
                uint index = area.index;
                RESULTATS_HORAIRES* weeklyResults = problem.ResultatsHoraires[index];

                double* waterVal = weeklyResults->valeurH2oHoraire;

                for (uint h = 0; h < nbHoursInAWeek; h++)
                    waterVal[h] = 0.;

                if (!cluster.reservoirManagement || !cluster.useWaterValue)
                    return;

                if (!cluster.useWaterValue)
                    return;

                double reservoirCapacity = cluster.reservoirCapacity;

                double* niv = weeklyResults->niveauxHoraires;

                Antares::Data::getWaterValue(
                  problem.PaliersHydroclusterDuPays[index].previousSimulationFinalLevel.at(
                    cluster.index)
                    * 100 / reservoirCapacity,
                  cluster.waterValues,
                  weekFirstDay,
                  state.h2oValueWorkVars,
                  waterVal[0]);
                for (uint h = 1; h < nbHoursInAWeek; h++)
                    Antares::Data::getWaterValue(niv[h - 1],
                                                 cluster.waterValues,
                                                 daysOfWeek[h / 24],
                                                 state.h2oValueWorkVars,
                                                 waterVal[h]);
            });
      });
}

void updatingWeeklyFinalHydroLevelForCluster(const Data::Study& study,
                                             PROBLEME_HEBDO& problem,
                                             uint nbHoursInAWeek)
{
    study.areas.each(
      [&](const Data::Area& area)
      {
          area.hydrocluster.list.each(
            [&](const Data::HydroclusterCluster& cluster)
            {
                if (!cluster.reservoirManagement)
                    return;

                uint index = area.index;

                double reservoirCapacity = cluster.reservoirCapacity;
                RESULTATS_HORAIRES* weeklyResults = problem.ResultatsHoraires[index];

                double* niv = weeklyResults->niveauxHoraires;

                problem.PaliersHydroclusterDuPays[index].previousSimulationFinalLevel.insert(
                  {cluster.index, niv[nbHoursInAWeek - 1] * reservoirCapacity / 100});
            });
      });
}

void updatingAnnualFinalHydroLevelForCluster(const Data::Study& study, PROBLEME_HEBDO& problem)
{
    if (!problem.hydroHotStart)
        return;

    study.areas.each(
      [&](const Data::Area& area)
      {
          area.hydrocluster.list.each(
            [&](const Data::HydroclusterCluster& cluster)
            {
                if (!cluster.reservoirManagement)
                    return;

                uint index = area.index;

                double reservoirCapacity = cluster.reservoirCapacity;

                problem.PaliersHydroclusterDuPays[index].previousYearFinalLevels.insert(
                  {cluster.index,
                   problem.PaliersHydroclusterDuPays[index].previousSimulationFinalLevel.at(
                     cluster.index)
                     / reservoirCapacity});
            });
      });
}

} // namespace Simulation
} // namespace Solver
} // namespace Antares
