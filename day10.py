#!/usr/bin/env python3
from math import atan2

S_ASTEROID = "#"
S_SPACE    = "."

def read_asteroids(filename: str) -> set[tuple[int, int]]:
    res: set = set()
    with open(filename, "r") as fp:
        for row, line in enumerate(fp):
            for col, char in enumerate(line):
                if char == "#":
                    res.add((row, col))
    return res

def enjoy_scenery(asteroids: set) -> dict[tuple[int, int], int]:
    res: dict = dict()
    for a in asteroids:
        tmp = list(asteroids - {a})
        res[a] = len(
            set([
                atan2(a[1] - tmp[i][1], tmp[i][0] - a[0])
                for i in range(len(tmp))
            ])
        )
    return res

def set_targets(
    base: tuple[int, int],
    asteroids: set
) -> dict[float, list[tuple[int, int]]]:
    result: dict[float, list[tuple[int, int]]] = {}
    for a in asteroids:
        if a == base:
            continue
        angle = atan2(a[1] - base[1], base[0] - a[0]) # trust me dude
        if angle not in result:
            result[angle] = list()
        result[angle].append(a)
        result[angle].sort(key=lambda x: distance(base, x))
    return result

def distance(a: tuple[int, int], b: tuple[int, int]) -> int:
    # Manhattan will do
    return sum(abs(a[dim] - b[dim]) for dim in range(len(a)))

def main() -> None:
    asteroids: set = read_asteroids("input10.txt")
    # Part 1
    view: dict = enjoy_scenery(asteroids)
    best_result: tuple[tuple[int, int], int] = sorted(
        view.items(),
        key=(lambda x: x[1])
    ).pop()
    print(best_result[1])
    # Part 2
    best_coord: tuple[int, int] = best_result[0]
    targets: dict[float, list[tuple[int, int]]] = set_targets(
        best_coord,
        asteroids
    )
    start_angle = atan2(0, 1)
    if start_angle not in targets:
        targets[start_angle] = list()
    # Loop through target's angles, popping the closest each time
    angles: list[int] = sorted(targets)
    index: int = angles.index(start_angle)
    kills: int = 0
    while kills < len(asteroids) - 1:
        while len(targets[angles[index]]) == 0:
            index = (index + 1) % len(angles)
        target = targets[angles[index]].pop(0)
        #print(f"Vapourised {target}.")
        kills += 1
        index = (index + 1) % len(angles)
        if kills == 200:
            y, x = target
            print(100 * x + y)
            break

if __name__ == "__main__":
    main()
