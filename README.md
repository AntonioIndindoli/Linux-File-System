# Linux File System

A custom file system in C that explores how a hierarchy of files and directories maps to blocks on disk. The project runs in user space, stores its volume in a regular host file, and exposes file-system operations through an interactive shell.

The implementation covers volume initialization, bitmap-based free-space management, directory metadata, path resolution, and an extent-based allocation design. It was developed by a four-person team for CSC 415: Operating Systems in Fall 2022.

**Technologies:** C · Linux/POSIX APIs · GCC · Make · GNU Readline

## Engineering highlights

- **Working directly with disk blocks.** File-system structures are read and written through a logical block address (LBA) interface. Initialization checks a volume signature to choose between formatting a volume and loading existing metadata.
- **Compact free-space tracking.** A bitmap represents each logical block with one bit. Allocation scans for a contiguous free run; bitwise operations reserve and release blocks.
- **Hierarchical namespace.** Directory routines handle absolute and relative paths, `.` and `..` entries, a current working directory, and file metadata. Path parsing distinguishes an existing entry, a missing final component, and an invalid path.
- **Extent-based storage design.** Each directory entry has three extent slots describing runs of blocks. The extent module includes direct addressing and paths for additional levels of indirect tables, exploring how file growth can be represented when data is split across runs.
- **Familiar interfaces.** The public API follows recognizable operations such as `fs_mkdir`, `fs_readdir`, `fs_stat`, and `b_open`. The buffered I/O design introduces a file-control-block table, per-file buffers, and byte offsets.

## Architecture and storage layout

The shell calls the file-system API, which resolves paths and updates directory and allocation metadata. A supplied block-I/O layer provides `LBAread` and `LBAwrite` against the backing volume file. Both functions take a block count and starting LBA, and return the number of **blocks** transferred.

The default configuration requests a **10,000,000-byte volume with 512-byte blocks**. The bitmap is sized for 19,531 logical blocks. The initial layout defined by the source is:

| Logical blocks | Contents | Purpose |
| --- | --- | --- |
| `0` | Volume control block | Records the volume signature, block geometry, bitmap location, and root location. |
| `1–5` | Free-space bitmap | Tracks which blocks are reserved or available. |
| `6–10` | Initial root directory | Holds directory entries, including `.` and `..`. |
| `11+` | Allocatable space | Used by additional directories, file storage, and extent tables. |

The partition layer stores an extra header before logical block `0`. When inspecting a volume with the bundled hexdump utility, physical file block `1` therefore corresponds to file-system LBA `0`.

### Directory metadata

The `directoryEntry` structure combines a 256-byte name field, three extents, a starting location, a byte size, a directory flag, and creation, modification, and access timestamps. New directories reserve five blocks and initialize eight entries: two for `.` and `..`, and six for children.

### Allocation tradeoffs

The bitmap keeps allocation metadata small and makes individual block updates straightforward. Finding a free run requires scanning the map, and a contiguous allocation can fail even when enough free blocks exist in separate regions.

Extents describe each run as a starting block and a count. The design keeps the first runs in the directory entry and uses the third slot for an indirect table when more runs are needed. This separates a file's logical block position from its physical location and illustrates the tradeoff between compact metadata and additional lookup steps.

## Source guide

