//
// Created by milos on 10/11/23.
//

#pragma once

#include "ortools/linear_solver/linear_solver.h"
#include "../../randomized-thermal-generator/RandomizedGenerator.h"
#include "../../../../libs/antares/study/maintenance_planning/MaintenanceGroup.h"
#include "../auxillary/AuxillaryStructures.h"
#include "../auxillary/AuxillaryFreeFunctions.h"
#include "../parameters/OptimizationParameters.h"
#include <antares/exception/AssertionError.hpp>

#include <iostream>
#include <fstream>

// static const std::string mntPlSolverName = "cbc";
static const int minNumberOfMaintenances = 2;
static const double solverDelta = 10e-4;

using namespace operations_research;

namespace Antares::Solver::TSGenerator
{

class OptimizedThermalGenerator : public GeneratorTempData
{
    using OptimizationResults = std::vector<Unit>;

private:
    /* ===================OPTIMIZATION=================== */

    // functions to build problem variables
    void buildProblemVariables(const OptProblemSettings& optSett);
    void buildEnsAndSpillageVariables(const OptProblemSettings& optSett);
    void buildUnitPowerOutputVariables(const OptProblemSettings& optSett);
    void buildUnitPowerOutputVariables(const OptProblemSettings& optSett,
                                       const Data::ThermalCluster& cluster);
    void buildUnitPowerOutputVariables(const OptProblemSettings& optSett,
                                       const Data::ThermalCluster& cluster,
                                       int unit);
    void buildStartEndMntVariables(const OptProblemSettings& optSett,
                                   const Data::ThermalCluster& cluster,
                                   int unit,
                                   Unit& unitRef);
    void buildStartVariables(const OptProblemSettings& optSett,
                             const Data ::ThermalCluster& cluster,
                             int unit,
                             Unit& unitRef,
                             int mnt);
    void buildEndVariables(const OptProblemSettings& optSett,
                           const Data ::ThermalCluster& cluster,
                           int unit,
                           Unit& unitRef,
                           int mnt);

    // functions to fix bounds of some variables
    void fixBounds();
    void fixBounds(const Unit& unit);
    void fixBounds(const Unit& unit, int averageMaintenanceDuration);
    void fixBoundsFirstMnt(const Unit& unit);
    void fixBoundsStartSecondMnt(const Unit& unit, int mnt);
    void fixBoundsMntEnd(const Unit& unit, int mnt, int averageMaintenanceDuration);

    // functions to build problem constraints
    void buildProblemConstraints(const OptProblemSettings& optSett);
    void setLoadBalanceConstraints(const OptProblemSettings& optSett);
    void setLoadBalanceConstraints(const OptProblemSettings& optSett, int& day);
    void insertEnsVars(MPConstraint* ct, int day);
    void insertSpillVars(MPConstraint* ct, int day);
    void insertPowerVars(MPConstraint* ct, int day);
    void insertPowerVars(MPConstraint* ct, int day, const Unit& unit);
    void setStartEndMntLogicConstraints(const OptProblemSettings& optSett);
    void setStartEndMntLogicConstraints(const OptProblemSettings& optSett, const Unit& unit);
    void setEndOfMaintenanceEventBasedOnAverageDurationOfMaintenanceEvent(
      const OptProblemSettings& optSett,
      const Unit& unit,
      int mnt);
    void setUpFollowingMaintenanceBasedOnAverageDurationBetweenMaintenanceEvents(
      const OptProblemSettings& optSett,
      const Unit& unit,
      int mnt);
    void setOnceStartIsSetToOneItWillBeOneUntilEndOfOptimizationTimeHorizon(
      const OptProblemSettings& optSett,
      const Unit& unit,
      int mnt);
    void setNextMaintenanceCanNotStartBeforePreviousMaintenance(const OptProblemSettings& optSett,
                                                                const Unit& cluster,
                                                                int mnt);
    void setMaxUnitOutputConstraints(const OptProblemSettings& optSett);
    void setMaxUnitOutputConstraints(const OptProblemSettings& optSett, int& day);
    void setMaxUnitOutputConstraints(const OptProblemSettings& optSett, int day, const Unit& unit);
    void insertStartSum(MPConstraint* ct, int day, const Unit& unit, double maxPower);
    void insertEndSum(MPConstraint* ct, int day, const Unit& cluster, double maxPower);

