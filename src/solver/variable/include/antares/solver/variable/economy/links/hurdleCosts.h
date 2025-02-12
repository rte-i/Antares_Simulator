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
#ifndef __SOLVER_VARIABLE_ECONOMY_HURDLE_COSTS_H__
#define __SOLVER_VARIABLE_ECONOMY_HURDLE_COSTS_H__

#include "../../variable.h"

namespace Antares
{
namespace Solver
{
namespace Variable
{
namespace Economy
{
struct VCardHurdleCosts
{
    //! Caption
    static std::string Caption()
    {
        return "HURDLE COST";
    }

    //! Unit
    static std::string Unit()
    {
        return "Euro";
    }

    //! The short description of the variable
    static std::string Description()
    {
        return "Hurdle costs, over all MC years";
    }

    //! The expecte results
    typedef Results<R::AllYears::Average< // The average values throughout all years
      R::AllYears::StdDeviation<          // The standard deviation values throughout all years
        R::AllYears::Min<                 // The minimum values throughout all years
          R::AllYears::Max<               // The maximum values throughout all years
            >>>>>
      ResultsType;

    //! Data Level
    static constexpr uint8_t categoryDataLevel = Category::DataLevel::link;
    //! File level (provided by the type of the results)
    static constexpr uint8_t categoryFileLevel = ResultsType::categoryFile
                                                 & (Category::FileLevel::id
                                                    | Category::FileLevel::va);
    //! Precision (views)
    static constexpr uint8_t precision = Category::all;
    //! Indentation (GUI)
    static constexpr uint8_t nodeDepthForGUI = +0;
    //! Decimal precision
    static constexpr uint8_t decimal = 0;
    //! Number of columns used by the variable (One ResultsType per column)
    static constexpr int columnCount = 1;
    //! The Spatial aggregation
    static constexpr uint8_t spatialAggregate = Category::spatialAggregateSum;
    static constexpr uint8_t spatialAggregateMode = Category::spatialAggregateEachYear;
    static constexpr uint8_t spatialAggregatePostProcessing = 0;
    //! Intermediate values
    static constexpr uint8_t hasIntermediateValues = 1;
    //! Can this variable be non applicable (0 : no, 1 : yes)
    static constexpr uint8_t isPossiblyNonApplicable = 0;

    typedef IntermediateValues IntermediateValuesBaseType;
    typedef std::vector<IntermediateValues> IntermediateValuesType;

}; // class VCard

/*!
** \brief Marginal HurdleCosts
*/
template<class NextT = Container::EndOfList>
class HurdleCosts: public Variable::IVariable<HurdleCosts<NextT>, NextT, VCardHurdleCosts>
{
public:
    //! Type of the next static variable
    typedef NextT NextType;
    //! VCard
    typedef VCardHurdleCosts VCardType;
    //! Ancestor
    typedef Variable::IVariable<HurdleCosts<NextT>, NextT, VCardType> AncestorType;

    //! List of expected results
    typedef typename VCardType::ResultsType ResultsType;

    typedef VariableAccessor<ResultsType, VCardType::columnCount> VariableAccessorType;

    enum
    {
        //! How many items have we got
        count = 1 + NextT::count,
    };

    template<int CDataLevel, int CFile>
    struct Statistics
    {
        enum
        {
            count = ((VCardType::categoryDataLevel & CDataLevel
                      && VCardType::categoryFileLevel & CFile)
                       ? (NextType::template Statistics<CDataLevel, CFile>::count
                          + VCardType::columnCount * ResultsType::count)
                       : NextType::template Statistics<CDataLevel, CFile>::count),
        };
    };

public:
    void initializeFromStudy(Data::Study& study)
    {
        pNbYearsParallel = study.maxNbYearsInParallel;

        // Average on all years
        AncestorType::pResults.initializeFromStudy(study);
        AncestorType::pResults.reset();

        // Intermediate values
        pValuesForTheCurrentYear.resize(pNbYearsParallel);
        for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
        {
            pValuesForTheCurrentYear[numSpace].initializeFromStudy(study);
        }

        // Next
        NextType::initializeFromStudy(study);
    }

    void initializeFromArea(Data::Study* study, Data::Area* area)
    {
        // Next
        NextType::initializeFromArea(study, area);
    }

    void initializeFromAreaLink(Data::Study* study, Data::AreaLink* link)
    {
        // Next
        NextType::initializeFromAreaLink(study, link);
    }

    void simulationBegin()
    {
        for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
        {
            pValuesForTheCurrentYear[numSpace].reset();
        }
        // Next
        NextType::simulationBegin();
    }

    void simulationEnd()
    {
        NextType::simulationEnd();
    }

    void yearBegin(uint year, unsigned int numSpace)
    {
        // Reset
        pValuesForTheCurrentYear[numSpace].reset();
        // Next variable
        NextType::yearBegin(year, numSpace);
    }

    void yearEndBuild(State& state, unsigned int year, unsigned int numSpace)
    {
        // Next variable
        NextType::yearEndBuild(state, year, numSpace);
    }

