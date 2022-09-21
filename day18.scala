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

case class Requirement(distance: Int, keys: immutable.Set[Char])

def readMaze(filename: String): Maze = {
    val lines: Iterator[String] = Source.fromFile(filename).getLines()
    var maze = new Maze
    lines.zipWithIndex.foreach{ case (line, row) => {
        line.zipWithIndex.foreach{ case (char, col) => {
            maze.pos.update((row, col), char)
            if (char == '@' || char.isLower)
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
            if (current.isUpper)
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
    // Assumes there is only ever one path to get from key to key.
    // *The Requirement is the steps required, and the set of keys needed.
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

def calcDistance(
    path: List[Char],
    reqMatrix: mutable.Map[(Char, Char), Requirement]
): Int =
    if (path.length <= 1)
        0
    else
        path.zip(path.tail).map(pair => reqMatrix(pair).distance).sum

def solveMaze(maze: Maze, startKey: Char): Int = {
    println("Building Requirement matrix...")
    val reqMatrix = getReqMatrix(maze) // Takes a second or two to calculate
    println("Solving maze...")
    var toSearch = mutable.PriorityQueue.empty[List[Char]](
        Ordering.by(calcDistance(_: List[Char], reqMatrix)).reverse
    )
    // For each search node, first char is most recently visited
    val minDists = mutable.Map.empty[(immutable.Set[Char], Char), Int]
    toSearch.enqueue(List('@'))
    //var farthestDist = 0
    while (toSearch.length > 0) {
        val path = toSearch.dequeue()
        var distance = calcDistance(path, reqMatrix)
        //if (distance > farthestDist) {
            //println("Distance", distance)
            //farthestDist = distance
        //}
        //println("path", path.reverse, "distance", distance)
        if ((!minDists.contains((path.toSet, path.head))) ||
                (minDists((path.toSet, path.head)) > distance)) {
            minDists.update((path.toSet, path.head), distance)
            if (path.length == maze.keyLocations.size) {
                println(path.reverse)
                return distance
            }

            val opts: List[List[Char]] =
                maze.keyLocations.keys.toList
                    .filter(x => !(path.contains(x)))
                    .filter(x =>
                        reqMatrix((path.head, x)).keys.subsetOf(path.toSet)
                    )
                    .map(x => x :: path)
            toSearch ++= opts
        }
    }
    return -1
}

def main(): Unit = {
    val maze = readMaze("input18.txt")
    // Part 1 (takes ~40s)
    println(solveMaze(maze, '@'))
}

main()