    // functions to set problem objective function
    void setProblemCost(const OptProblemSettings& optSett);
    void setProblemEnsCost(MPObjective* objective);
    void setProblemSpillCost(MPObjective* objective);
    void setProblemPowerCost(const OptProblemSettings& optSett, MPObjective* objective);
    void setProblemPowerCost(const OptProblemSettings& optSett,
                             MPObjective* objective,
                             const Unit& unit);

    // solve problem and check if optimal solution found
    bool solveProblem(OptProblemSettings& optSett);

    // reset problem and variable structure
    void resetProblem();

    /* ===================END-OPTIMIZATION=================== */

    /* ===================MAIN=================== */

    // Functions called in main method:
    void allocateWhereToWriteTs();
    bool runOptimizationProblem(OptProblemSettings& optSett);
    void writeTsResults();

    /* ===================END-MAIN=================== */

    /* ===================CLASS-VARIABLES=================== */

    // variables
    Data::MaintenanceGroup& maintenanceGroup_;
    bool globalThermalTSgeneration_;
    int scenarioLength_;
    int scenarioNumber_;

    OptimizationParameters par;
    OptimizationVariables vars;
    OptimizationResults scenarioResults;

    // MPSolver instance
    MPSolver solver;
    double solverInfinity;
    Solver::Progression::Task& progression_;
    /* ===================END-CLASS-VARIABLES=================== */

public:
    explicit OptimizedThermalGenerator(Data::Study& study,
                                       Data::MaintenanceGroup& maintenanceGroup,
                                       uint year,
                                       bool globalThermalTSgeneration,
                                       Solver::Progression::Task& progr,
                                       IResultWriter& writer) :
     GeneratorTempData(study, progr, writer),
     maintenanceGroup_(maintenanceGroup),
     progression_(progr),
     par(study, maintenanceGroup, globalThermalTSgeneration, vars, scenarioResults, progr, writer),
     solver(MPSolver("MaintenancePlanning", MPSolver::CBC_MIXED_INTEGER_PROGRAMMING))
    {
        currentYear = year;
        globalThermalTSgeneration_ = globalThermalTSgeneration;
        scenarioLength_ = study.parameters.maintenancePlanning.getScenarioLength();
        scenarioNumber_ = study.parameters.maintenancePlanning.getScenarioNumber();
        nbThermalTimeseries = scenarioLength_ * scenarioNumber_;
        par.scenarioLength_ = scenarioLength_;

        // Solver Settings
        // MP solver parameters / TODD CR27: do we change this -
        // I would keep it on default values for the time being

        // Access solver parameters
        MPSolverParameters params;
        // Set parameter values
        // params.SetIntegerParam(MPSolverParameters::SCALING, 0);
        // params.SetIntegerParam(MPSolverParameters::PRESOLVE, 0);

        // set solver infinity
        solverInfinity = solver.infinity();
    }

    ~OptimizedThermalGenerator() = default;

    // Main functions - loop per scenarios and
    // through the scenario length step by step
    // (moving window)
    void GenerateOptimizedThermalTimeSeries(Data::Study &study);

    void printResidualLoad(Data::Study& study)
    {
        YString path = study.folderInput;
        String buffer;
        buffer.clear() << path << SEP << "MaintenanceData/residual_load.txt";
        Matrix<double> residualLoad;
        residualLoad.clear();
        residualLoad.reset(1U, DAYS_PER_YEAR);

        // File doesn't exist, so create a new one

        // Check if the file is opened successfully

        // Write data to the file
        for (uint i = 0; i < DAYS_PER_YEAR; ++i)
        {
            residualLoad[0][i] = this->par.getResidualLoad(i);
        }

        residualLoad.saveToCSVFile(buffer, 4);
    }

