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
#ifndef __ANTARES_TOOLBOX_COMPONENT_HTMLLISTBOX_ITEM_HYDROCLUSTER_CLUSTER_H__
#define __ANTARES_TOOLBOX_COMPONENT_HTMLLISTBOX_ITEM_HYDROCLUSTER_CLUSTER_H__

#include "cluster-item.h"

#include <memory>

namespace Antares
{
namespace Component
{
namespace HTMLListbox
{
namespace Item
{
/*!
** \brief Single item for a renewable cluster.
**
** See parent classes for more explanations
*/
class HydroclusterClusterItem : public ClusterItem
{
public:
    using Ptr = std::shared_ptr<HydroclusterClusterItem>;

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default Constructor
    */
    HydroclusterClusterItem(Antares::Data::HydroclusterCluster* a);
    /*!
    ** \brief additional Additional HTML content ("<td>my text</td>")
    */
    HydroclusterClusterItem(Antares::Data::HydroclusterCluster* a, const wxString& additional);
    //! Destructor
    virtual ~HydroclusterClusterItem();
    //@}

    //! Get the attached cluster
    Antares::Data::HydroclusterCluster* hydroclusterAggregate() const;

    void addAdditionalIcons(wxString& out) const override;

private:
    wxString htmlContentTail() override;

private:
    //! The current HydroclusterCluster
    Antares::Data::HydroclusterCluster* pHydroclusterCluster;

}; // class HydroclusterClusterItem

} // namespace Item
} // namespace HTMLListbox
} // namespace Component
} // namespace Antares

#endif // __ANTARES_TOOLBOX_COMPONENT_HTMLLISTBOX_ITEM_HYDROCLUSTER_CLUSTER_H__