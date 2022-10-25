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
    val maze = new Maze
    lines.zipWithIndex.foreach{ case (line, row) => {
        line.zipWithIndex.foreach{ case (char, col) => {
            maze.pos.update((row, col), char)
            if (char == '@' || char.isLower)
                maze.keyLocations.update(char, (row, col))
        }}
    }}
    return maze
}

def quadrify(maze: Maze): Array[Maze] = {
    // This is not the best method of solving this problem.
    // It'd be better to just keep track of where the four robots are in a
    // single maze.
    val cRow: Int = maze.keyLocations('@')._1
    val cCol: Int = maze.keyLocations('@')._2
    val maxRow = maze.pos.keys.map(_._1).max
    val maxCol = maze.pos.keys.map(_._2).max
    val pos = maze.pos.clone()
    Array(
        (cRow, cCol), (cRow + 1, cCol), (cRow - 1, cCol),
                      (cRow, cCol + 1), (cRow, cCol - 1)
    ).foreach(coord => pos.update(coord, '#'))
    Array(
        (cRow + 1, cCol + 1), (cRow + 1, cCol - 1),
        (cRow - 1, cCol + 1), (cRow - 1, cCol - 1)
    ).foreach(coord => pos.update(coord, '@'))
    Array(
        ((0 to cRow), (0 to cCol)),
        ((cRow to maxRow), (0 to cCol)),
        ((0 to cRow), (cCol to maxCol)),
        ((cRow to maxRow), (cCol to maxCol))
    ).map{ case (rowRange, colRange) => {
        val m = new Maze
        rowRange.map(row => {
            colRange.map(col => {
                m.pos((row, col)) = pos((row, col))
                if (m.pos((row, col)) == '@' || m.pos((row, col)).isLower)
                    m.keyLocations.update(m.pos((row, col)), (row, col))
            })
        })
        m
    }}
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
                if (req.distance != -1) {
                    result.update((a, b), req)
                    result.update((b, a), req)
                }
            }
        )
    )
    return result
}

def robotPositions(
    path: List[Char],
    reqMatrix: mutable.Map[(Char, Char), Requirement]
): immutable.Set[Char] = {
    var covered = Set('@')
    var res = immutable.Set.empty[Char]
    var index = 0
    while (covered.size < path.length) {
        while (covered.contains(path(index)))
            index += 1
        res += path(index)
        covered += path(index)
        covered ++= path.filter(x => reqMatrix.contains((path(index), x)))
    }
    return res
}

def calcDistance(
    path: List[Char],
    reqMatrix: mutable.Map[(Char, Char), Requirement]
): Int =
    if (path.length <= 1)
        0
    else
        // In part 2, pairs of values aren't necessarily in the same submaze
        reqMatrix(
            path.head,
            path.tail
                .filter(x => reqMatrix.contains((path.head, x)))
                .head
        ).distance + calcDistance(path.tail, reqMatrix)

def solveMaze(
    mazes: Array[Maze],
    reqMatrix: mutable.Map[(Char, Char), Requirement]
): Int = {
    println("Solving maze...")
    var toSearch = mutable.PriorityQueue.empty[List[Char]](
        Ordering.by(calcDistance(_: List[Char], reqMatrix)).reverse
    )
    // For each search node, first char is most recently visited
    // minDists is (collected keys, robot positions) => distance
    val minDists = mutable.Map.empty[(
        immutable.Set[Char],
        immutable.Set[Char]
    ), Int]
    val keyLocations = mazes.map(_.keyLocations).reduce(_ ++ _)
    toSearch.enqueue(List('@'))
    while (toSearch.length > 0) {
        val path = toSearch.dequeue()
        var distance = calcDistance(path, reqMatrix)
        var rPositions = robotPositions(path, reqMatrix)
        if (rPositions.size < mazes.length)
            rPositions += '@'
        if ((!minDists.contains((path.toSet, rPositions))) ||
                (minDists((path.toSet, rPositions)) > distance)) {
            minDists.update((path.toSet, rPositions), distance)
            if (path.length == keyLocations.size) {
                println(path.reverse)
                return distance
            }

            val opts: List[List[Char]] =
                keyLocations.keys.toList
                    .filter(x => !(path.contains(x)))
                    .filter(x =>
                        rPositions
                            .map(rPos =>
                                (reqMatrix.contains((rPos, x)) &&
                                reqMatrix((rPos, x)).keys.subsetOf(path.toSet))
                            )
                            .exists(p => p)
                    )
                    .map(x => x :: path)
            toSearch ++= opts
        }
    }
    return -1
}

def solveSingle(maze: Maze): Int = {
    println("Building Requirement matrix...")
    val reqMatrix = getReqMatrix(maze) // Takes a second or two to calculate
    solveMaze(Array(maze), reqMatrix)
}

def solveSimul(mazes: Array[Maze]): Int = {
    println("Building Requirement matrices...")
    val reqMatrices = mazes.map(getReqMatrix)
    solveMaze(mazes, reqMatrices.reduce(_ ++ _))
}

def main(): Unit = {
    val maze = readMaze("input18.txt")
    // Part 1 (takes ~100s)
    println(solveSingle(maze))
    // Part 2 (takes ~500s)
    println(solveSimul(quadrify(maze)))
}

main()
