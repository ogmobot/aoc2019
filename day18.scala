#!/usr/bin/env scala
import scala.io.Source
import scala.collection.mutable
import scala.collection.immutable

class Maze() {
    // Always index these with (row, col)
    val pos = mutable.Map.empty[(Int, Int), Char]
    val keyLocations = mutable.Map.empty[Char, (Int, Int)]

    def adjacent(initial: (Int, Int)): List[(Int, Int)] = {
        val (row, col) = initial
        List(
            (row + 1, col), (row - 1, col), (row, col + 1), (row, col - 1)
        ).filter(x => pos(x) != '#') // Input is padded so this can't OoB
    }
}

case class State(position: (Int, Int), collected: immutable.Set[Char])
case class SearchNode(distance: Int, state: State)
case class Requirement(distance: Int, keys: immutable.Set[Char])

def readMaze(filename: String): Maze = {
    val lines: Iterator[String] = Source.fromFile(filename).getLines()
    var maze = new Maze
    lines.zipWithIndex.foreach{ case (line, row) => {
        line.zipWithIndex.foreach{ case (char, col) => {
            maze.pos.update((row, col), char)
            if (char == '@' || ('a' <= char && char <= 'z'))
                maze.keyLocations.update(char, (row, col))
        }}
    }}
    return maze
}

def keyToKey(maze: Maze, fromKey: Char, toKey: Char): Requirement = {
    val seenPositions = mutable.Set.empty[(Int, Int)]
    // Each node in this list has (distance, (row, col))
    var toSearch = mutable.PriorityQueue.empty[(Requirement, (Int, Int))](
        Ordering.by((_: (Requirement, (Int, Int)))._1.distance).reverse
    )
    toSearch.enqueue(
        (Requirement(0, immutable.Set.empty[Char]), maze.keyLocations(fromKey))
    )
    while (toSearch.length > 0) {
        val (requirement, position) = toSearch.dequeue()
        seenPositions.add(position)
        val current =  maze.pos(position)
        if (current == toKey)
            return requirement

        val newRequirement = Requirement(
            requirement.distance + 1,
            if ('A' <= current && current <= 'Z')
                requirement.keys + current.toLower
            else
                requirement.keys
        )
        val opts: List[(Requirement, (Int, Int))] =
            maze.adjacent(position)
                .filter(x => !seenPositions.contains(x))
                .map(x => (newRequirement, x))
        toSearch ++= opts
    }
    return Requirement(-1, immutable.Set.empty[Char])
}

def getReqMatrix(maze: Maze): mutable.Map[(Char, Char), Requirement] = {
    // Produces a Map for which entry (a, b) contains a Requirement* to get from
    // key a to key b.
    // The Requirement is the steps required, and the set of keys needed.
    val keyNames = maze.keyLocations.keys.toArray
    val result = mutable.Map.empty[(Char, Char), Requirement]
    keyNames.map( a =>
        keyNames.map(b =>
            if (a != b && !result.contains((a, b))) {
                val req = keyToKey(maze, a, b)
                result.update((a, b), req)
                result.update((b, a), req)
            }
        )
    )
    return result
}

def main(): Unit = {
    val maze = readMaze("input18.txt")
    // Takes ~7s to build this
    val reqMatrix = getReqMatrix(maze)
}

main()
