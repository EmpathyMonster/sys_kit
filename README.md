# Systems Programming Learning Materials

This repo holds beginner-to-intermediate learning content on systems programming fundamentals. Think memory management, concurrency, performance, and the tradeoffs you face when you get close to the metal.

Each section stands on its own but builds toward a coherent mental model of how computers actually execute your code.

---

## What's in here

Topics include:

- **Memory layout** — stack vs. heap, alignment, pointers, lifetimes
- **Data structures at the metal** — cache locality, false sharing, allocator basics
- **Concurrency primitives** — mutexes, atomics, lock-free patterns, memory ordering
- **System interfaces** — syscalls, file I/O, processes, signals
- **Performance** — profiling, benchmarking, micro-optimisation traps
- **Safety tradeoffs** — when undefined behaviour matters, how to reason about it

Content is written for people who have basic programming experience and want to understand *how* things work, not just *that* they work.

---

## Prerequisites

You should be comfortable with:

- At least one mainstream language (C, C++, Rust, Go, or similar)
- Basic command-line usage
- Reading error messages and debugging with `gdb`/`lldb` or an IDE

If you're rusty on OS fundamentals (processes, threads, virtual memory), check the appendix or the recommended reading section.

---

## Structure

```
/
├── chapters/          # Main learning content, one directory per topic
├── examples/          # Runnable code snippets referenced in the chapters
├── exercises/         # Practice problems with suggested solutions
├── assets/            # Diagrams, screenshots, other media
└── resources/         # Links, papers, further reading
```

Start at `chapters/README.md` for a suggested order. Jump around if you prefer, but some sections assume earlier ones.

---

## Running the examples

Most examples are self-contained and build with standard toolchains:

```bash
cd examples/<chapter-name>
make        # or cmake / cargo / go build, depending on the subdir
./program
```

If something doesn't compile or behaves oddly, open an issue with your platform and compiler version.

---

## Contributing

We welcome fixes, improvements, and new sections. Read [`CONTRIBUTING.md`](./CONTRIBUTING.md) before sending a PR — it's short.

Quick checklist for submissions:

- ✅ Change is small enough to review comfortably
- ✅ Examples compile and run (or you've noted how to test)
- ✅ Explanations match the code, and vice versa
- ✅ You've checked existing issues to avoid duplicates

---

## Feedback & Corrections

Found a mistake? That's fine — tell us. Open an issue with:

1. The file path and line number if possible
2. The problematic excerpt
3. A suggested correction (optional but helpful)

For typos, feel free to just fix it in a PR.

---

## Licence

Content is licensed under CC BY-SA 4.0. Code examples are MIT licensed unless stated otherwise. Check individual directories if you're unsure.

---

## Credits

Built by contributors including [list people here]. Inspired by talks, blog posts, and textbooks from the broader systems community — full attributions in `resources/`.

---

## Roadmap

What's being worked on next lives in the issues pinned to the top. If you're eager to help, look for the `help wanted` label.

---

## Contact

Questions? Suggestions? Raise an issue, or ping [@your-handle] in discussions if it feels too open-ended for a bug report.

Happy hacking.
