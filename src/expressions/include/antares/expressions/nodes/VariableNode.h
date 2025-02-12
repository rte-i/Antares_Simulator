#pragma once

#include <string>

#include <antares/expressions/nodes/Leaf.h>
#include "antares/expressions/visitors/TimeIndex.h"

namespace Antares::Expressions::Visitors
{
enum class TimeIndex : unsigned int;
}

namespace Antares::Expressions::Nodes
{

/**
 * @brief Represents a variable node in a syntax tree, storing a string value.
 */
class VariableNode final: public Leaf<std::string>
{
public:
    explicit VariableNode(
      const std::string& value,
      Visitors::TimeIndex time_index = Visitors::TimeIndex::VARYING_IN_TIME_AND_SCENARIO):
        Leaf<std::string>(value),
        time_index_(time_index)
    {
    }

    std::string name() const override
    {
        return "VariableNode";
    }

    Visitors::TimeIndex timeIndex() const
    {
        return time_index_;
    }

private:
    Visitors::TimeIndex time_index_;
};
} // namespace Antares::Expressions::Nodes
