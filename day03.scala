#!/usr/bin/env scala
import scala.io.Source
import scala.collection.mutable

def manhattan(a: (Int, Int)): Int = a._1.abs + a._2.abs

def putGrid(
    grid: mutable.Map[(Int, Int), Int],
    start_dist: ((Int, Int), Int),
    wireSection: String
): ((Int, Int), Int) = {
    val delta = wireSection(0) match {
        case 'U' => ( 0, -1)
        case 'D' => ( 0,  1)
        case 'L' => (-1,  0)
        case 'R' => ( 1,  0)
    }
    val (start, dist) = start_dist
    val amount = wireSection.substring(1).toInt
    (1 to amount).map(d => {
        val coord = (start._1 + (d * delta._1), (start._2 + (d * delta._2)))
        grid.update(coord, grid.getOrElse(coord, dist + d))
    })
    return (
        (start._1 + (amount * delta._1), start._2 + (amount * delta._2)),
        dist + amount
    )
}

def main() = {
    val wires = Source.fromFile("input03.txt")
                      .getLines()
                      .map(_.split(",").toList)
                      .toList
    val grid_1 = mutable.Map.empty[(Int, Int), Int]
    val grid_2 = mutable.Map.empty[(Int, Int), Int]
    var pt_dist = ((0, 0), 0)
    for (w <- wires(0))
        pt_dist = putGrid(grid_1, pt_dist, w)
    pt_dist = ((0, 0), 0)
    for (w <- wires(1))
        pt_dist = putGrid(grid_2, pt_dist, w)
    val intersections = grid_1.keys.toSet.intersect(grid_2.keys.toSet)
    // Part 1
    println(intersections.map(manhattan).min)
    // Part 2
    println(intersections.map(x => grid_1(x) + grid_2(x)).min)
}

main()
