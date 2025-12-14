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
    def __init__(self, value, action: Optional[Callable]) -> None:
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

    def set_branch(self, node):
        self.default_node = node
        return self

    def evaluate(self, context):
        for condition, node in self.branches:
            if condition(context):
                return node.evaluate(context)
        
        if self.default_node:
            return self.default_node.evaluate(context)
        
        return None
