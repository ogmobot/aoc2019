#!/usr/bin/env python3
from math import ceil

ONE_TRILLION = int(1e12)

class Mat:
    def __init__(self, a, b=None) -> None:
        if b == None:
            parts = a.split()
            self.qty =  int(parts[0])
            self.name = parts[1]
        else:
            self.qty = a
            self.name = b
        return

    def __hash__(self) -> int:
        return hash((self.qty, self.name))

    def __eq__(self, other) -> bool:
        return self.qty == other.qty and self.name == other.name

    def __repr__(self) -> str:
        return f"Mat({self.qty}, {self.name})"

def load_reactions(filename) -> dict[Mat, list[Mat]]:
    result: dict[str, tuple[list[Mat], int]] = dict()
    with open(filename, "r") as fp:
        for line in fp:
            reqs, product = line.split(" => ")
            matproduct = Mat(product)
            result[matproduct.name] = (
                [Mat(req) for req in reqs.split(", ")],
                matproduct.qty
            )
        return result

def do_reaction(
    reactions: dict[str, tuple[list[Mat], int]],
    product: Mat,
    stockpile: dict[str, int]
) -> int:
    #print(f"making {product}")
    if product.name == "ORE":
        stockpile["ORE"] = stockpile.get("ORE", 0) + product.qty
        return product.qty
    ore_used: int = 0
    batches_required: int = ceil(product.qty / reactions[product.name][1])
    requirement_list: list[Mat] = [
        Mat(batches_required * req.qty, req.name)
        for req in reactions[product.name][0]
    ]
    while requirement_list:
        requirement: Mat = requirement_list.pop()
        if requirement.name == "ORE":
            ore_used += requirement.qty
        else:
            if stockpile.get(requirement.name, 0) < requirement.qty:
                amount_required: int = (requirement.qty
                                    - stockpile.get(requirement.name, 0))
                ore_used += do_reaction(
                                reactions,
                                Mat(amount_required, requirement.name),
                                stockpile)
            stockpile[requirement.name] -= requirement.qty
    stockpile[product.name] = (stockpile.get(product.name, 0)
                            + (batches_required * reactions[product.name][1]))
    return ore_used

def main() -> None:
    reactions = load_reactions("input14.txt")
    # Part 1
    ore_used: int = do_reaction(reactions, Mat(1, "FUEL"), dict())
    print(ore_used)
    # Part 2
    lower: int = ONE_TRILLION // ore_used
    upper: int = 2 * lower # arbitrary
    while upper - lower > 100:
        guess: int = (upper + lower) // 2
        ore_used = do_reaction(reactions, Mat(guess, "FUEL"), dict())
        if ore_used > ONE_TRILLION:
            upper = guess
        else:
            lower = guess
    # Switch to linear search
    guess = upper
    ore_used = do_reaction(reactions, Mat(guess, "FUEL"), dict())
    while ore_used > ONE_TRILLION:
        guess -= 1
        ore_used = do_reaction(reactions, Mat(guess, "FUEL"), dict())
    print(guess)

if __name__ == "__main__":
    main()
