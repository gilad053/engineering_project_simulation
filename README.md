# Many-Core Processor Simulator

A single-threaded, discrete-event simulator for modeling cycle-accurate execution of task DAGs on configurable many-core architectures with hierarchical memory systems.

## Overview

This simulator enables computer architects and researchers to evaluate different many-core processor configurations by modeling:

- **Task Execution**: DAG-based task scheduling with configurable policies
- **Memory Hierarchy**: DTCM, caches, memory banks, and main memory
- **Interconnect**: Configurable on-chip network with bandwidth and latency modeling
- **Chiplets**: Multi-chiplet architectures with inter-chiplet communication penalties
- **Performance Analysis**: Detailed statistics on utilization, latency, and conflicts

## Architecture

The simulator uses a discrete-event approach where all state changes occur through events scheduled in a priority queue ordered by simulation time (cycles).

```
┌─────────────────────────────────────────────────────────────┐
│                         Simulator                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ EventQueue   │  │    Config    │  │StatsCollector│      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
         │                    │                    │
         ├────────────────────┼────────────────────┤
         │                    │                    │
┌─────────────┐    ┌─────────────┐    ┌─────────────────────┐
│  TaskGraph  │    │  Scheduler  │    │   MemorySystem      │
│             │    │             │    │  ┌──────┐ ┌──────┐  │
│  ┌──────┐   │    │  Ready Set  │    │  │Cache │ │DTCM  │  │
│  │Tasks │   │    │  Policies   │    │  └──────┘ └──────┘  │
│  └──────┘   │    └─────────────┘    │  ┌────────────────┐ │
└─────────────┘            │           │  │ Interconnect   │ │
                           │           │  └────────────────┘ │
                    ┌──────┴──────┐    │  ┌────────────────┐ │
                    │   Cores     │────┤  │ Memory Banks   │ │
                    │  (Array)    │    │  └────────────────┘ │
                    └─────────────┘    └─────────────────────┘
```

### Key Components

- **Simulator**: Orchestrates event processing and manages global simulation time
- **TaskGraph**: Parses task DAGs and manages task dependencies
- **Scheduler**: Dispatches ready tasks to idle cores using configurable policies
- **Core**: Executes compute and memory operations sequentially
- **MemorySystem**: Routes memory requests through the hierarchy (DTCM → Cache → Banks)
- **Interconnect**: Models on-chip network with bandwidth and contention
- **StatsCollector**: Tracks performance metrics and generates reports

## Building the Simulator

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the simulator
make

# The executable will be in the build directory
./many_core_sim
```

### Build Options

```bash
# Debug build with symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Usage

### Command-Line Interface

```bash
./many_core_sim --config <config.json> --tasks <tasks.csv> --ops <ops.csv>
```

**Arguments**:
- `--config`: Path to JSON configuration file
- `--tasks`: Path to CSV file defining tasks and dependencies
- `--ops`: Path to CSV file defining operations for each task

### Example

```bash
./many_core_sim --config example_config.json --tasks test_tasks.csv --ops test_ops.csv
```

## Input File Formats

### Configuration File (JSON)

The configuration file specifies all runtime parameters for the simulation. See `example_config.json` for a fully commented example.

**Key Parameters**:
- `cores`: Number of processing cores
- `chiplets`: Number of chiplets
- `scheduling_policy`: Task dispatch policy (fifo, round_robin, shortest_ops_first)
- `cache`: Cache configuration (enabled, size, latency, port_limit)
- `dtcm`: DTCM configuration (enabled, base_address, size, latency)
- `memory_banks`: Bank configuration (count, latency, conflict_policy, port_limit)
- `interconnect`: Network configuration (topology, latency, bandwidth)
- `frequency_ghz`: Clock frequency for time conversion

### Tasks File (CSV)

Defines tasks and their dependencies.

**Format**:
```csv
id,name,executions,deps
```

**Fields**:
- `id`: Unique task identifier (integer)
- `name`: Human-readable task name
- `executions`: Number of times to instantiate this task
- `deps`: Semicolon-separated list of predecessor task IDs (empty if no dependencies)

**Example** (`tasks.csv`):
```csv
id,name,executions,deps
1,MatMulTile,256,
2,Postproc,256,1
3,Reduce,1,2
```

This creates:
- 256 instances of MatMulTile (no dependencies)
- 256 instances of Postproc (each depends on one MatMulTile)
- 1 instance of Reduce (depends on all Postproc instances)

### Operations File (CSV)

Defines the sequence of operations for each task.

**Format**:
```csv
task_id,seq_idx,type,cycles,address,rw
```

**Fields**:
- `task_id`: References task ID from tasks.csv
- `seq_idx`: Operation sequence number (0-indexed, must be sequential)
- `type`: Operation type (`compute` or `mem`)
- `cycles`: Compute cycles (required for compute ops, 0 for memory ops)
- `address`: Memory address in hex format (required for mem ops, empty for compute)
- `rw`: Access type (`R` for read, `W` for write; required for mem ops, empty for compute)

**Example** (`ops.csv`):
```csv
task_id,seq_idx,type,cycles,address,rw
1,0,mem,0,0x80000000,R
1,1,compute,200,,
1,2,mem,0,0x80004000,W
2,0,compute,50,,
2,1,mem,0,0x90000000,R
```

Task 1 performs:
1. Read from address 0x80000000
2. Compute for 200 cycles
3. Write to address 0x80004000

Task 2 performs:
1. Compute for 50 cycles
2. Read from address 0x90000000

## Output

### Console Output

The simulator prints a formatted summary of key performance metrics:

