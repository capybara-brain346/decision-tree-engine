from abc import ABC, abstractmethod
from typing import Callable, Optional


class Node(ABC):
    @abstractmethod
    def evaluate(self, context):
        pass


class DecisionNode(Node):
    def __init__(
        self,
        name,
        condition: Callable,
        true_node: Optional[Node] = None,
        false_node: Optional[Node] = None,
    ) -> None:
        self.name = name
        self.condition = condition
        self.true_node = true_node
        self.false_node = false_node

    def evaluate(self, context):
        result = self.condition(context)

        if result and self.true_node:
            return self.true_node.evaluate(context)
        elif not result and self.false_node:
            return self.false_node.evaluate(context)
        else:
            return None


class OutcomeNode(Node):
    def __init__(self, value, action: Optional[Callable] = None) -> None:
        self.value = value
        self.action = action

    def evaluate(self, context):
        if self.action:
            self.action(context)

        return self.value


class MultiBranchNode(Node):
    def __init__(self, name):
        self.name = name
        self.branches = []
        self.default_node = None

    def add_branch(self, condition, node):
        self.branches.append((condition, node))
        return self

    def set_default(self, node):
        self.default_node = node
        return self

    def evaluate(self, context):
        for condition, node in self.branches:
            if condition(context):
                return node.evaluate(context)

        if self.default_node:
            return self.default_node.evaluate(context)

        return None


class DecisionTreeEngine:
    def __init__(self, root: Node) -> None:
        self.root = root
        self.trace = []

    def evaluate(self, context, trace):
        if trace:
            self.trace = []

        result = self.root.evaluate(context)
        return result

    def get_trace(self):
        return self.trace


if __name__ == "__main__":
    approved = OutcomeNode(
        "APPROVED", lambda ctx: print(f"Loan approved for ${ctx.get('amount')}")
    )
    denied_income = OutcomeNode("DENIED - Insufficient Income")
    denied_credit = OutcomeNode("DENIED - Low Credit Score")
    manual_review = OutcomeNode("MANUAL REVIEW REQUIRED")

    credit_check = DecisionNode(
        name="Credit Score Check",
        condition=lambda ctx: ctx.get("credit_score", 0) >= 650,
        true_node=approved,
        false_node=denied_credit,
    )

    income_check = DecisionNode(
        name="Income Check",
        condition=lambda ctx: ctx.get("income", 0) >= 50000,
        true_node=credit_check,
        false_node=denied_income,
    )

    amount_check = DecisionNode(
        name="Loan Amount Check",
        condition=lambda ctx: ctx.get("amount", 0) <= 100000,
        true_node=income_check,
        false_node=manual_review,
    )

    engine = DecisionTreeEngine(amount_check)

    test_cases = [
        {"amount": 50000, "income": 75000, "credit_score": 700},
        {"amount": 50000, "income": 40000, "credit_score": 700},
        {"amount": 50000, "income": 75000, "credit_score": 600},
        {"amount": 150000, "income": 75000, "credit_score": 700},
    ]

    print("=== Loan Approval Decision Tree ===\n")
    for i, case in enumerate(test_cases, 1):
        print(f"Test Case {i}: {case}")
        result = engine.evaluate(case, trace=None)
        print(f"Result: {result}\n")

    print("\n=== Risk Assessment (Multi-Branch) ===\n")

    low_risk = OutcomeNode("LOW RISK")
    medium_risk = OutcomeNode("MEDIUM RISK")
    high_risk = OutcomeNode("HIGH RISK")
    critical_risk = OutcomeNode("CRITICAL RISK")

    risk_assessment = MultiBranchNode("Risk Level")
    risk_assessment.add_branch(
        lambda ctx: ctx.get("credit_score", 0) >= 750
        and ctx.get("debt_ratio", 1) < 0.3,
        low_risk,
    ).add_branch(
        lambda ctx: ctx.get("credit_score", 0) >= 650
        and ctx.get("debt_ratio", 1) < 0.5,
        medium_risk,
    ).add_branch(lambda ctx: ctx.get("credit_score", 0) >= 550, high_risk).set_default(
        critical_risk
    )

    risk_engine = DecisionTreeEngine(risk_assessment)

    risk_cases = [
        {"credit_score": 780, "debt_ratio": 0.25},
        {"credit_score": 680, "debt_ratio": 0.4},
        {"credit_score": 600, "debt_ratio": 0.6},
        {"credit_score": 500, "debt_ratio": 0.8},
    ]

    for case in risk_cases:
        print(f"Case: {case}")
        result = risk_engine.evaluate(case, trace=[])
        print(f"Risk: {result}\n")
