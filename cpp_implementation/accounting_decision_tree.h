#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <variant>
#include <any>

using Context = std::map<std::string, std::any>;

using Result = std::variant<std::string, int, double, bool>;

template<typename T>
T getContextValue(const Context& ctx, const std::string& key, T defaultValue) {
    auto it = ctx.find(key);
    if (it != ctx.end()) {
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

class Node;

using NodePtr = std::shared_ptr<Node>;
using Condition = std::function<bool(const Context&)>;
using Action = std::function<void(const Context&)>;

class Node {
public:
    virtual ~Node() = default;
    virtual Result evaluate(const Context& context) const = 0;
    virtual std::string getType() const = 0;
    virtual std::string toJson(int indent = 0) const = 0;
};

class OutcomeNode : public Node {
private:
    Result value_;
    Action action_;

public:
    OutcomeNode(Result value, Action action = nullptr);

    Result evaluate(const Context& context) const override;
    std::string getType() const override;
    std::string toJson(int indent = 0) const override;
};

class DecisionNode : public Node {
private:
    std::string name_;
    Condition condition_;
    NodePtr trueNode_;
    NodePtr falseNode_;

public:
    DecisionNode(const std::string& name,
                 Condition condition,
                 NodePtr trueNode = nullptr,
                 NodePtr falseNode = nullptr);

    Result evaluate(const Context& context) const override;
    std::string getType() const override;
    std::string toJson(int indent = 0) const override;

    void setTrueNode(NodePtr node);
    void setFalseNode(NodePtr node);
};

class MultiBranchNode : public Node {
private:
    std::string name_;
    std::vector<std::pair<Condition, NodePtr>> branches_;
    NodePtr defaultNode_;

public:
    explicit MultiBranchNode(const std::string& name);

    MultiBranchNode& addBranch(Condition condition, NodePtr node);
    MultiBranchNode& setDefault(NodePtr node);

    Result evaluate(const Context& context) const override;
    std::string getType() const override;
    std::string toJson(int indent = 0) const override;
};

class DecisionTreeEngine {
private:
    NodePtr root_;
    mutable std::vector<std::string> trace_;

public:
    explicit DecisionTreeEngine(NodePtr root);

    Result evaluate(const Context& context, bool enableTrace = false);
    const std::vector<std::string>& getTrace() const;
    void printTree() const;
};

std::string resultToString(const Result& result);

void loanApprovalExample();
void riskAssessmentExample();
