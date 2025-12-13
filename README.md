# High-Performance, Zero-Allocation Solver for 2L-UFLP (C)

## Overview
This repository contains a custom, high-performance solver implemented in **C** for the **Two-Level Uncapacitated Facility Location Problem (2L-UFLP)**. The core methodology is a hybrid **Bee Colony Optimization (BCO)** metaheuristic.

The development focus was strictly on **system-level optimization**, achieving maximal execution speed and predictable latency by **eliminating dynamic memory overhead** in critical runtime operations.

---

## 🚀 Key Performance Engineering Decisions

The solver's efficiency is rooted in two core principles: avoiding runtime memory allocation and optimizing data lookup complexity.

### 1. Zero-Allocation Runtime (The C Advantage)
To ensure the lowest possible execution latency and memory overhead, all dynamic memory allocation (`malloc`/`free`) was removed from the main optimization loops.
* **Pre-allocated Buffers:** All temporary arrays and worker memory required for the BCO and Local Search phases are allocated **once** at the beginning of the program.
* **Predictable Performance:** This eliminates expensive operating system calls for memory management inside the iterative loop, preventing heap fragmentation and maximizing performance predictability.

### 2. $O(1)$ Greedy Assignment (Offline Optimization)
The objective function calculation (assigning customers to the cheapest open two-level route) is the most computationally expensive part of the solver.

* **Offline Sorting:** For every customer $i$, the entire cost matrix $c_{ij_1 j_2}$ is pre-processed and sorted ascendingly into a single linear array **before** the optimization algorithm begins[cite: 161].
* **Optimized Lookup:** During the fitness evaluation (`upgradedSolutionValue`), the solver iterates through this pre-sorted array and stops at the very **first pair $(j_1, j_2)$** where both facilities are marked as open in the current solution[cite: 162].
* **Result:** This transforms the assignment subproblem from a potentially complex search into a near **$O(1)$** lookup for the minimum cost, drastically reducing the total execution time[cite: 162].

### 3. Algorithm & Logic
* **Hybrid Metaheuristic:** The BCO search is hybridized with a Local Search (LS) mechanism to improve the quality of solutions (escaping local optima) found by the **Recruiter** bees.
* [cite_start]**Solution Representation:** The problem solution is a two-segment binary sequence, where the first segment represents the decision to open facilities in $V_1$ ($y_{j_1}$), and the second segment represents $V_2$ ($y_{j_2}$)[cite: 155, 156].

---

## 🔎 Mathematical Model and Problem Definition
[cite_start]The problem addressed is the **Two-Level Uncapacitated Facility Location Problem (2L-UFLP)**, which aims to minimize the sum of fixed facility opening costs and customer assignment costs[cite: 121, 123].

For a detailed formal problem definition, including the objective function, constraints, and full mathematical notation, please refer to the attached document:

➡️ **[bco.pdf]**

---

## Technical Details

| Component | Technology | Description |
| :--- | :--- | :--- |
| **Language** | C (ISO C99) | Chosen for absolute control over memory and computational efficiency. |
| **Compilation** | GCC/Clang | Compiles with standard C flags. |
| **Dependencies** | Standard C Libraries (`stdio.h`, `stdlib.h`, `math.h`, `time.h`, `assert.h`). |

### Build and Run

To compile the source code, use the following commands:

```bash
# Compile the main program
gcc -Wall -O3 main.c -o solver -lm

# Example run: pass the data file as the first argument
./solver data/instance_small.txt
