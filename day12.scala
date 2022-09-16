#!/usr/bin/env scala
import scala.io.Source
import scala.util.matching.Regex
import scala.math.BigInt

class Moon(
    var  x: Int,    var  y: Int,    var  z: Int,
    var vx: Int,    var vy: Int,    var vz: Int
) {
    def applyGravity(other: Moon): Unit = {
        if (other.x > x) vx += 1
        if (other.x < x) vx -= 1
        if (other.y > y) vy += 1
        if (other.y < y) vy -= 1
        if (other.z > z) vz += 1
        if (other.z < z) vz -= 1
    }
    def update(): Unit = {
        x += vx
        y += vy
        z += vz
    }
    def energy(): Int = (x.abs + y.abs + z.abs) * (vx.abs + vy.abs + vz.abs)
}

// Logic functions

def lowestCommonMultiple(a: BigInt, b: BigInt): BigInt = a * b / a.gcd(b)

def applyGravityToAll(moons: List[Moon]): Unit = {
    moons.map { m => {
        moons.map { _.applyGravity(m) }
    }}
}

def extractDimensions(moons: List[Moon]): List[List[Int]] = {
    var xs = List.empty[Int]
    var ys = List.empty[Int]
    var zs = List.empty[Int]
    for (moon <- moons) {
        xs ++= List(moon.x, moon.vx)
        ys ++= List(moon.y, moon.vy)
        zs ++= List(moon.z, moon.vz)
    }
    return List(xs, ys, zs)
}

// IO functions

def lineToMoon(line: String): Moon = {
    val matcher: Regex = "<x=(-?[0-9]+), y=(-?[0-9]+), z=(-?[0-9]+)>".r
    line match {
        case matcher(x, y, z) => new Moon(x.toInt, y.toInt, z.toInt, 0, 0, 0)
        case _                => new Moon(0, 0, 0, 0, 0, 0)
    }
}

def printMoons(moons: List[Moon]): Unit = {
    moons.map { m =>
        println(s"<x=${m.x}, y=${m.y}, z=${m.z}> " ++
            s"vel=<x=${m.vx}, y=${m.vy}, z=${m.vz}>")
    }
}

// Main function

def main(): Unit = {
    val moons: List[Moon] = Source.fromFile("input12.txt")
                                  .getLines()
                                  .map(lineToMoon)
                                  .toList
    val initial: List[List[Int]] = extractDimensions(moons)
    var cycles: Array[Int] = Array(0, 0, 0)
    var timer: Int = 0
    while (cycles.min == 0) {
        applyGravityToAll(moons)
        moons.map { _.update() }
        timer += 1
        //printMoons(moons)

        // Part 1
        if (timer == 1000)
            println(moons.map { _.energy() } .sum)
        // Part 2
        val current: List[List[Int]] = extractDimensions(moons)
        for (dim <- (0 until cycles.length)) {
            if (cycles(dim) == 0 && current(dim) == initial(dim))
                cycles(dim) = timer
        }
    }
    //println(s"${cycles(0)} ${cycles(1)} ${cycles(2)}")
    println(
        cycles.foldLeft(BigInt(1))((a, b) =>
            lowestCommonMultiple(a, BigInt(b))))
}

main()
