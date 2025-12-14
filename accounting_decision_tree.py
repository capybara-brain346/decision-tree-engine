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