    void printDaySinceLastMaintenance(Data::Study& study)
    {
        YString path = study.folderInput;
        String buffer;
        buffer.clear() << path << SEP << "MaintenanceData/days_since_last_maintenance.txt";
        Matrix<uint> days_since_last_maintenace;
        days_since_last_maintenace.clear();
        // days_since_last_maintenace.reset(1U, );

        // File doesn't exist, so create a new one

        std::vector<uint> days_since_last_maintenace_vector = {};
        uint numberOfAllUnits = 0;
        for (auto& clusterEntry : this->par.clusterData)
        {
            auto& cluster = *(clusterEntry.first);
            if (!(cluster.doWeGenerateTS(globalThermalTSgeneration_)
                  && cluster.optimizeMaintenance))
                continue;
            for (int unit = 0; unit < cluster.unitCount; ++unit)
            {
                int value = this->par.getDaysSinceLastMaintenance(cluster, unit);
                ++numberOfAllUnits;
                days_since_last_maintenace_vector.push_back(value);
            }
        }

        days_since_last_maintenace.reset(1U, numberOfAllUnits);

        for (uint i = 0; i < numberOfAllUnits; ++i)
        {
            days_since_last_maintenace[0][i] = days_since_last_maintenace_vector[i];
        }

        days_since_last_maintenace.saveToCSVFile(buffer);
    }

    void printAverageMaintenanceDuration(Data::Study& study)
    {
        YString path = study.folderInput;
        String buffer;
        buffer.clear() << path << SEP << "MaintenanceData/average_maintenace_duration.txt";
        Matrix<uint> average_maintenace_duration;
        average_maintenace_duration.clear();
        // days_since_last_maintenace.reset(1U, );

        // File doesn't exist, so create a new one

        std::vector<uint> average_maintenace_duration_vector = {};
        uint numberOfAllUnits = 0;
        for (auto& clusterEntry : this->par.clusterData)
        {
            auto& cluster = *(clusterEntry.first);
            if (!(cluster.doWeGenerateTS(globalThermalTSgeneration_)
                  && cluster.optimizeMaintenance))
                continue;
            for (int unit = 0; unit < cluster.unitCount; ++unit)
            {
                int value = this->par.getAverageMaintenanceDuration(cluster);
                ++numberOfAllUnits;
                average_maintenace_duration_vector.push_back(value);
            }
        }

        average_maintenace_duration.reset(1U, numberOfAllUnits);

        for (uint i = 0; i < numberOfAllUnits; ++i)
        {
            average_maintenace_duration[0][i] = average_maintenace_duration_vector[i];
        }

        average_maintenace_duration.saveToCSVFile(buffer);
    }

    void printMaintenanceSchedule()
    {
        uint numberOfUnits = vars.clusterUnits.size();
        for (uint numUnits = 0; numUnits < numberOfUnits; ++numUnits)
        {
            uint numberOfMaintenances = vars.clusterUnits[numUnits].maintenances.size();
            for (uint numMaintenance = 0; numMaintenance < numberOfMaintenances; ++numMaintenance)
            {
                Matrix<uint> data;
                data.reset(2, this->par.timeHorizonFirstStep_);
                for (uint i = 0; i < this->par.timeHorizonFirstStep_; ++i)
                {
                    data[0][i] = vars.clusterUnits[numUnits]
                                   .maintenances[numMaintenance]
                                   .start[i]
                                   ->solution_value();
                    data[1][i] = vars.clusterUnits[numUnits]
                                   .maintenances[numMaintenance]
                                   .end[i]
                                   ->solution_value();
                }
                YString path = study.folderInput;
                String buffer;
                buffer.clear() << path << SEP << "MaintenanceSchedule" << SEP
                               << "Maintenance" + std::to_string(numMaintenance + 1) + "-Unit"
                                    + std::to_string(numUnits + 1) + ".txt";
                data.saveToCSVFile(buffer);
            }
        }
    }
    
    void setRes()
    {
        String buffer = "/home/nikola/Documents/Specialized Softwares in "
                        "PSE/MaintenancePlanning/input/data/res.txt";
        Matrix<double> res;
        res.clear();
        res.reset(1U, par.timeHorizon_);
        Matrix<>::BufferType dataBuffer;

        res.loadFromCSVFile(buffer, 1, par.timeHorizon_, &dataBuffer);

        for (uint i = 0; i < par.timeHorizon_; ++i)
        {
            par.residualLoadDailyValues_[i] = res[0][i];
        }
    }
};

} // namespace Antares::Solver::TSGenerator