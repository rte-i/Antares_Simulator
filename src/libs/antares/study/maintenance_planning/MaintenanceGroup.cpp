#include "MaintenanceGroup.h"
#include <yuni/yuni.h>
#include <yuni/core/math.h>
#include <algorithm>
#include <vector>
#include "../study.h"

using namespace Yuni;
using namespace Antares;

#define SEP IO::Separator

namespace Antares::Data
{

const char* MaintenanceGroup::ResidualLoadDefinitionTypeToCString(
  MaintenanceGroup::ResidualLoadDefinitionType type)
{
    static const char* const names[typeMax + 1] = {"", "weights", "timeserie", ""};
    assert((uint)type < (uint)(typeMax + 1));
    return names[type];
}

MaintenanceGroup::ResidualLoadDefinitionType MaintenanceGroup::StringToResidualLoadDefinitionType(
  const AnyString& text)
{
    ShortString16 l(text);
    l.toLower();

    if (l == "weights")
        return typeWeights;
    if (l == "timeserie")
        return typeTimeserie;
    return typeUnknown;
}

MaintenanceGroup::~MaintenanceGroup()
{
#ifndef NDEBUG
    name_ = "<INVALID>";
    ID_ = "<INVALID>";
#endif
}

void MaintenanceGroup::name(const std::string& newName)
{
    name_ = std::move(newName);
    ID_.clear();
    Antares::TransformNameIntoID(name_, ID_);
}

void MaintenanceGroup::loadWeight(const Area* area, double w)
{
    if (area)
        weights_[area].load = w;
}

void MaintenanceGroup::renewableWeight(const Area* area, double w)
{
    if (area)
        weights_[area].renewable = w;
}

void MaintenanceGroup::rorWeight(const Area* area, double w)
{
    if (area)
        weights_[area].ror = w;
}

void MaintenanceGroup::removeAllWeights()
{
    weights_.clear();
}

void MaintenanceGroup::resetToDefaultValues()
{
    enabled_ = true;
    removeAllWeights();
}

void MaintenanceGroup::clear()
{
    // Name / ID
    this->name_.clear();
    this->ID_.clear();
    this->type_ = typeUnknown;
    this->enabled_ = true;
}

bool MaintenanceGroup::contains(const MaintenanceGroup* mnt) const
{
    return (this == mnt);
}

bool MaintenanceGroup::contains(const Area* area) const
{
    const auto i = weights_.find(area);
    return (i != weights_.end());
}

uint64_t MaintenanceGroup::memoryUsage() const
{
    return sizeof(MaintenanceGroup)
           // Estimation
           + weights_.size() * (sizeof(double) * 3 + 3 * sizeof(void*) * 3);
}

void MaintenanceGroup::enabled(bool v)
{
    enabled_ = v;
}

void MaintenanceGroup::setResidualLoadDefinitionType(MaintenanceGroup::ResidualLoadDefinitionType t)
{
    type_ = t;
}

// TODO CR27: not implemented for now - used only for UI - maybe to added later
bool MaintenanceGroup::hasAllWeightedClustersOnLayer(size_t layerID)
{
    return true;
}
// TODO CR27: not implemented for now - used only for UI - maybe to added later
void MaintenanceGroup::copyWeights()
{
}

double MaintenanceGroup::loadWeight(const Area* area) const
{
    auto i = weights_.find(area);
    return (i != weights_.end()) ? i->second.load : 0.;
}

double MaintenanceGroup::renewableWeight(const Area* area) const
{
    auto i = weights_.find(area);
    return (i != weights_.end()) ? i->second.renewable : 0.;
}

double MaintenanceGroup::rorWeight(const Area* area) const
{
    auto i = weights_.find(area);
    return (i != weights_.end()) ? i->second.ror : 0.;
}

void MaintenanceGroup::setUsedResidualLoadTS(std::array<double, 8760> ts)
{
    usedResidualLoadTS_ = ts;
}

std::array<double, 8760> MaintenanceGroup::getUsedResidualLoadTS() const
{
    return usedResidualLoadTS_;
}

void MaintenanceGroup::clearAndReset(const MaintenanceGroupName& name,
                                     MaintenanceGroup::ResidualLoadDefinitionType newType)
{
    // Name / ID
    name_ = std::move(name);
    ID_.clear();
    TransformNameIntoID(name_, ID_);
    // New type
    type_ = newType;
    // Resetting the weights
    removeAllWeights();
}

void MaintenanceGroup::copyFrom(MaintenanceGroup const* original)
{
    clearAndReset(original->name(), original->type());
    weights_ = original->weights_;
    enabled_ = original->enabled_;
}

} // namespace Antares::Data