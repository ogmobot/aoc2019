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

    def find_path(self, start, end):
        to_search = [[start]]
        visited = set()
        while to_search:
            path = to_search.pop(0)
            if path[0] == end:
                return path
            visited.add(path[0])
            for adj in self.get_adjacent(path[0]):
                if adj not in visited:
                    to_search.append([adj] + path)
        return None

def main():
    maze = PortalMaze("input20.txt")
    path = maze.find_path(maze.start, maze.end)
    #print(path)
    print(len(path) - 1) # path includes start point
    #portals = []
    #for p in path:
        #if p in maze.portals:
            #portals.append(maze.portal_names[p])
    #print(portals)

if __name__ == "__main__":
    main()
