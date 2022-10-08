#!/usr/bin/env pypy3

S_ALIVE = '#'

def load_universe_2d(lines: list) -> set:
    result = set()
    for row, line in enumerate(lines):
        for col, symbol in enumerate(line):
            if symbol == S_ALIVE:
                result.add((row, col))
    return result

def load_universe_3d(lines: list) -> set:
    # Well, kind of 3d.
    result = set()
    for row, line in enumerate(lines):
        for col, symbol in enumerate(line):
            if symbol == S_ALIVE:
                result.add((row, col, 0))
    return result

def universe_value(universe: set) -> int:
    total = 0
    for row in range(5):
        for col in range(5):
            total += (int((row, col) in universe) << ((5 * row) + col))
    return total

def count_adjacent_2d(universe: set, row: int, col: int) -> int:
    return sum(
        (row + dr, col + dc) in universe
        for dr, dc in ((-1, 0), (1, 0), (0, -1), (0, 1))
    )

def count_adjacent_3d(universe: set, row: int, col: int, layer: int) -> int:
    # This could probably be refactored to look a lot nicer
    total = 0
    # above
    if row == 0:
        total += int((1, 2, layer - 1) in universe)
    elif row == 3 and col == 2:
        total += sum((4, c, layer + 1) in universe for c in range(5))
    else:
        total += int((row - 1, col, layer) in universe)
    # below
    if row == 4:
        total += int((3, 2, layer - 1) in universe)
    elif row == 1 and col == 2:
        total += sum((0, c, layer + 1) in universe for c in range(5))
    else:
        total += int((row + 1, col, layer) in universe)
    # left
    if col == 0:
        total += int((2, 1, layer - 1) in universe)
    elif col == 3 and row == 2:
        total += sum((r, 4, layer + 1) in universe for r in range(5))
    else:
        total += int((row, col - 1, layer) in universe)
    # right
    if col == 4:
        total += int((2, 3, layer - 1) in universe)
    elif col == 1 and row == 2:
        total += sum((r, 0, layer + 1) in universe for r in range(5))
    else:
        total += int((row, col + 1, layer) in universe)
    return total

def update_universe_2d(universe: set) -> set:
    result = set()
    for row in range(5):
        for col in range(5):
            n_adj = count_adjacent_2d(universe, row, col)
            if (row, col) in universe:
                if n_adj == 1:
                    result.add((row, col))
            else:
                if n_adj == 1 or n_adj == 2:
                    result.add((row, col))
    return result

def update_universe_3d(universe: set) -> set:
    result = set()
    lowest_layer  = min(coord[2] for coord in universe)
    highest_layer = max(coord[2] for coord in universe)
    for row in range(5):
        for col in range(5):
            if row == 2 and col == 2:
                continue
            for layer in range(lowest_layer - 1, highest_layer + 2):
                n_adj = count_adjacent_3d(universe, row, col, layer)
                #print(f"{(row, col, layer)} has {n_adj} neighoburs")
                if (row, col, layer) in universe:
                    if n_adj == 1:
                        result.add((row, col, layer))
                else:
                    if n_adj == 1 or n_adj == 2:
                        result.add((row, col, layer))
    return result

def display(universe: set) -> None:
    if len(list(universe)[0]) == 2:
        for row in range(5):
            print(" ".join([
                {True: "#", False: "."}.get(
                    (row, col) in universe)
                for col in range(5)
            ]))
    else:
        lowest_layer  = min(coord[2] for coord in universe)
        highest_layer = max(coord[2] for coord in universe)
        for layer in range(lowest_layer, highest_layer + 1):
            print(f"Layer {layer}:")
            for row in range(5):
                print(" ".join([
                    {True: "#", False: "."}.get(
                        (row, col, layer) in universe)
                    for col in range(5)
                ]))

def main():
    with open("input24.txt", "r") as f:
        lines = f.readlines()

    universe_p1 = load_universe_2d(lines)
    seen_values = set()
    val = universe_value(universe_p1)
    while val not in seen_values:
        seen_values.add(val)
        universe_p1 = update_universe_2d(universe_p1)
        val = universe_value(universe_p1)
    print(val)

    universe_p2 = load_universe_3d(lines)
    for _ in range(200):
        universe_p2 = update_universe_3d(universe_p2)
    print(len(universe_p2))

if __name__ == "__main__":
    main()
