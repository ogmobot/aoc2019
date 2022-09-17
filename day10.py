#!/usr/bin/env python3
from math import atan2

def read_asteroids(filename: str) -> list[tuple[int, int]]:
    with open(filename, "r") as fp:
        return [
            (row, col)
            for row, line in enumerate(fp)
            for col, char in enumerate(line)
            if char == "#"
        ]

def set_targets(
    base: tuple[int, int],
    asteroids: list[int, int]
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
    asteroids: list[tuple[int, int]] = read_asteroids("input10.txt")
    # Part 1
    scenery: dict[tuple[int, int], dict[float, list[tuple[int, int]]]] = {
        a: set_targets(a, asteroids)
        for a in asteroids
    }
    targets: tuple[tuple[int, int], dict[float, list[tuple[int,int]]]] = sorted(
        scenery.items(),
        key=(lambda x: len(x[1]))
    ).pop()[1]
    print(len(targets))
    # Part 2
    start_angle = atan2(0, 1)
    if start_angle not in targets:
        targets[start_angle] = list()
    # Loop through target's angles, popping the closest each time
    angles: list[float] = sorted(targets)
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
