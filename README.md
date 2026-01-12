# OS Scheduler + Buddy Memory Allocation (Phase 2)

A Linux/WSL C project that simulates an OS-style CPU scheduler (SJF, Preemptive HPF, Round Robin, MLFP/MLFQ) and integrates a Buddy Memory Allocation system (Phase 2) to allocate/free memory for processes and log memory operations.

## Key Features
- CPU Scheduling Algorithms:
  - SJF (Shortest Job First)
  - PHPF (Preemptive Highest Priority First)
  - RR (Round Robin)
  - MLFP / MLFQ (Multi-Level Feedback Queue)
- Buddy Memory Allocation (Phase 2):
  - Allocates memory on process arrival
  - Frees memory on process completion
  - Splits and merges buddy blocks to reduce fragmentation
  - Generates `memory.log` in the required format

## My Contribution (Phase 2)
I implemented the Phase 2 extension:
- Buddy allocator logic (split/merge, allocate/free)
- Integration with scheduler events (arrival/finish)
- `memory.log` generation (allocation + free events with address ranges)

> Note: Phase 1 core scheduler/IPC structure was a team effort. See Credits below.

## Build
> Update the file list if your repo uses different .c files.

### Option 1: Using Make (recommended)
```bash
make
## Credits
This was a team project for CMP(S)303.

- **Phase 2 (Buddy Memory Allocation + `memory.log` integration):** Ahmad ([GitHub](https://github.com/<your-username>))
- **Phase 1 (Scheduler core + IPC + base structure):**
  - Teammate Name 1 ([GitHub](https://github.com/OmarElshereef))
  - Teammate Name 2 ([GitHub](https://github.com/AnAs101AlAa))

> Note: This repository is shared for portfolio/learning purposes.
