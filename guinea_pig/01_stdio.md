# Introduction to Input/Output

There are two fundamental abstraction layers for I/O operations, buffered and unbuffered. Each operates at a different layer of the operating system.

## Core Distinction

| Aspect | Unbuffered I/O | Buffered I/O |
|--------|---------------|--------------|
| **API Layer** | Syscalls (`read`, `write`) | Stdio (`fread`, `fwrite`) |
| **Handle Type** | `int` file descriptor | `FILE*` stream |
| **Buffering** | None (direct to kernel) | Library-managed buffer |
| **System Calls** | Per operation | Batched, fewer calls |

---

## File Descriptors (Unbuffered I/O)

File descriptors are low-level handles managed directly by the kernel:

```c
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()

int fd = open("file.txt", O_RDWR);  // Returns int fd
char buf[1024];
read(fd, buf, sizeof(buf));         // Direct syscall → kernel
write(fd, buf, n);                  // Direct syscall → kernel
close(fd);                          // Close fd
```

**Characteristics:**
- Each `read()`/`write()` = immediate syscall
- Kernel buffers only (user space has none)
- Fine-grained control
- Works with pipes, sockets, devices—not just files

---

## FILE Streams (Buffered I/O)

`FILE*` streams wrap file descriptors with user-space buffering:

```c
#include <stdio.h>

FILE *fp = fopen("file.txt", "r+");  // Opens internally, wraps fd
char buf[1024];
fread(buf, 1, sizeof(buf), fp);      // Reads from user-space buffer
fwrite(buf, n, 1, fp);               // Writes to buffer first
fclose(fp);                          // Flushes buffer, closes fd
```

**Characteristics:**
- **Standard library** maintains a buffer (typically 4KB–8KB)
- Multiple reads/writes batched → fewer syscalls
- `fflush()` forces buffer flush to kernel
- Automatic buffering on EOF or buffer full

---

## Buffering Types (Stdio)

```c
setvbuf(fp, NULL, _IONBF, 0);   // Unbuffered
setvbuf(fp, buf, _IOFBF, size); // Fully buffered (default for files)
setvbuf(fp, buf, _IOLBF, size); // Line buffered (default for terminals)
```

| Mode | Behavior |
|------|----------|
| `_IONBF` | No buffering (like raw syscalls) |
| `_IOFBF` | Fill buffer, then flush |
| `_IOLBF` | Flush on newline (`\n`) |

---

## Key Differences Summary

| Feature | File Descriptor | FILE Stream |
|---------|----------------|-------------|
| **Syscall Overhead** | High (per-call) | Low (batched) |
| **Control** | Precise, atomic | Buffered semantics |
| **Portability** | POSIX standard | ANSI C standard |
| **Convenience** | Verbose, manual | Format functions (`fprintf`) |
| **Use Case** | Systems programming, IPC | General application I/O |

---

## When to Use Which

- **Use syscalls** when: precise timing matters, working with pipes/devices, avoiding extra buffering layer
- **Use stdio** when: text processing, performance through batching, formatting convenience

Both ultimately reach the kernel—the difference is whether the libc layer adds an intermediate buffer.

# Unix/Linux I/O Architecture: Buffered vs Unbuffered

## Overview Table

| Aspect | Unbuffered I/O (Syscalls) | Buffered I/O (Stdio/FILE*) |
|--------|---------------------------|----------------------------|
| **API** | `read()`, `write()` | `fread()`, `fwrite()` |
| **Handle** | `int fd` (file descriptor) | `FILE* stream` |
| **User Buffer** | None | Yes (typically 4KB–8KB) |
| **Syscall Frequency** | Every operation | Only when buffer fills/flushes |
| **Overhead** | Higher (context switches) | Lower (batched operations) |

---

## Architecture Diagrams

### Unbuffered I/O (Raw Syscalls)

