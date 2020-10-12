# Nullspace Seedfinder
These are the tools written to find a seed which meets the requirements of the Nullspace Minecraft server. The requirements are:
- For Minecraft versions 1.7 and 1.8
- Triple witch huts within 2k of the origin
- Quad witch huts within 32k of the origin
- Double ocean monuments within 32k of the origin
- A mushroom island, mesa, ice spikes and jungle within 2k of the origin

This was written using the [cubiomes](https://github.com/Cubitect/cubiomes) library, a Minecraft seedfinding library for C and C++.
The seedfinding process is split up into 5 tools, described below:
- `find_lower_bits`: Finds the lower 20 bits of seeds which have potential to generate quad witch huts at (0,0).
- `find_origin_quads:` Finds base seeds for quad witch huts at (0,0).
- `find_multi_bases`: Finds quad witch hut base seeds which also have a triple witch hut within a given distance and translates them such that the triple witch huts are at (0,0).
- `find_final_seeds`: Translates triple + quad witch hut base seeds, moving the triple hut to all regions within an acceptable range of (0,0). The upper 16 bits of each of these structure seeds are then bruteforced to find valid biomes at the triple and quad witch hut locations, as well as valid biomes for a double ocean monument.
- `find_rare_biomes`: Searches a list for seeds with certain rare biomes within an acceptable range of (0,0).


## Warning
These tools are not intended for general use, and are not polished or well-documented. If you have similar seed requirements to those above, you should probably just look through the seeds already found with these tools in [final_seeds.csv](https://github.com/Pineapplecake/nullspace-seedfinder/blob/master/seeds/final_seeds.csv) (No special biome requirements) or [best_seeds.csv](https://github.com/Pineapplecake/nullspace-seedfinder/blob/master/seeds/best_seeds.csv). (With biome requirements)

## Compiling
Compile the tools using the provided makefile. Each one can be specified as its own individual target, or they can all be compiled with optimizations using `make release`.
