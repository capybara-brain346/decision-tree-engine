#include "accounting_decision_tree.h"

OutcomeNode::OutcomeNode(Result value, Action action)
    : value_(value), action_(action) {}

Result OutcomeNode::evaluate(const Context& context) const {
    if (action_) {
        action_(context);
    }
    return value_;
}

std::string OutcomeNode::getType() const {
    return "OutcomeNode";
}

std::string OutcomeNode::toJson(int indent) const {
    std::string indentStr(indent, ' ');
    std::string nextIndentStr(indent + 2, ' ');

    std::string json = indentStr + "{\n";
    json += nextIndentStr + "\"type\": \"outcome\",\n";
    json += nextIndentStr + "\"value\": \"" + resultToString(value_) + "\",\n";
    json += nextIndentStr + "\"hasAction\": " + (action_ ? "true" : "false") + "\n";
    json += indentStr + "}";

    return json;
}

DecisionNode::DecisionNode(const std::string& name,
                           Condition condition,
                           NodePtr trueNode,
                           NodePtr falseNode)
    : name_(name), condition_(condition),
      trueNode_(trueNode), falseNode_(falseNode) {}

Result DecisionNode::evaluate(const Context& context) const {
    bool result = condition_(context);

    if (result && trueNode_) {
        return trueNode_->evaluate(context);
    } else if (!result && falseNode_) {
        return falseNode_->evaluate(context);
    }

    return std::string("NO_RESULT");
}

std::string DecisionNode::getType() const {
    return "DecisionNode: " + name_;
}

void DecisionNode::setTrueNode(NodePtr node) {
    trueNode_ = node;
}

void DecisionNode::setFalseNode(NodePtr node) {
    falseNode_ = node;
}

std::string DecisionNode::toJson(int indent) const {
    std::string indentStr(indent, ' ');
    std::string nextIndentStr(indent + 2, ' ');

    std::string json = indentStr + "{\n";
    json += nextIndentStr + "\"type\": \"decision\",\n";
    json += nextIndentStr + "\"name\": \"" + name_ + "\",\n";

    if (trueNode_) {
        json += nextIndentStr + "\"trueBranch\": \n";
        json += trueNode_->toJson(indent + 2);
        json += ",\n";
    }

    if (falseNode_) {
        json += nextIndentStr + "\"falseBranch\": \n";
        json += falseNode_->toJson(indent + 2);
        json += "\n";
    }

    json += indentStr + "}";
    return json;
}

MultiBranchNode::MultiBranchNode(const std::string& name)
    : name_(name), defaultNode_(nullptr) {}

MultiBranchNode& MultiBranchNode::addBranch(Condition condition, NodePtr node) {
    branches_.emplace_back(condition, node);
    return *this;
}

MultiBranchNode& MultiBranchNode::setDefault(NodePtr node) {
    defaultNode_ = node;
    return *this;
}

Result MultiBranchNode::evaluate(const Context& context) const {
    for (const auto& [condition, node] : branches_) {
        if (condition(context)) {
            return node->evaluate(context);
        }
    }

    if (defaultNode_) {
        return defaultNode_->evaluate(context);
    }

    return std::string("NO_MATCH");
}

std::string MultiBranchNode::getType() const {
    return "MultiBranchNode: " + name_;
}

std::string MultiBranchNode::toJson(int indent) const {
    std::string indentStr(indent, ' ');
    std::string nextIndentStr(indent + 2, ' ');
    std::string arrayIndentStr(indent + 4, ' ');

    std::string json = indentStr + "{\n";
    json += nextIndentStr + "\"type\": \"multibranch\",\n";
    json += nextIndentStr + "\"name\": \"" + name_ + "\",\n";
    json += nextIndentStr + "\"branches\": [\n";

    for (size_t i = 0; i < branches_.size(); ++i) {
        json += arrayIndentStr + "{\n";
        json += arrayIndentStr + "  \"condition\": \"branch_" + std::to_string(i) + "\",\n";
        json += arrayIndentStr + "  \"node\": \n";
        json += branches_[i].second->toJson(indent + 6);
        json += "\n" + arrayIndentStr + "}";

        if (i < branches_.size() - 1 || defaultNode_) {
            json += ",";
        }
        json += "\n";
    }

    if (defaultNode_) {
        json += arrayIndentStr + "{\n";
        json += arrayIndentStr + "  \"condition\": \"default\",\n";
        json += arrayIndentStr + "  \"node\": \n";
        json += defaultNode_->toJson(indent + 6);
        json += "\n" + arrayIndentStr + "}\n";
    }

    json += nextIndentStr + "]\n";
    json += indentStr + "}";

    return json;
}

DecisionTreeEngine::DecisionTreeEngine(NodePtr root) : root_(root) {}

Result DecisionTreeEngine::evaluate(const Context& context, bool enableTrace) {
    if (enableTrace) {
        trace_.clear();
    }

    if (!root_) {
        return std::string("NO_ROOT");
    }

    return root_->evaluate(context);
}

const std::vector<std::string>& DecisionTreeEngine::getTrace() const {
    return trace_;
}

