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

#include "../../study.h"
#include "containerhydrocluster.h"
#include "../../../inifile.h"
#include "../../../array/array1d.h"

using namespace Antares;
using namespace Yuni;

#define SEP IO::Separator

namespace Antares
{
namespace Data
{
 
PartHydrocluster::PartHydrocluster()
{
}

bool PartHydrocluster::invalidate(bool reload) const
{
    bool ret = true;
    ret = list.invalidate(reload) && ret;
    return ret;
}

void PartHydrocluster::markAsModified() const
{
    list.markAsModified();
}

void PartHydrocluster::estimateMemoryUsage(StudyMemoryUsage& u) const
{
    u.requiredMemoryForInput += sizeof(PartHydrocluster);
    list.estimateMemoryUsage(u);
}

PartHydrocluster::~PartHydrocluster()
{
}

void PartHydrocluster::prepareAreaWideIndexes()
{
    // Copy the list with all Hydrocluster clusters
    // And init the areaWideIndex (unique index for a given area)
    if (list.empty())
    {
        clusters.clear();
        return;
    }

    clusters = std::vector<HydroclusterCluster*>(list.size());

    auto end = list.end();
    uint idx = 0;
    for (auto i = list.begin(); i != end; ++i)
    {
        HydroclusterCluster* t = i->second.get();
        t->areaWideIndex = idx;
        clusters[idx] = t;
        ++idx;
    }
}

uint PartHydrocluster::removeDisabledClusters()
{
    // nothing to do if there is no cluster available
    if (list.empty())
        return 0;

    std::vector<ClusterName> disabledClusters;

    for (auto& it : list)
    {
        if (!it.second->enabled)
            disabledClusters.push_back(it.first);
    }

    for (auto& cluster : disabledClusters)
        list.remove(cluster);

    const auto count = disabledClusters.size();
    if (count)
        list.rebuildIndex();

    return count;
}

void PartHydrocluster::reset()
{
    list.clear();
    clusters.clear();
}


} // namespace Data
} // namespace Antares