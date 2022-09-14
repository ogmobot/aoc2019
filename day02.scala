#!/usr/bin/env scala
import scala.io.Source
import scala.collection.mutable

def manhattan(a: (Int, Int)): Int = a._1.abs + a._2.abs

def putGrid(
    grid: mutable.Map[(Int, Int), Int],
    start: (Int, Int),
    wireSection: String
): (Int, Int) = {
    val delta = wireSection(0) match {
        case 'U' => ( 0, -1)
        case 'D' => ( 0,  1)
        case 'L' => (-1,  0)
        case 'R' => ( 1,  0)
    }
    val dist = grid.getOrElse(start, 0)
    val amount = wireSection.substring(1).toInt
    (1 to amount).map { d =>
        grid.update(
            (start._1 + (d * delta._1), (start._2 + (d * delta._2))),
            dist + d
        )
    }
    return (start._1 + (amount * delta._1), start._2 + (amount * delta._2))
}

def main() = {
    val wires = Source.fromFile("input02.txt").getLines().map(
        _.split(",").toList
    ).toList
    val grid_1 = mutable.Map.empty[(Int, Int), Int]
    val grid_2 = mutable.Map.empty[(Int, Int), Int]
    var pt = (0, 0)
    for (w <- wires(0)) {
        pt = putGrid(grid_1, pt, w)
    }
    pt = (0, 0)
    for (w <- wires(1)) {
        pt = putGrid(grid_2, pt, w)
    }
    val intersections = grid_1.keys.toSet.intersect(grid_2.keys.toSet)
    // Part 1
    println(intersections.map(manhattan).min)
    // Part 2
    println(intersections.map({ x => grid_1(x) + grid_2(x) }).min)
}

main()