```
=== Simulation Results ===
Makespan: 125000 cycles (0.0000625 seconds)

Core Utilization:
  Core 0: 85.2%
  Core 1: 82.7%
  ...

Task Statistics:
  Average Latency: 450.5 cycles
  Average Wait Time: 25.3 cycles

Memory Hierarchy:
  DTCM Hits: 5000
  Cache Hits: 3000
  Cache Misses: 1000
  Main Memory Accesses: 1000

Interconnect Utilization: 45.0%

Conflicts:
  Bank Conflicts: 150
  Cache Port Conflicts: 50
  Bank Port Conflicts: 75
  Intra-Chiplet: 200
  Inter-Chiplet: 75
```

### JSON Output

Detailed statistics are written to `stats.json`:

```json
{
  "makespan_cycles": 125000,
  "makespan_seconds": 0.0000625,
  "core_utilization": [0.85, 0.82, 0.88, ...],
  "avg_task_latency_cycles": 450.5,
  "avg_task_wait_cycles": 25.3,
  "memory_accesses": {
    "dtcm_hits": 5000,
    "cache_hits": 3000,
    "cache_misses": 1000,
    "main_memory_accesses": 1000
  },
  "interconnect_utilization": 0.45,
  "conflicts": {
    "bank_conflicts": 150,
    "cache_port_conflicts": 50,
    "bank_port_conflicts": 75,
    "intra_chiplet_conflicts": 200,
    "inter_chiplet_conflicts": 75
  }
}
```

## Configuration Examples

### Minimal Configuration

Simple 2-core system with basic memory:

```json
{
  "cores": 2,
  "chiplets": 1,
  "scheduling_policy": "fifo",
  "cache": {"enabled": false},
  "dtcm": {"enabled": false},
  "memory_banks": {
    "count": 2,
    "service_latency_cycles": 10,
    "bank_index_function": "addr_mod_n",
    "conflict_policy": "serialize",
    "port_limit": 1
  },
  "interconnect": {
    "topology": "bus",
    "base_latency_cycles": 5,
    "link_width_bytes_per_cycle": 8
  },
  "chiplet": {"remote_penalty_cycles": 0},
  "frequency_ghz": 1.0
}
```

### High-Performance Configuration

16-core system with full memory hierarchy:

```json
{
  "cores": 16,
  "chiplets": 2,
  "scheduling_policy": "shortest_ops_first",
  "cache": {
    "enabled": true,
    "size_bytes": 65536,
    "hit_latency_cycles": 1,
    "port_limit": 8
  },
  "dtcm": {
    "enabled": true,
    "base_address": "0x80000000",
    "size_bytes": 32768,
    "latency_cycles": 1
  },
  "memory_banks": {
    "count": 16,
    "service_latency_cycles": 40,
    "bank_index_function": "xor_fold",
    "conflict_policy": "queue",
    "port_limit": 4
  },
  "interconnect": {
    "topology": "mesh",
    "base_latency_cycles": 8,
    "link_width_bytes_per_cycle": 16
  },
  "chiplet": {"remote_penalty_cycles": 15},
  "frequency_ghz": 2.5
}
```

## Scheduling Policies

### FIFO (First-In-First-Out)
Tasks are dispatched in the order they become ready. Simple and predictable.

### Round Robin
Tasks are distributed evenly across cores in a round-robin fashion. Balances load across cores.

### Shortest Ops First
Prioritizes tasks with fewer remaining operations. Can reduce average task latency.

## Memory Hierarchy

### Request Flow

1. **DTCM Check**: If enabled and address in range → fast path (1-2 cycles)
2. **Cache Check**: If enabled → hit (2-5 cycles) or miss
3. **Interconnect**: Route to appropriate memory bank
4. **Memory Bank**: Service request with potential conflicts (40-100 cycles)

### Bank Conflict Policies

- **Serialize**: Queue all conflicting requests, service one at a time
- **Queue**: Allow parallel service up to port limit
- **Extra Delay**: Add fixed penalty for each conflict

## Performance Analysis

### Key Metrics

- **Makespan**: Total simulation time (critical path length)
- **Core Utilization**: Percentage of time each core is busy
- **Task Latency**: Time from ready to completion
- **Task Wait Time**: Time from ready to dispatch (scheduling delay)
- **Memory Tier Hits**: Distribution of accesses across hierarchy
- **Conflicts**: Bank, cache, and interconnect contention

### Optimization Strategies

1. **Reduce Memory Latency**: Enable DTCM for frequently accessed data
2. **Minimize Conflicts**: Increase bank count or adjust bank index function
3. **Improve Scheduling**: Try different policies for your workload
4. **Balance Chiplets**: Distribute tasks to minimize inter-chiplet traffic

## Troubleshooting

### Common Issues

**Error: "File not found"**
- Verify all file paths are correct
- Use absolute paths or ensure files are in the working directory

**Error: "Invalid configuration"**
- Check JSON syntax (no trailing commas, proper quotes)
- Verify all required fields are present
- Ensure numeric values are positive

**Error: "Cycle detected in task graph"**
- Review task dependencies in tasks.csv
- Ensure no circular dependencies exist

**Simulation hangs or runs forever**
- Check that all tasks have finite operations
- Verify task dependencies are satisfiable
- Ensure no deadlocks in the DAG

## Examples

The repository includes several example workloads:

- `test_tasks.csv` / `test_ops.csv`: Small test workload
- `test_config.json`: Basic configuration for testing

## License

[Add your license information here]

## Contributing

[Add contribution guidelines here]

## Contact

[Add contact information here]
