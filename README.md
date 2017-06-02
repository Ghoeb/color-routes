# Color Routes

Repository for experimenting with backtracking and enhancements to a graph edge assignment problem

Written to use as an assignment on the 2016-1 edition of Data Structures & Algorithms course at PUC, Chile

(This was my first big C project, and I plan to keep working on it and cleaning it up)

## The problem

Given an edgeless graph where nodes have a specific desired degree, and some nodes have a color assigned while others don't, set the edges to satisfy the following:

_If (u,v) is an edge on the graph, then the nodes u and v have the same color assigned_

_All nodes on the graph have a color assigned_

_All nodes have their desired degree_

Considering that you can't change the color already assigned to a node.

## Dependencies

This program's visualizer depends on Gtk+ 3.0 to work

## Running the program

This consists of 3 programs, which will be all compiled by entering

```
make
```

on the terminal.

They all depend on eachother to work, so I'll explain them in call order.

### Generator

Generates a puzzle based on a given bmp file. The information on each color channel specifies something to the puzzle. Outputs the puzzle to `stdout`, which you can redirect to any of the other 2 programs, or to a file.

Run with

```
./generator <bmp> <seed> [-d]
```

`<bmp>` is the bmp file to base the puzzle on

`<seed>` is the random seed for the puzzle

`[-d]` is the debug flag. Show every step taken by the generator. Only makes sense if passing `stdout` to the `watcher`.

### Solver

The heuristic solver (solver_h) solves the puzzle using backtracking pruning and heuristics. Reads the puzzle from `stdin`, and outputs the solution steps to `stdout`, which you should redirect to the `watcher`

Run with

```
./solver_h [-s]
```

`[-s]` is to print every step taken by the algorithm. Use this to see the undos

### Watcher

Opens a GTK+ 3.0 window to visualize the steps taken on the puzzle in a pretty representation of the graph. Reads the steps from `stdin`.

Run with

```
./watcher [-u] [timestep]
```

`[-u]` is to show the underlying graph of the problem.

`[timestep]` is to regulate the pause between 2 updates to the window.

### All together

The idea is to join the 3 programs together

```
./generator <bmp> <seed> | ./solver_h [-s] | ./watcher [-u]
```

or

```
./generator <bmp> <seed> [-d] | ./watcher [-u]
```
