#!/usr/bin/env scala
import scala.io.Source
import scala.util.matching.Regex
import scala.math.BigInt

// This puzzle is all about linear transforms.
// Let the tuple (a, b, m) represent the transform f(x) = ax + b (mod m)

def foldTransformMod(
    base: (BigInt, BigInt, BigInt),
    line: String
): (BigInt, BigInt, BigInt) = {
    val (a, b, mod) = base
    val reverseMatcher:  Regex = "deal into new stack".r
    val multiplyMatcher: Regex = "deal with increment ([0-9]+)".r
    val addMatcher:      Regex = "cut (-?[0-9]+)".r
    line match {
        case reverseMatcher() => (
            (mod - a) % mod,
            (mod - b - 1) % mod,
            mod
        )
        case multiplyMatcher(m) => (
            (a * m.toInt) % mod,
            (b * m.toInt) % mod,
            mod
        )
        case addMatcher(c) => (
            a,
            (mod + b - c.toInt) % mod,
            mod
        )
        case _ => throw new RuntimeException(s"Failed to read '${line}'")
    }
}

def applyTransformMod(
    x: BigInt,
    transformMod: (BigInt, BigInt, BigInt)
): BigInt = (((x * transformMod._1) + transformMod._2) % transformMod._3)

def iterateNTimes(
    transformMod: (BigInt, BigInt, BigInt),
    n: BigInt
): (BigInt, BigInt, BigInt) = {
    // Computes f^n(x) = f(f(f(...(x)...)))
    // What's the formula for a geometric series sum again..?
    val (a, b, mod) = transformMod
    (a.modPow(n, mod), b*(a.modPow(n, mod) - 1)*modInverse(a - 1, mod), mod)
    // Note that this works with negative iterations too!
}

def modInverse(x: BigInt, mod: BigInt): BigInt = x.modPow(mod - 2, mod)

def main(): Unit = {
    val lines = Source.fromFile("input22.txt").getLines().toArray

    val partOneShuffle = lines.foldLeft(
        (BigInt(1), BigInt(0), BigInt(10007))
    )(foldTransformMod)
    println(applyTransformMod(BigInt(2019), partOneShuffle))

    val partTwoHalfShuffle = lines.foldLeft(
        (BigInt(1), BigInt(0), BigInt("119315717514047"))
    )(foldTransformMod)
    val partTwoReverseFullShuffle = iterateNTimes(
        partTwoHalfShuffle,
        -BigInt("101741582076661")
    )
    println(applyTransformMod(2020, partTwoReverseFullShuffle))
}

main()
