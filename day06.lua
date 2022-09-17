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

path_to_com = function(orbits, moon)
    local res = {[1] = moon}
    while (moon ~= centre_of_mass) do
        moon = orbits[moon]
        table.insert(res, moon)
    end
    return res
end

main = function()
    local orbits = get_orbits("input06.txt")
    -- Part 1
    local total = 0
    for _, moon in pairs(orbits) do
        total = total + (# path_to_com(orbits, moon))
    end
    print(total)
    -- Part 2
    local you_to_com = path_to_com(orbits, your_location)
    local san_to_com = path_to_com(orbits, santa_location)
    while table.remove(you_to_com) == table.remove(san_to_com) do
        --[[
        Comparing the paths in this way is destructive, and in addition to the
        common nodes, one extra node is removed from both paths!
        For instance:
            (COM) -- (A) -- (B) -- (C) -- (YOU)
                        \
                         (D) -- (E) -- (SAN)
        In this scenario, nodes (B) and (D) would be removed from both paths.
        (However, since we shouldn't include our starting points, this saves us
        the trouble of subtracting one from each path length)
        --]]
    end
    print((# you_to_com) + (# san_to_com))
end

main()