void DecisionTreeEngine::printTree() const {
    if (!root_) {
        std::cout << "{ \"error\": \"No root node\" }" << std::endl;
        return;
    }

    std::cout << root_->toJson(0) << std::endl;
}

std::string resultToString(const Result& result) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        }
        return "unknown";
    }, result);
}

void loanApprovalExample() {
    std::cout << "=== Loan Approval Decision Tree ===\n\n";

    auto approved = std::make_shared<OutcomeNode>(
        std::string("APPROVED"),
        [](const Context& ctx) {
            int amount = getContextValue<int>(ctx, "amount", 0);
            std::cout << "  -> Loan approved for $" << amount << "\n";
        }
    );

    auto deniedIncome = std::make_shared<OutcomeNode>(
        std::string("DENIED - Insufficient Income")
    );

    auto deniedCredit = std::make_shared<OutcomeNode>(
        std::string("DENIED - Low Credit Score")
    );

    auto manualReview = std::make_shared<OutcomeNode>(
        std::string("MANUAL REVIEW REQUIRED")
    );

    auto creditCheck = std::make_shared<DecisionNode>(
        "Credit Score Check",
        [](const Context& ctx) {
            return getContextValue<int>(ctx, "credit_score", 0) >= 650;
        },
        approved,
        deniedCredit
    );

    auto incomeCheck = std::make_shared<DecisionNode>(
        "Income Check",
        [](const Context& ctx) {
            return getContextValue<int>(ctx, "income", 0) >= 50000;
        },
        creditCheck,
        deniedIncome
    );

    auto amountCheck = std::make_shared<DecisionNode>(
        "Loan Amount Check",
        [](const Context& ctx) {
            return getContextValue<int>(ctx, "amount", 0) <= 100000;
        },
        incomeCheck,
        manualReview
    );

    DecisionTreeEngine engine(amountCheck);

    std::cout << "\n=== Tree Structure (JSON) ===\n";
    engine.printTree();
    std::cout << "\n";

    std::vector<Context> testCases = {
        {{"amount", 50000}, {"income", 75000}, {"credit_score", 700}},
        {{"amount", 50000}, {"income", 40000}, {"credit_score", 700}},
        {{"amount", 50000}, {"income", 75000}, {"credit_score", 600}},
        {{"amount", 150000}, {"income", 75000}, {"credit_score", 700}}
    };

    std::cout << "\n=== Test Results ===\n";
    int testNum = 1;
    for (const auto& testCase : testCases) {
        std::cout << "Test Case " << testNum++ << ":\n";
        std::cout << "  Amount: " << getContextValue<int>(testCase, "amount", 0)
                  << ", Income: " << getContextValue<int>(testCase, "income", 0)
                  << ", Credit: " << getContextValue<int>(testCase, "credit_score", 0) << "\n";

        Result result = engine.evaluate(testCase);
        std::cout << "  Result: " << resultToString(result) << "\n\n";
    }
}

void riskAssessmentExample() {
    std::cout << "\n=== Risk Assessment (Multi-Branch) ===\n\n";

    auto lowRisk = std::make_shared<OutcomeNode>(std::string("LOW RISK"));
    auto mediumRisk = std::make_shared<OutcomeNode>(std::string("MEDIUM RISK"));
    auto highRisk = std::make_shared<OutcomeNode>(std::string("HIGH RISK"));
    auto criticalRisk = std::make_shared<OutcomeNode>(std::string("CRITICAL RISK"));

    auto riskAssessment = std::make_shared<MultiBranchNode>("Risk Level");

    riskAssessment->addBranch(
        [](const Context& ctx) {
            return getContextValue<int>(ctx, "credit_score", 0) >= 750 &&
                   getContextValue<double>(ctx, "debt_ratio", 1.0) < 0.3;
        },
        lowRisk
    ).addBranch(
        [](const Context& ctx) {
            return getContextValue<int>(ctx, "credit_score", 0) >= 650 &&
                   getContextValue<double>(ctx, "debt_ratio", 1.0) < 0.5;
        },
        mediumRisk
    ).addBranch(
        [](const Context& ctx) {
            return getContextValue<int>(ctx, "credit_score", 0) >= 550;
        },
        highRisk
    ).setDefault(criticalRisk);

    DecisionTreeEngine riskEngine(riskAssessment);

    std::cout << "=== Tree Structure (JSON) ===\n";
    riskEngine.printTree();
    std::cout << "\n";

    std::vector<Context> riskCases = {
        {{"credit_score", 780}, {"debt_ratio", 0.25}},
        {{"credit_score", 680}, {"debt_ratio", 0.4}},
        {{"credit_score", 600}, {"debt_ratio", 0.6}},
        {{"credit_score", 500}, {"debt_ratio", 0.8}}
    };

    std::cout << "=== Test Results ===\n";
    for (const auto& riskCase : riskCases) {
        std::cout << "Case: Credit=" << getContextValue<int>(riskCase, "credit_score", 0)
                  << ", Debt Ratio=" << getContextValue<double>(riskCase, "debt_ratio", 0.0) << "\n";

        Result result = riskEngine.evaluate(riskCase);
        std::cout << "Risk: " << resultToString(result) << "\n\n";
    }
}
