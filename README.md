#  Battle Game (C++)

## Project Structure

A modular, multi-threaded C++ project designed with two independent components:

1. **Game Manager & Algorithm (.so plugins)**  
   Game manager - Implements the game logic, including tank movement, collisions, shell trajectories.
   Algorithm - decision-making algorithm.
   Each Game Manager and Algorithm is compiled as a shared library, allowing multiple implementations to be loaded dynamically.

3. **Simulator**
   Executes battles between different Game Managers and Algorithms, manages threading, parses input maps , and records match results.  
   It runs multiple games concurrently and supports competition and comparative modes.

The project was part of the TAU course, Advanced Topics in Programming, emphasizing modularity, concurrency, and dynamic linking.  
(The project still has a few unresolved issues due to the academic deadline — currently being refined and improved.)

## Build & Run Instructions

This part explains how to build and run the different parts of the assignment from the command line using make.

Note: assume using Linux/WSL (Ubuntu).

### Build
from project root:
make all

Build outputs:  
Simulator/simulator_207177197_301521571 – main simulator executable
GameManager/libGameManager_207177197_301521571.so – GM dynamic library
Algorithm/libAlgorithm_207177197_301521571.so – Algorithma and layer dynamic library

### Run
from Simulator directory:

**Comparative run**

./simulator_207177197_301521571 -comparative \
game_map=<game_map_filename> \
game_managers_folder=<game_managers_folder> \
algorithm1=<algorithm_so_filename> \
algorithm2=<algorithm_so_filename> \
[num_threads=<num>] [-verbose]

**Competition run** 

./simulator_207177197_301521571 \
-competition game_maps_folder=<game_maps_folder> \
game_manager=<game_manager_so_filename> \
algorithms_folder=<algorithms_folder> \
[num_threads=<num>] [-verbose]


