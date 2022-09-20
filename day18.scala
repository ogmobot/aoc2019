#!/usr/bin/env scala
import scala.io.Source
import scala.collection.mutable
import scala.collection.immutable

class Maze() {
    // Always index these with (row, col)
    val pos = mutable.Map.empty[(Int, Int), Char]
    var start: (Int, Int) = (0, 0)
    var numKeys: Int = 0

    def adjacent(initial: (Int, Int)): List[(Int, Int)] = {
        val (row, col) = initial
        List(
            (row + 1, col), (row - 1, col), (row, col + 1), (row, col - 1)
        ).filter(x => pos(x) != '#') // Input is padded so this can't OoB
    }
}

case class State(position: (Int, Int), collected: immutable.Set[Char])
case class SearchNode(distance: Int, state: State)

def readMaze(filename: String): Maze = {
    val lines: Iterator[String] = Source.fromFile(filename).getLines()
    var maze = new Maze
    lines.zipWithIndex.foreach{ case (line, row) => {
        line.zipWithIndex.foreach{ case (char, col) => {
            maze.pos.update((row, col), char)
            if (char == '@')
                maze.start = (row, col)
            if ('a' <= char && char <= 'z')
                maze.numKeys += 1
        }}
    }}
    return maze
}

// Naive approach -- used it to discover the solution is longer than 2970 steps.
// The process took ~90 min.
def _getKeys(maze: Maze): Int = {
    val seenStates = mutable.Set.empty[State]
    var toSearch = mutable.PriorityQueue.empty[SearchNode](
        Ordering.by((_: SearchNode).distance).reverse
    )
    toSearch.enqueue(
        SearchNode(0, State(maze.start, immutable.Set.empty[Char]))
    )
    var highestDistance: Int = 0
    while (toSearch.length > 0) {
        var node = toSearch.dequeue()
        if (node.distance > highestDistance) {
            println("Searching paths of distance=")
            println(node.distance)
            highestDistance = node.distance
        }
        seenStates.add(node.state)
        val current =  maze.pos(node.state.position)
        if ('a' <= current && current <= 'z')
            node = SearchNode(
                node.distance,
                State(node.state.position, node.state.collected + current)
            )
        if (node.state.collected.size == maze.numKeys)
            return node.distance
        val opts: List[SearchNode] =
            maze.adjacent(node.state.position).filter(x =>
                maze.pos(x) < 'A' || maze.pos(x) > 'Z' ||
                    node.state.collected.contains(maze.pos(x).toLower)
            )
            .map(x =>
                SearchNode(
                    node.distance + 1,
                    State(x, node.state.collected)))
            .filter(x => !seenStates.contains(x.state))
        toSearch ++= opts
    }
    return -1
}

def main(): Unit = {
    val maze = readMaze("input18.txt")
    println(getKeys(maze))
}

main()