| File | Responsibility |
| --- | --- |
| [`fsInit.c`](fsInit.c) | Volume formatting, signature checks, loading the root directory and bitmap, and shutdown. |
| [`bitMap.c`](bitMap.c), [`bitMap.h`](bitMap.h) | Free-space representation, contiguous-run search, allocation, and block release. |
| [`directory.c`](directory.c), [`directory.h`](directory.h) | Directory-entry representation, directory initialization, and file-entry creation. |
| [`mfs.c`](mfs.c), [`mfs.h`](mfs.h) | Path parsing, navigation, directory iteration, metadata queries, moves, and removal. |
| [`extents.c`](extents.c), [`extents.h`](extents.h) | Extent allocation and translation from file-relative blocks to LBAs. |
| [`b_io.c`](b_io.c), [`b_io.h`](b_io.h) | Buffered file API, per-file state, and a table of up to 20 open file-control blocks. |
| [`fileInfo.c`](fileInfo.c) | File-information lookup helper. |
| [`fsshell.c`](fsshell.c) | Interactive command dispatch, Readline input, command history, and host-file transfer handlers. |
| [`fsLow.h`](fsLow.h) | Interface to the supplied partition and block-I/O objects. |
| [`Hexdump/`](Hexdump/) | Utility for inspecting the volume's raw bytes. |
| [`MilestoneOne_writeup.pdf`](MilestoneOne_writeup.pdf) | Original milestone writeup. |

For a focused code walkthrough, start with `formatVolume()` in `fsInit.c`, follow `allocateBlocks()` in `bitMap.c`, then read `parsePath()` in `mfs.c`. Together, these show how the project connects disk layout, allocation, and namespace operations.

## Build and run

### Prerequisites

Use a Linux environment with GCC, Make, GNU Readline development headers, and the standard C, math, and pthread libraries. The Makefile selects the supplied `fsLowM1.o` when `uname -m` reports `aarch64`, and `fsLow.o` otherwise; the selected object must match the target architecture and ABI.

> Building from source requires `volumeControlBlock.h` and `fileInfo.h`, which are not included in this checkout.

From the repository root:

```sh
# Build the application from source.
make -B

# Open a separate demo volume using the project's default geometry.
make run RUNOPTIONS="DemoVolume 10000000 512"
```

The executable takes three arguments:

```text
./fsshell <volume-file> <volume-size-in-bytes> <block-size-in-bytes>
```

Using `DemoVolume` keeps the bundled `SampleVolume` available for inspection. The partition layer creates a new backing file if it does not exist; for an existing file, it uses the stored volume geometry. Keep the 512-byte block size and default volume size because several structures use fixed constants.

For toolchains requiring common-symbol compatibility, use `make -B CFLAGS="-g -I. -fcommon"`.

### Shell walkthrough

Example session at the `Prompt >` prompt, creating and navigating a directory before removing it:

```text
help
pwd
ls
md projects
cd projects
pwd
ls
cd /
rm projects
exit
```

Use `exit` to run file-system shutdown, write the in-memory bitmap, and close the partition. The shell's `cd` changes the directory inside the custom volume; host paths used by transfer commands remain relative to the process's host working directory.

### Command reference

| Command | Purpose |
| --- | --- |
| `ls [-a] [-l] [path]` | List directory contents, optionally including hidden entries or metadata. |
| `md <path>` | Create a directory. |
| `cd <path>` / `pwd` | Change or display the current directory. |
| `mv <source> <destination>` | Move or rename an entry. |
| `rm <path>` | Remove a file or an empty directory. |
| `touch <path>` | Open or create a file entry. |
| `history` / `help` | Show command history or built-in help. |
| `exit` | Shut down the file system and close the backing file. |

Additional handlers for `cp`, `cat`, `cp2l`, and `cp2fs` are included as part of the ongoing buffered-I/O work.

## Inspecting the volume

The hexdump utility can be built independently of the file system:

```sh
make -C Hexdump -B
./Hexdump/hexdump --file SampleVolume --start 0 --count 12
```

`--start` and `--count` are measured in 512-byte blocks. This view includes the partition header and the initial file-system metadata region, making it useful for comparing the storage layout with the bytes written to disk.

## Team and attribution

Developed by **Evan Giampaoli, Nathan Rennacker, Hann Zhao, and Antonio Indindoli** as **Team EHNA** for CSC 415: Operating Systems, Fall 2022.

The course provided the low-level partition/block-I/O layer, initial shell driver, API scaffolding, and hexdump utility, credited to Professor Robert Bierman. The team's work builds on that foundation with volume organization, free-space management, directory operations, and file-storage components.