```
┌─────────────────────────────────────────┐
│           User Space                    │
│                                         │
│  ┌───────────────────────────────────┐  │
│  │         Application Code          │  │
│  │    read()/write() direct call     │  │
│  └───────────────────────────────────┘  │
│                      │                  │
│                      ▼ syscall          │
├─────────────────────────────────────────┤
│           Kernel Space                  │
│                                         │
│  ┌───────────────────────────────────┐  │
│  │         Kernel Space              │  │
│  │    VFS + File System Cache        │  │
│  └───────────────────────────────────┘  │
│                      │                  │
│                      ▼                  │
├─────────────────────────────────────────┤
│           Hardware Layer                │
│                                         │
│  ┌───────────────────────────────────┐  │
│  │          Hardware                 │  │
│  │    Disk / Device Driver           │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Flow:** `read()/write()` → **Immediate syscall** → Kernel → Hardware  
**Characteristics:** Direct path, no user buffering, high syscall overhead per operation

---

### Buffered I/O (Stdio/FILE*)

```
┌─────────────────────────────────────────┐
│           User Space                    │
│                                         │
│  ┌───────────────────────────────────┐  │
│  │         Application Code          │  │
│  │    fread()/fwrite() call          │  │
│  └───────────────────────────────────┘  │
│                      │                  │
│                      ▼                   │
│  ┌───────────────────────────────────┐  │
│  │       Stdio Library (libc)        │  │
│  │    ╭─────────────────────────╮    │  │
│  │    │   User-Space Buffer     │    │  │
│  │    │   (4KB – 8KB typical)   │    │  │
│  │    ╰─────────────────────────╯    │  │
│  └───────────────────────────────────┘  │
│                      │                  │
│                      ▼ syscall (periodic)│
├─────────────────────────────────────────┤
│           Kernel Space                  │
│                                         │
│  ┌───────────────────────────────────┐  │
│  │         Kernel Space              │  │
│  │    VFS + Page Cache               │  │
│  └───────────────────────────────────┘  │
│                      │                  │
│                      ▼                  │
├─────────────────────────────────────────┤
│           Hardware Layer                │
│                                         │
│  ┌───────────────────────────────────┐  │
│  │          Hardware                 │  │
│  │    Disk / Device Driver           │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Flow:** `fread()/fwrite()` → **User buffer first** → **syscall (when full/flush)** → Kernel → Hardware  
**Characteristics:** Extra buffering layer, fewer syscalls, better performance for most workloads

---

## Call Flow Comparison

```
Unbuffered I/O:
═══════════════

Application
    │
    ├─ read() / write()  [Per call]
    │         │
    │         ▼ syscall ──────────────────────┐
    │                                       │
Kernel                            HIGH OVERHEAD
    │                                       │
    ▼                                       │
Hardware                              Many context switches
                                      ❌ Less efficient


Buffered I/O:
═════════════

Application
    │
    ├─ fread() / fwrite()  [Multiple calls]
    │         │
    │         ▼
    │    User Buffer
    │    (accumulates data)
    │         │
    │         ▼ syscall ──────────────────────┐
    │         │ (only when buffer full        │
    │         │  or fflush/fclose)            │
Kernel                                LOW OVERHEAD
    │         │                             │
    ▼         │                       Fewer context switches
Hardware      │                         ✅ More efficient
              │
              └── Batched writes/reads
```

---

## Key Points

### Why Buffered I/O Is Faster

1. **Fewer Context Switches**: User↔Kernel transitions are expensive; batching reduces this cost
2. **Larger Transfer Units**: Writing 8KB at once is more efficient than 1KB eight times
3. **CPU Caching**: User-space buffers leverage CPU cache effectively

### When to Use Each

**Unbuffered I/O** (`read()`/`write()`):
- Interactive programs requiring immediate visibility
- Working with pipes, sockets, terminals
- When precise control over timing is critical
- Streaming data where buffering causes latency

**Buffered I/O** (`fread()`/`fwrite()`):
- General file processing
- Text files with line-oriented access
- Performance-critical bulk operations
- When formatting convenience (`fprintf()`, `fscanf()`) is needed

---

## Buffer Control (Stdio)

```c
// Force buffer flush before automatic flush
fflush(fp);

// Set buffering mode
setvbuf(fp, NULL, _IONBF, 0);   // Unbuffered (no buffer)
setvbuf(fp, NULL, _IOFBF, 0);   // Fully buffered (default for files)
setvbuf(fp, NULL, _IOLBF, 0);   // Line buffered (default for terminals)
```

---

## Summary

| Concept | Description |
|---------|-------------|
| **File Descriptor** | Low-level `int` handle, kernel-managed, unbuffered |
| **FILE* Stream** | High-level `struct*` wrapper, libc-managed, buffered |
| **Syscall Boundary** | User↔Kernel transition (expensive operation) |
| **Optimal Strategy** | Use buffered I/O for throughput, unbuffered for precision |

The fundamental difference: **unbuffered I/O crosses the syscall boundary on every operation**, while **buffered I/O batches multiple operations into fewer syscalls** via a user-space intermediary.
