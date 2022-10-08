#!/usr/bin/env python3
import sys

class PortalMaze:
    def __init__(self, filename):
        self.grid = {}
        self.portals: dict(tuple(int, int), tuple(int, int)) = {}
        self.portal_names: dict(tuple(int, int), str) = {}
        portal_candidates = set()
        with open(filename, "r") as fp:
            for row, line in enumerate(fp):
                for col, symbol in enumerate(line):
                    self.grid[(row, col)] = symbol
                    if symbol.isupper():
                        portal_candidates.add((row, col))
        open_portals = {}
        for coord in portal_candidates:
            neighbours = [
                (coord[0] + d[0], coord[1] + d[1])
                for d in ((1, 0), (-1, 0), (0, 1), (0, -1))
            ]
            if any(self.grid.get(n, "#") == "." for n in neighbours):
                # This is a portal
                portal_name = "".join(sorted([self.grid[coord]] + [
                    self.grid.get(n)
                    for n in neighbours
                    if self.grid.get(n, ' ').isupper()]))
                portal_pos = [
                    n
                    for n in neighbours
                    if self.grid.get(n) == '.'
                ].pop()
                self.portal_names[portal_pos] = portal_name
                if portal_name in open_portals:
                    self.portals[open_portals[portal_name]] = portal_pos
                    self.portals[portal_pos] = open_portals[portal_name]
                else:
                    open_portals[portal_name] = portal_pos
        self.start = open_portals["AA"]
        self.end = open_portals["ZZ"]

    def portal_name(self, coord):
        neighbours = [
            (coord[0] + d[0], coord[1] + d[1])
            for d in ((1, 0), (-1, 0), (0, 1), (0, -1))
        ]
        return "".join(sorted([self.grid[coord]] + [
            self.grid.get(n)
            for n in neighbours
            if self.grid.get(n, ' ').isupper()]))

    def get_adjacent(self, coord):
        res = []
        for delta in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            target = (coord[0] + delta[0], coord[1] + delta[1])
            if self.grid[target] == '.' or target in [self.start, self.end]:
                res.append(target)
        if coord in self.portals:
            res.append(self.portals[coord])
        return res

    def find_path_length(self, start, end):
        to_search = [(start, 0)]
        visited = set()
        while to_search:
            path = to_search.pop(0)
            if path[0] == end:
                return path[1]
            visited.add(path[0])
            for adj in self.get_adjacent(path[0]):
                if adj not in visited:
                    to_search.append((adj, path[1] + 1))
        return None

class RecursiveMaze(PortalMaze):
    def __init__(self, template: PortalMaze):
        self.grid = template.grid
        self.start = (template.start[0], template.start[1], 0)
        self.end = (template.end[0], template.end[1], 0)
        self.inner_portals = {}
        self.outer_portals = {}

        portals = template.portals
        min_row = min(k[0] for k in portals)
        max_row = max(k[0] for k in portals)
        min_col = min(k[1] for k in portals)
        max_col = max(k[1] for k in portals)
        for p, q in portals.items():
            if p[0] in [min_row, max_row] or p[1] in [min_col, max_col]:
                self.outer_portals[p] = q
            else:
                self.inner_portals[p] = q

    def get_adjacent(self, coord):
        res = []
        for delta in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            target = (coord[0] + delta[0], coord[1] + delta[1], coord[2])
            if self.grid[target[:2]] == '.' or target in [self.start, self.end]:
                res.append(target)
        if coord[:2] in self.inner_portals:
            res.append(self.inner_portals[coord[:2]] + (coord[2] + 1,))
        if coord[:2] in self.outer_portals and coord[2] > 0:
            res.append(self.outer_portals[coord[:2]] + (coord[2] - 1,))
        return res

def main():
    # Part 1
    maze = PortalMaze("input20.txt")
    print(maze.find_path_length(maze.start, maze.end))
    #portals = []
    #for p in path:
        #if p in maze.portals:
            #portals.append(maze.portal_names[p])
    #print(portals)

    # Part 2 (Takes ~20 sec)
    rec_maze = RecursiveMaze(maze)
    print(rec_maze.find_path_length(rec_maze.start, rec_maze.end))

if __name__ == "__main__":
    main()
