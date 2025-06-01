#  PSS AND RSS METRICS

## Description
This document summarizes memory usage measured by PSS (Proportional Set Size) and RSS (Resident Set Size) for each implementation evaluated in the project. The goal is to compare CPU memory requirements when generating zero-knowledge proofs with the same logic (DNI validation, password hashing, Sudoku solving)

## PSS (Proportional Set Size)
Indicates how much memory a process uses exclusively plus its fair share of any shared memory pages.  

**What it measures:**  
  The “proportional” portion of shared memory pages—each process is charged only its allocated fraction.  


| IMPLEMENTATION | AVERAGE MEMORY USAGE KB       | AVERAGE MEMORY USAGE %        |
| -----------    | ----------------------------- | ----------------------------- |
| Circom         | 189966.42Kb                   | 1.18%                         |
| libsnark       | 16519.81Kb                    | 0.10%                         |
| noir           | 59571.82Kb                    | 0.37%                         |
| risc-zero      | 11844.62Kb                    | 0.07%                         |


| IMPLEMENTATION | AVERAGE PEAK MEMORY USAGE KB  | AVERAGE PEAK MEMORY USAGE %  |
| -----------    | ----------------------------- | ---------------------------- |
| Circom         | 443306.23Kb                   | 2.76%                        |
| libsnark       | 22055.93Kb                    | 0.14%                        |
| noir           | 99981.37Kb                    | 0.62%                        |
| risc-zero      | 11844.90Kb                    | 0.07%                        |


## RSS (Resident Set Size)
Is the total physical memory (RAM) a process has in use at a given moment, counting both private and shared pages.  

**What it measures:**  
  The sum of all resident pages in RAM assigned to the process.  


| IMPLEMENTATION | AVERAGE MEMORY USAGE KB       | AVERAGE MEMORY USAGE %        |
| -----------    | ----------------------------- | ----------------------------- |
| Circom         | 119549.49Kb                   | 0.74%                         |
| libsnark       | 11209.05Kb                    | 0.07%                         |
| noir           | 35691.73Kb                    | 0.22%                         |
| risc-zero      | 13859.61Kb                    | 0.09%                         |


| IMPLEMENTATION | AVERAGE PEAK MEMORY USAGE KB  | AVERAGE PEAK MEMORY USAGE %  |
| -----------    | ----------------------------- | ---------------------------- |
| Circom         | 252369.20Kb                   | 1.57%                        |
| libsnark       | 13954.00Kb                    | 0.09%                        |
| noir           | 58500.00Kb                    | 0.36%                        |
| risc-zero      | 13859.73Kb                    | 0.09%                        |