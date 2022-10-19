#!/usr/bin/env scala
import scala.io.Source
import scala.util.matching.Regex
import scala.math.BigInt

class Moon(
    var pos: Array[Int]
) {
    var vel: Array[Int] = Array.fill(pos.length)(0)
    def applyGravity(other: Moon): Unit =
        for (dim <- 0 until pos.length) {
            if (other.pos(dim) > pos(dim)) vel(dim) += 1
            if (other.pos(dim) < pos(dim)) vel(dim) -= 1
        }
    def update: Unit =
        for (dim <- 0 until pos.length)
            pos(dim) += vel(dim)
    def energy: Int = pos.map(_.abs).sum * vel.map(_.abs).sum
}

// Logic functions

def lowestCommonMultiple(a: BigInt, b: BigInt): BigInt = a * b / a.gcd(b)

def applyGravityToAll(moons: Array[Moon]): Unit =
    moons.map(m => moons.map(_.applyGravity(m)))

def extractDimensions(moons: Array[Moon]): Array[Array[Int]] =
    moons.map(m => Array(m.pos, m.vel)).reduce((a, b) => a ++ b).transpose

// IO functions

def lineToMoon3D(line: String): Moon = {
    val matcher: Regex = "<x=(-?[0-9]+), y=(-?[0-9]+), z=(-?[0-9]+)>".r
    line match {
        case matcher(x, y, z) => new Moon(Array(x.toInt, y.toInt, z.toInt))
        case _                => new Moon(Array.empty[Int])
    }
}

def printMoons3D(moons: Array[Moon]): Unit =
    moons.map(m =>
        println(s"<x=${m.pos(0)}, y=${m.pos(1)}, z=${m.pos(2)}> " ++
            s"vel=<x=${m.vel(0)}, y=${m.vel(1)}, z=${m.vel(2)}>")
    )

// Main function

def main(): Unit = {
    val moons: Array[Moon] = Source.fromFile("input12.txt")
                                   .getLines()
                                   .map(lineToMoon3D)
                                   .toArray
    val initial: Array[Array[Int]] = extractDimensions(moons)
    var cycles: Array[Int] = Array.fill(initial.length)(0)
    var timer: Int = 0
    while (cycles.min == 0) {
        applyGravityToAll(moons)
        moons.map(_.update)
        timer += 1
        //printMoons3D(moons)

        // Part 1
        if (timer == 1000)
            println(moons.map(_.energy).sum)
        // Part 2
        val current: Array[Array[Int]] = extractDimensions(moons)
        for (dim <- 0 until cycles.length)
            if (cycles(dim) == 0 && current(dim).sameElements(initial(dim)))
                cycles(dim) = timer
    }
    //println(s"${cycles(0)} ${cycles(1)} ${cycles(2)}")
    println(
        cycles.map(BigInt(_)).reduce((a, b) =>
            lowestCommonMultiple(a, b)))
}

main()