    void yearEnd(unsigned int year, unsigned int numSpace)
    {
        // Compute all statistics for the current year (daily,weekly,monthly)
        pValuesForTheCurrentYear[numSpace].computeStatisticsForTheCurrentYear();

        // Next variable
        NextType::yearEnd(year, numSpace);
    }

    void computeSummary(std::map<unsigned int, unsigned int>& numSpaceToYear,
                        unsigned int nbYearsForCurrentSummary)
    {
        for (unsigned int numSpace = 0; numSpace < nbYearsForCurrentSummary; ++numSpace)
        {
            // Merge all those values with the global results
            AncestorType::pResults.merge(numSpaceToYear[numSpace],
                                         pValuesForTheCurrentYear[numSpace]);
        }

        // Next variable
        NextType::computeSummary(numSpaceToYear, nbYearsForCurrentSummary);
    }

    void hourForEachArea(State& state, unsigned int numSpace)
    {
        // Next variable
        NextType::hourForEachArea(state, numSpace);
    }

    void hourForEachLink(State& state, unsigned int numSpace)
    {
        // Flow assessed over all MC years (linear)
        if (state.link->useHurdlesCost)
        {
            const double flowLinear = state.ntc.ValeurDuFlux[state.link->index];

            if (state.link->useLoopFlow)
            {
                const double loopFlow = state.problemeHebdo->ValeursDeNTC[state.hourInTheWeek]
                                          .ValeurDeLoopFlowOrigineVersExtremite[state.link->index];
                if (flowLinear - loopFlow > 0.)
                {
                    const double hurdleCostDirect = (flowLinear - loopFlow)
                                                    * state.link->parameters
                                                        .entry[Data::fhlHurdlesCostDirect]
                                                              [state.hourInTheYear];
                    pValuesForTheCurrentYear[numSpace].hour[state.hourInTheYear]
                      += hurdleCostDirect;
                    // Incrementing annual system cost (to be printed in output in a separate file)
                    state.annualSystemCost += hurdleCostDirect;
                }
                else
                {
                    const double hurdleCostIndirect = -(flowLinear - loopFlow)
                                                      * state.link->parameters
                                                          .entry[Data::fhlHurdlesCostIndirect]
                                                                [state.hourInTheYear];
                    pValuesForTheCurrentYear[numSpace].hour[state.hourInTheYear]
                      += hurdleCostIndirect;
                    // Incrementing annual system cost (to be printed in output into a separate
                    // file)
                    state.annualSystemCost += hurdleCostIndirect;
                }
            }
            else
            {
                if (flowLinear > 0.)
                {
                    const double hurdleCostDirect = flowLinear
                                                    * state.link->parameters
                                                        .entry[Data::fhlHurdlesCostDirect]
                                                              [state.hourInTheYear];
                    pValuesForTheCurrentYear[numSpace].hour[state.hourInTheYear]
                      += hurdleCostDirect;
                    // Incrementing annual system cost (to be printed in output in a separate file)
                    state.annualSystemCost += hurdleCostDirect;
                }
                else
                {
                    const double hurdleCostIndirect = -flowLinear
                                                      * state.link->parameters
                                                          .entry[Data::fhlHurdlesCostIndirect]
                                                                [state.hourInTheYear];
                    pValuesForTheCurrentYear[numSpace].hour[state.hourInTheYear]
                      += hurdleCostIndirect;
                    // Incrementing annual system cost (to be printed in output into a separate
                    // file)
                    state.annualSystemCost += hurdleCostIndirect;
                }
            }
        }
        // Next item in the list
        NextType::hourForEachLink(state, numSpace);
    }

    void buildDigest(SurveyResults& results, int digestLevel, int dataLevel) const
    {
        // Next
        NextType::buildDigest(results, digestLevel, dataLevel);
    }

    Antares::Memory::Stored<double>::ConstReturnType retrieveRawHourlyValuesForCurrentYear(
      unsigned int,
      unsigned int numSpace) const
    {
        return pValuesForTheCurrentYear[numSpace].hour;
    }

    void localBuildAnnualSurveyReport(SurveyResults& results,
                                      int fileLevel,
                                      int precision,
                                      unsigned int numSpace) const
    {
        // Initializing external pointer on current variable non applicable status
        results.isCurrentVarNA = AncestorType::isNonApplicable;

        if (AncestorType::isPrinted[0])
        {
            // Write the data for the current year
            results.variableCaption = VCardType::Caption();
            results.variableUnit = VCardType::Unit();
            pValuesForTheCurrentYear[numSpace]
              .template buildAnnualSurveyReport<VCardType>(results, fileLevel, precision);
        }
    }

private:
    //! Intermediate values for each year
    typename VCardType::IntermediateValuesType pValuesForTheCurrentYear;
    unsigned int pNbYearsParallel;
}; // class HurdleCosts

} // namespace Economy
} // namespace Variable
} // namespace Solver
} // namespace Antares

#endif // __SOLVER_VARIABLE_ECONOMY_HURDLE_COSTS_H__
