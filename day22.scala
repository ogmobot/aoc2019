#!/usr/bin/env scala
import scala.io.Source
import scala.util.matching.Regex
import scala.math.BigInt

// This puzzle is all about linear transforms.
// Let the tuple (a, b, n) represent the transform f(x) = ax + b (mod n)

def foldLinear(
    base: (BigInt, BigInt, BigInt),
    line: String
): (BigInt, BigInt, BigInt) = {
    val (a, b, n) = base
    val reverseMatcher:  Regex = "deal into new stack".r
    val multiplyMatcher: Regex = "deal with increment ([0-9]+)".r
    val offsetMatcher:   Regex = "cut (-?[0-9]+)".r
    line match {
        case reverseMatcher()   => (n - a,             n - b - 1,             n)
        case multiplyMatcher(m) => ((a * m.toInt) % n, (b * m.toInt) % n,     n)
        case offsetMatcher(c)   => (a,                 (n + b - c.toInt) % n, n)
        case _ => throw new RuntimeException(s"Failed to read '${line}'")
    }
}

def applyLinearMod(x: BigInt, linearMod: (BigInt, BigInt, BigInt)): BigInt =
    (((x * linearMod._1) + linearMod._2) % linearMod._3)

def iterateNTimes(
    linearMod: (BigInt, BigInt, BigInt),
    n: BigInt
): (BigInt, BigInt, BigInt) = {
    // Computes f^n(x) = f(f(f(...(x)...)))
    // What's the formula for a geometric series sum again..?
    val (a, b, mod) = linearMod
    (a.modPow(n, mod), b * (a.modPow(n, mod) - 1) * modInverse(a - 1, mod), mod)
    // Note that this works with negative iterations too!
}

def modInverse(x: BigInt, mod: BigInt): BigInt = x.modPow(mod - 2, mod)

def idLinearMod(mod: BigInt): (BigInt, BigInt, BigInt) =
    (BigInt(1), BigInt(0), mod)
// Returns the identity linear transform (i.e. y = 1x + 0) for a given modulus

def main(): Unit = {
    val lines = Source.fromFile("input22.txt").getLines().toArray

    val partOneShuffle = lines.foldLeft(idLinearMod(BigInt(10007)))(foldLinear)
    println(applyLinearMod(BigInt(2019), partOneShuffle))

    val partTwoCards      = BigInt("119315717514047")
    val partTwoIterations = BigInt("101741582076661")
    val partTwoShuffle = iterateNTimes(
        lines.foldLeft(idLinearMod(partTwoCards))(foldLinear),
        -partTwoIterations
    )
    println(applyLinearMod(2020, partTwoShuffle))
}

main()
