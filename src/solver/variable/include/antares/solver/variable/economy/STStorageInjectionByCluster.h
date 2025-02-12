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
#pragma once

#include "antares/solver/variable/variable.h"

namespace Antares::Solver::Variable::Economy
{
struct VCardSTstorageInjectionByCluster
{
    //! Caption
    static std::string Caption()
    {
        return "STS inj by plant";
    }

    //! Unit
    static std::string Unit()
    {
        return "P-injection - MW";
    }

    //! The short description of the variable
    static std::string Description()
    {
        return "Energy injected by ST storage clusters";
    }

    //! The expecte results
    typedef Results<R::AllYears::Average< // The average values throughout all years
      >>
      ResultsType;

    //! The VCard to look for for calculating spatial aggregates
    typedef VCardSTstorageInjectionByCluster VCardForSpatialAggregate;

    //! Data Level
    static constexpr uint8_t categoryDataLevel = Category::DataLevel::area;
    //! File level (provided by the type of the results)
    static constexpr uint8_t categoryFileLevel = ResultsType::categoryFile
                                                 & (Category::FileLevel::de_sts);
    //! Precision (views)
    static constexpr uint8_t precision = Category::all;
    //! Indentation (GUI)
    static constexpr uint8_t nodeDepthForGUI = +0;
    //! Decimal precision
    static constexpr uint8_t decimal = 0;
    //! Number of columns used by the variable
    static constexpr int columnCount = Category::dynamicColumns;
    //! The Spatial aggregation
    static constexpr uint8_t spatialAggregate = Category::spatialAggregateSum;
    static constexpr uint8_t spatialAggregateMode = Category::spatialAggregateEachYear;
    static constexpr uint8_t spatialAggregatePostProcessing = 0;
    //! Intermediate values
    static constexpr uint8_t hasIntermediateValues = 1;
    //! Can this variable be non applicable (0 : no, 1 : yes)
    static constexpr uint8_t isPossiblyNonApplicable = 0;

    typedef IntermediateValues IntermediateValuesDeepType;
    typedef std::vector<IntermediateValues> IntermediateValuesBaseType;
    typedef std::vector<IntermediateValuesBaseType> IntermediateValuesType;

}; // class VCard

/*!
** \brief Energy generated by short term storage clusters
*/
template<class NextT = Container::EndOfList>
class STstorageInjectionByCluster: public Variable::IVariable<STstorageInjectionByCluster<NextT>,
                                                              NextT,
                                                              VCardSTstorageInjectionByCluster>
{
public:
    //! Type of the next static variable
    typedef NextT NextType;
    //! VCard
    typedef VCardSTstorageInjectionByCluster VCardType;
    //! Ancestor
    typedef Variable::IVariable<STstorageInjectionByCluster<NextT>, NextT, VCardType> AncestorType;

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
    void initializeFromArea(Data::Study* study, Data::Area* area)
    {
        // Get the number of years in parallel
        pNbYearsParallel = study->maxNbYearsInParallel;
        pValuesForTheCurrentYear.resize(pNbYearsParallel);

        // Get the area
        nbClusters_ = area->shortTermStorage.count();
        if (nbClusters_)
        {
            AncestorType::pResults.resize(nbClusters_);

            for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
            {
                pValuesForTheCurrentYear[numSpace].resize(nbClusters_);
            }

            for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
            {
                for (unsigned int i = 0; i != nbClusters_; ++i)
                {
                    pValuesForTheCurrentYear[numSpace][i].initializeFromStudy(*study);
                }
            }

            for (unsigned int i = 0; i != nbClusters_; ++i)
            {
                AncestorType::pResults[i].initializeFromStudy(*study);
                AncestorType::pResults[i].reset();
            }
        }
        else
        {
            AncestorType::pResults.clear();
        }
        // Next
        NextType::initializeFromArea(study, area);
    }

    size_t getMaxNumberColumns() const
    {
        return nbClusters_ * ResultsType::count;
    }

    void yearBegin(unsigned int year, unsigned int numSpace)
    {
        // Reset the values for the current year
        for (unsigned int i = 0; i != nbClusters_; ++i)
        {
            pValuesForTheCurrentYear[numSpace][i].reset();
        }
        // Next variable
        NextType::yearBegin(year, numSpace);
    }

    void yearEnd(unsigned int year, unsigned int numSpace)
    {
        for (unsigned int clusterIndex = 0; clusterIndex < nbClusters_; ++clusterIndex)
        {
            // Compute all statistics from hourly results for the current year (daily, weekly,
            // monthly, ...)
            pValuesForTheCurrentYear[numSpace][clusterIndex].computeStatisticsForTheCurrentYear();
        }
        // Next variable
        NextType::yearEnd(year, numSpace);
    }

    void computeSummary(std::map<unsigned int, unsigned int>& numSpaceToYear,
                        unsigned int nbYearsForCurrentSummary)
    {
        for (unsigned int numSpace = 0; numSpace < nbYearsForCurrentSummary; ++numSpace)
        {
            for (unsigned int clusterIndex = 0; clusterIndex < nbClusters_; ++clusterIndex)
            {
                // Merge all those values with the global results
                AncestorType::pResults[clusterIndex].merge(
                  numSpaceToYear[numSpace],
                  pValuesForTheCurrentYear[numSpace][clusterIndex]);
            }
        }

        // Next variable
        NextType::computeSummary(numSpaceToYear, nbYearsForCurrentSummary);
    }

    void hourBegin(unsigned int hourInTheYear)
    {
        // Next variable
        NextType::hourBegin(hourInTheYear);
    }

    void hourForEachArea(State& state, unsigned int numSpace)
    {
        for (uint clusterIndex = 0; clusterIndex != state.area->shortTermStorage.count();
             ++clusterIndex)
        {
            // ST storage injection for the current cluster and this hour
            pValuesForTheCurrentYear[numSpace][clusterIndex].hour[state.hourInTheYear]
              = state.hourlyResults->ShortTermStorage[state.hourInTheWeek].injection[clusterIndex];
        }

        // Next variable
        NextType::hourForEachArea(state, numSpace);
    }

    inline void buildDigest(SurveyResults& results, int digestLevel, int dataLevel) const
    {
        // Ask to build the digest to the next variable
        NextType::buildDigest(results, digestLevel, dataLevel);
    }

    Antares::Memory::Stored<double>::ConstReturnType retrieveRawHourlyValuesForCurrentYear(
      unsigned int column,
      unsigned int numSpace) const
    {
        return pValuesForTheCurrentYear[numSpace][column].hour;
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
            assert(NULL != results.data.area);
            const auto& shortTermStorage = results.data.area->shortTermStorage;

            // Write the data for the current year
            uint clusterIndex = 0;
            for (const auto& cluster: shortTermStorage.storagesByIndex)
            {
                // Write the data for the current year
                results.variableCaption = cluster.properties.name;
                results.variableUnit = VCardType::Unit();
                pValuesForTheCurrentYear[numSpace][clusterIndex]
                  .template buildAnnualSurveyReport<VCardType>(results, fileLevel, precision);

                clusterIndex++;
            }
        }
    }

private:
    //! Intermediate values for each year
    typename VCardType::IntermediateValuesType pValuesForTheCurrentYear;
    size_t nbClusters_;
    unsigned int pNbYearsParallel;

}; // class STstorageInjectionByCluster

} // End namespace Antares::Solver::Variable::Economy
