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
#ifndef __SOLVER_VARIABLE_ADEQUACY_ALL_H__
#define __SOLVER_VARIABLE_ADEQUACY_ALL_H__

#include "antares/solver/variable/adequacy/links.h"
#include "antares/solver/variable/adequacy/overallCost.h"
#include "antares/solver/variable/adequacy/spilledEnergy.h"
#include "antares/solver/variable/area.h"
#include "antares/solver/variable/bindConstraints.h"
#include "antares/solver/variable/commons/hydro.h"
#include "antares/solver/variable/commons/join.h"
#include "antares/solver/variable/commons/load.h"
#include "antares/solver/variable/commons/miscGenMinusRowPSP.h"
#include "antares/solver/variable/commons/psp.h"
#include "antares/solver/variable/commons/rowBalance.h"
#include "antares/solver/variable/commons/solar.h"
#include "antares/solver/variable/commons/spatial-aggregate.h"
#include "antares/solver/variable/commons/wind.h"
#include "antares/solver/variable/economy/STSbyGroup.h"
#include "antares/solver/variable/economy/STStorageInjectionByCluster.h"
#include "antares/solver/variable/economy/STStorageLevelsByCluster.h"
#include "antares/solver/variable/economy/STStorageWithdrawalByCluster.h"
#include "antares/solver/variable/economy/avail-dispatchable-generation.h"
#include "antares/solver/variable/economy/balance.h"
#include "antares/solver/variable/economy/dispatchable-generation-margin.h"
#include "antares/solver/variable/economy/dispatchableGeneration.h"
#include "antares/solver/variable/economy/hydroCost.h"
#include "antares/solver/variable/economy/hydrostorage.h"
#include "antares/solver/variable/economy/inflow.h"
#include "antares/solver/variable/economy/lold.h"
#include "antares/solver/variable/economy/lolp.h"
#include "antares/solver/variable/economy/max-mrg.h"
#include "antares/solver/variable/economy/operatingCost.h"
#include "antares/solver/variable/economy/overflow.h"
#include "antares/solver/variable/economy/price.h"
#include "antares/solver/variable/economy/productionByDispatchablePlant.h"
#include "antares/solver/variable/economy/productionByRenewablePlant.h"
#include "antares/solver/variable/economy/pumping.h"
#include "antares/solver/variable/economy/renewableGeneration.h"
#include "antares/solver/variable/economy/reservoirlevel.h"
#include "antares/solver/variable/economy/thermalAirPollutantEmissions.h"
#include "antares/solver/variable/economy/unsupliedEnergy.h"
#include "antares/solver/variable/economy/waterValue.h"
#include "antares/solver/variable/setofareas.h"
#include "antares/solver/variable/variable.h"

// Output variables associated to binding constraints
#include "antares/solver/variable//economy/bindingConstraints/bindingConstraintsMarginalCost.h"

// By thermal plant
#include "../economy/profitByPlant.h"

