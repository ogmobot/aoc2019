#!/usr/bin/env pypy3

def load_universe(fp) -> set:
    result = set()
    for row, line in enumerate(fp):
        for col, symbol in enumerate(line):
            if symbol == "#":
                result.add((row, col, 0))
    return result

def universe_value(universe: set) -> int:
    total = 0
    for row in range(5):
        for col in range(5):
            total += (int((row, col, 0) in universe) << ((5 * row) + col))
    return total

def count_adjacent_2d(universe: set, row: int, col: int) -> int:
    return sum(
        (row + dr, col + dc, 0) in universe
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

def update_universe(universe: set, layered: bool) -> set:
    result = set()
    if layered:
        lowest_layer  = min(coord[2] for coord in universe)
        highest_layer = max(coord[2] for coord in universe)
        all_layers = [l for l in range(lowest_layer - 1, highest_layer + 2)]
    else:
        all_layers = (0,)
    for row in range(5):
        for col in range(5):
            if layered and row == 2 and col == 2:
                continue
            for layer in all_layers:
                if layered:
                    n_adj = count_adjacent_3d(universe, row, col, layer)
                else:
                    n_adj = count_adjacent_2d(universe, row, col)
                if (row, col, layer) in universe:
                    if n_adj == 1:
                        result.add((row, col, layer))
                else:
                    if n_adj == 1 or n_adj == 2:
                        result.add((row, col, layer))
    return result

def main():
    with open("input24.txt", "r") as f:
        orig = load_universe(f)

    # Part 1
    u = orig.copy()
    seen_values = set()
    val = universe_value(u)
    while val not in seen_values:
        seen_values.add(val)
        u = update_universe(u, False)
        val = universe_value(u)
    print(val)

    # Part 2
    u = orig.copy()
    for _ in range(200):
        u = update_universe(u, True)
    print(len(u))

if __name__ == "__main__":
    main()
