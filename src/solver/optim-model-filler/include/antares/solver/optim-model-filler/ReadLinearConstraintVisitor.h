/*
 * Copyright 2007-2024, RTE (https://www.rte-france.com)
 * See AUTHORS.txt
 * SPDX-License-Identifier: MPL-2.0
 * This file is part of Antares-Simulator,
 * Adequacy and Performance assessment for interconnected energy networks.
 *
 * Antares_Simulator is free software: you can redistribute it and/or modify
 * it under the terms of the Mozilla Public Licence 2.0 as published by
 * the Mozilla Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Antares_Simulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Mozilla Public Licence 2.0 for more details.
 *
 * You should have received a copy of the Mozilla Public Licence 2.0
 * along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
 */

#pragma once

#include <limits>

#include <antares/expressions/visitors/EvaluationContext.h>
#include <antares/expressions/visitors/NodeVisitor.h>

#include "ReadLinearExpressionVisitor.h"

/**
 * Read Linear Constraint Visitor
 * Visits a Node and produces a Linear Constraint (defined by its Linear Expression and bounds).
 * The root node is expected to be a comparison node.
 */
namespace Antares::Optimization
{

/**
 * Linear Constraint
 * Represents a linear constraint in an optimization problem.
 * It is fully defined by:
 * - a Linear Expression (defined by an offset and non-zero variable coefficients)
 * - a lower and an upper bounds
 */
struct LinearConstraint
{
    std::map<std::string, double> coef_per_var;
    double lb = -std::numeric_limits<double>::infinity();
    double ub = std::numeric_limits<double>::infinity();
};

class ReadLinearConstraintVisitor: public Expressions::Visitors::NodeVisitor<LinearConstraint>
{
public:
    ReadLinearConstraintVisitor() = default;
    explicit ReadLinearConstraintVisitor(Expressions::Visitors::EvaluationContext context);
    std::string name() const override;

private:
    ReadLinearExpressionVisitor linear_expression_visitor_;
    LinearConstraint visit(const Expressions::Nodes::SumNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::SubtractionNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::MultiplicationNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::DivisionNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::EqualNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::LessThanOrEqualNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::GreaterThanOrEqualNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::NegationNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::VariableNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::ParameterNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::LiteralNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::PortFieldNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::PortFieldSumNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::ComponentVariableNode* node) override;
    LinearConstraint visit(const Expressions::Nodes::ComponentParameterNode* node) override;
};
} // namespace Antares::Optimization