namespace Antares
{
namespace Solver
{
namespace Variable
{
namespace Adequacy
{
/*!
** \brief All variables for a single area (economy)
*/
typedef                                              // Prices
  Variable::Adequacy::OverallCost                    // Overall Cost (Op. Cost + Unsupplied Eng.)
  <Variable::Economy::OperatingCost                  // Operating Cost
   <Variable::Economy::Price                         // Marginal price
                                                     // pollutant
    <Variable::Economy::ThermalAirPollutantEmissions // Pollutant emissions (from all thermal
                                                     // dispatchable clusters) Production by thermal
                                                     // cluster
     <Variable::Economy::ProductionByDispatchablePlant  // Energy generated by thermal dispatchable
                                                        // clusters
      <Variable::Economy::ProductionByRenewablePlant    // Energy generated by renewable clusters
                                                        // (must-run)
       <Variable::Economy::Balance                      // Nodal Energy Balance
                                                        // Misc Gen.
        <Variable::Economy::RowBalance                  // Misc Gen. Row balance
         <Variable::Economy::PSP                        // PSP
          <Variable::Economy::MiscGenMinusRowPSP        // Misc Gen. - Row Balance - PSP
                                                        // Time series
           <Variable::Economy::TimeSeriesValuesLoad     // Load
            <Variable::Economy::TimeSeriesValuesHydro   // Hydro
             <Variable::Economy::TimeSeriesValuesWind   // Wind
              <Variable::Economy::TimeSeriesValuesSolar // Solar
                                                        // Other
               <Variable::Economy::DispatchableGeneration // All dispatchable generation
                <Variable::Economy::RenewableGeneration<
                  Variable::Economy::HydroStorage     // Hydro Storage Generation
                  <Variable::Economy::Pumping         // Pumping generation
                   <Variable::Economy::ReservoirLevel // Reservoir levels
                    <Variable::Economy::Inflows       // Hydraulic inflows
                     <Variable::Economy::Overflows    // Hydraulic overflows
                      <Variable::Economy::WaterValue  // Water values
                       <Variable::Economy::HydroCost  // Hydro costs
                        <Variable::Economy::STSbyGroup<
                          Variable::Economy::STstorageInjectionByCluster<
                            Variable::Economy::STstorageWithdrawalByCluster<
                              Variable::Economy::STstorageLevelsByCluster<
                                Variable::Economy::UnsupliedEnergy // Unsuplied Energy
                                <Variable::Adequacy::SpilledEnergy // Spilled Energy
                                 <Variable::Economy::LOLD          // LOLD
                                  <Variable::Economy::LOLP         // LOLP
                                   <Variable::Economy::AvailableDispatchGen<
                                     Variable::Economy::DispatchableGenMargin<
                                       Variable::Economy::Marge // OP. MRG
                                       <Variable::Economy::ProfitByPlant
                                        // Links
                                        <Variable::Adequacy::Links // All links
                                         >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    VariablesPerArea;

/*!
** \brief All variables for a single set of areas (economy)
*/
typedef // Prices
  Common::SpatialAggregate<
    Variable::Adequacy::OverallCost,
    Common::SpatialAggregate<
      Variable::Economy::OperatingCost,
      Common::SpatialAggregate<
        Variable::Economy::Price,
        // pollutant
        Common::SpatialAggregate<
          Variable::Economy::ThermalAirPollutantEmissions,
          // Production by thermal cluster
          Common::SpatialAggregate<
            Variable::Economy::Balance,
            // Misc Gen.
            Common::SpatialAggregate<
              Variable::Economy::RowBalance,
              Common::SpatialAggregate<
                Variable::Economy::PSP,
                Common::SpatialAggregate<
                  Variable::Economy::MiscGenMinusRowPSP,
                  // Time series
                  Common::SpatialAggregate<
                    Variable::Economy::TimeSeriesValuesLoad,
                    Common::SpatialAggregate<
                      Variable::Economy::TimeSeriesValuesHydro,
                      Common::SpatialAggregate<
                        Variable::Economy::TimeSeriesValuesWind,
                        Common::SpatialAggregate<
                          Variable::Economy::TimeSeriesValuesSolar,
                          // Other
                          Common::SpatialAggregate<
                            Variable::Economy::DispatchableGeneration,
                            Common::SpatialAggregate<
                              Variable::Economy::RenewableGeneration,
                              Common::SpatialAggregate<
                                Variable::Economy::HydroStorage,
                                Common::SpatialAggregate<
                                  Variable::Economy::Pumping,
                                  Common::SpatialAggregate<
                                    Variable::Economy::ReservoirLevel,
                                    Common::SpatialAggregate<
                                      Variable::Economy::Inflows,
                                      Common::SpatialAggregate<
                                        Variable::Economy::Overflows,
                                        Common::SpatialAggregate<
                                          Variable::Economy::WaterValue,
                                          Common::SpatialAggregate<
                                            Variable::Economy::HydroCost,
                                            Common::SpatialAggregate<
                                              Variable::Economy::UnsupliedEnergy,
                                              Common::SpatialAggregate<
                                                Variable::Adequacy::SpilledEnergy,
                                                // LOLD
                                                Common::SpatialAggregate<
                                                  Variable::Economy::LOLD,
                                                  Common::SpatialAggregate<
                                                    Variable::Economy::LOLP,

                                                    Common::SpatialAggregate<
                                                      Variable::Economy::AvailableDispatchGen,
                                                      Common::SpatialAggregate<
                                                        Variable::Economy::DispatchableGenMargin,
                                                        Common::SpatialAggregate<
                                                          Variable::Economy::
                                                            Marge>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    VariablesPerSetOfAreas;

typedef Variable::Economy::BindingConstMarginCost< // Marginal cost for a binding constraint
  Container::EndOfList                             // End of variable list
  >

  VariablesPerBindingConstraints;

typedef Variable::Join<
  // Variables for each area / links attached to the areas
  Variable::Areas<VariablesPerArea>,
  // Variables for each set of areas
  Variable::Join<Variable::SetsOfAreas<VariablesPerSetOfAreas>,
                 // Variables for each binding constraint
                 Variable::BindingConstraints<VariablesPerBindingConstraints>>>
  ItemList;

/*!
** \brief All variables for a simulation (economy)
*/
typedef Container::List<ItemList> AllVariables;

} // namespace Adequacy
} // namespace Variable
} // namespace Solver
} // namespace Antares

#endif // __SOLVER_VARIABLE_ADEQUACY_ALL_H__
