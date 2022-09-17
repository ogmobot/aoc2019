#!/usr/bin/env lua

centre_of_mass  = "COM"
your_location   = "YOU"
santa_location  = "SAN"

get_orbits = function(filename)
    local tmp = io.input()
    local res = {}
    io.input(filename)
    local text = io.read("*all")
    for planet, moon in text:gmatch("(%w+)%)(%w+)") do
        res[moon] = planet
    end
    io.input():close()
    io.input(tmp)
    return res
end

distance_to_com = function(orbits, moon, cache)
    if not cache[moon] then
        if orbits[moon] == centre_of_mass then
            cache[moon] = 1
        else
            cache[moon] = 1 + distance_to_com(orbits, orbits[moon], cache)
        end
    end
    return cache[moon]
end

path_to_com = function(orbits, moon)
    if moon == centre_of_mass then
        return {[1] = centre_of_mass}
    else
        local tmp = path_to_com(orbits, orbits[moon])
        table.insert(tmp, moon)
        return tmp
    end
end

calc_distances = function(orbits)
    local res = {}
    for k, _ in pairs(orbits) do
        res[k] = distance_to_com(orbits, k, res)
    end
    return res
end

main = function()
    local orbits = get_orbits("input06.txt")
    -- Part 1
    local distances = calc_distances(orbits)
    local total = 0
    for _, v in pairs(distances) do
        total = total + v
    end
    print(total)
    -- Part 2
    local you_to_com = path_to_com(orbits, your_location)
    local san_to_com = path_to_com(orbits, santa_location)
    local num_equal = 0
    for i = 1, (# you_to_com) do
        if you_to_com[i] == san_to_com[i] then
            num_equal = num_equal + 1
        else
            break
        end
    end
    -- Don't include starting points in path length
    print((# you_to_com - 1) + (# san_to_com - 1) - (2 * num_equal))
end

main()
