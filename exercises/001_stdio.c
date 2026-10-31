/*
 * EXERCISE 001 — Standard I/O: file descriptors, streams, buffering, and locks
 *
 *
 * PURPOSE
 * -------
 *
 * Get familiar with the two main I/O layers found on Unix-like systems:
 *
 *   1. POSIX file-descriptor API
 *   2. C standard I/O (stdio) API
 *
 * This exercise doesn't provide any code skeleton and is intentionally comments
 * only. Read the explanations, then there is main with small goals in comments.
 * Implement each tiny goal immediately below its instructions.
 *
 * NOTE: POSIX functions are Unix/POSIX interfaces, not ISO (standardized) C.
 * The stdio functions are part of the C standard library.
 */

/* =========================================================================
 * EXPLANATION PART 1 — BUFFERING
 * =========================================================================
 *
 *
 * WHY BUFFERING EXISTS
 * --------------------
 *
 * Moving bytes between your program and the operating system has a cost.
 * Syscalls are actually really fucking expensive. Doing that for every single
 * character you read/write will murder your performance.
 *
 * A buffered IO implementation (stdio) keep buffers in the memory your program
 * is using (program memory space).
 * Output functions can copy bytes into output buffer and send one
 * larger syscall later, either when the buffer fills or when flush() is called.
 * Flush may be called in many different scenarios automatically, more on that
 * later. Input functions can read more than currently necessary into input
 * buffer and keep unconsumed bytes in memory for later calls. Refilling the
 * buffer once its empty, or on peeking past the filled part. (more on that in
 * later exercises) This reduces ammount of syscall I/O operations drastically.
 *
 *
 * TYPICAL STDIO PATH: OUTPUT
 * --------------------------
 *
 *       printf("Hello ");    // just adds it to the buffer, in this example we
 * are assuming the buffer doesnt get full by doing so printf("World\n");
 *            |
 *            v
 *     +---------------------------+
 *     |       written into        |
 *     |    stdio output buffer    |
 *     +---------------------------+
 *            |
 *            | stdout here is line-buffered, and '\n' triggers a flush
 *            v
 *     +---------------------------+
 *     |   POSIX FD write called   |
 *     +---------------------------+
 *            |
 *            v
 *     +---------------------------+
 *     |    SYSCALL into kernel    |
 *     +---------------------------+
 *            |
 *            v
 *      delivered to terminal, pipe, file, etc.
 *
 *
 * TYPICAL STDIO PATH: INPUT
 * -------------------------
 *
 *     // stdin's input buffer is initially empty
 *
 *     int a = getchar();
 *               |
 *               v
 *     +---------------------------+
 *     |  try to read anything in  |
 *     |     stdio input buffer    |
 *     +---------------------------+
 *               |
 *               | The input buffer was empty (shocker)
 *               v
 *     +---------------------------+
 *     |    POSIX FD read called   |
 *     +---------------------------+
 *               |
 *               v
 *     +---------------------------+
 *     |    SYSCALL into kernel    | Kernel has nothing yet, so program is
 * paused (blocking syscall, you may have heard of it)
 *     +---------------------------+
 *
 *
 *     // User has typed "ABC".      (followed by Enter, assuming a terminal in
 * canonical mode)
 *
 *
 *     +---------------------------+
 *     |        TERMINAL           |
 *     |      User types ABC       |
 *     +---------------------------+
 *               |
 *               v
 *     +---------------------------+
 *     |          KERNEL           | stdio passed the address of its internal
 * input buffer to read() which then gave it to the syscall |  Receives terminal
 * input  | kernel copies the received bytes directly into that memory
 *     +---------------------------+
 *               |
 *               | Kernel sees the waiting read syscall, and wakes our program
 * giving it the typed text.
 *               v
 *     +---------------------------+
 *     |   read SYSCALL returned   | with status code giving us more info how
 * the read went
 *     +---------------------------+
 *               |
 *               v
 *     +---------------------------+
 *     |  POSIX FD read returned   | with -1 or length of the buffer filled
 *     +---------------------------+
 *               |
 *               v
 *     +---------------------------+
 *     |      result is now in     |
 *     |     stdio input buffer    | Exactly [A] [B] [C] [\n] bytes
 *     +---------------------------+
 *               |
 *               v
 *     int a = getchar(); // returns 'A'
 *
 *     int b = getchar(); // buffer still contains unread bytes, returns 'B'
 *
 *     int c = getchar(); // buffer still contains unread bytes, returns 'C'
 *
 *     int new_line = getchar(); // buffer still contains unread bytes, returns
 * '\n'
 *
 *     // Any further reads would once again trigger the FD read path
 *
 * NOTE:
 * - getchar() returns an int, not a char, so it can represent EOF.
 * - The example assumes the requested bytes fit in the available buffer.
 * - The call doesnt have to actually be completely handeled by kernel.
 *
 *
 * WHAT "FLUSH" MEANS
 * ------------------
 * Flushing output means asking the C library to pass pending bytes in a
 * stream's output buffer to the underlying destination.
 *
 * Flush is generally called when output stream is closed using fclose().
 * The program terminates normally, aka. returning from main or calling exit().
 *
 * A newline often causes stdout to flush when stdout is connected to an
 * interactive terminal, because it is commonly line-buffered. This is a
 * common default, not a promise for every destination or implementation.
 *
 * Common buffering modes:
 * - Unbuffered: output is passed onward as promptly as the library provides.
 * - Line-buffered: output is commonly passed onward when a newline is written.
 * - Fully-buffered: output is passed onward when the buffer fills, is flushed,
 * or the stream is closed.
 *
 * Typical defaults:
 * - stdout is commonly line-buffered when connected to an interactive terminal.
 * - stdout is commonly fully-buffered when redirected to a file or pipe.
 * - stderr is commonly completely unbuffered.
 * - stdin  is commonly fully buffered.
 *
 * NOTE: The C function setvbuf() can configure a stream's buffering mode.
 * It must be used before other operations are performed on that stream.
 *     (You do not need to call it in this exercise, or almost ever)
 *
 * NOTE: fflush(stdout) explicitly flushes pending output from stdout.
 * It is useful when a prompt has no newline and must appear before the program
 * waits for input.
 *
 *
 * FINAL THOUGHTS
 * --------------
 *
 * Mixing stdio input with raw FD calls on the same underlying input source is
 * dangerous. Its because stdio may already have read ahead, so a later read()
 * can appear to skip characters. Mixing stdio output with raw FD calls on the
 * same destination can also make output ordering surprising, unless you
 * carefully coordinate and flush the stream or are sure the stdio is
 * unbuffered.
 *
 *
 * BEGINNER RULE
 * -------------
 *
 * Use one I/O layer consistently for a given stream or descriptor, for us it
 * will be stdio for now. Do not mix stdio and raw descriptor calls. If you are
 * sad you wont get to play with the FD API. Trust me, you will have plenty fun
 * with it in later exercise, but you must understand the stdio first.
 */

/* =========================================================================
 * EXPLANATION PART 2 — MUTEXES & LOCKING
 * =========================================================================
 *
 *
 * WHAT IS A MUTEX?
 * ----------------
 *
 * A mutex is a mutual-exclusion lock. It protects shared state by making
 * threads take turns: while one thread owns the lock, another thread that
 * needs the same lock must wait. (you should have already encountered them)
 *
 * A FILE stream contains internal state, including buffer positions, error
 * indicators, end-of-file state, and other bookkeeping. Concurrent changes
 * without coordination could corrupt that state.
 *
 *
 * STREAM LOCKING
 * --------------
 *
 * On POSIX systems, stdio functions that operate on a FILE stream lock that
 * stream while performing the individual operation. Common C libraries
 * implement this with a lock associated with each FILE object. The lock is
 * usually invisible to the programmer.
 *
 * Thus, for the normal stdio functions:
 * - They operate on a FILE stream.
 * - They use that stream's buffering setup.
 * - On a normal thread-safe POSIX C library, the individual operation takes the
 * stream lock automatically.
 *
 * This is often described informally as "stdio has a mutex".
 * The precise implementation may use a mutex or another lock mechanism.
 *
 * NOTE: There are often unlocked variants for common functions if you want to
 * bypass the lock for some reason (often performance). Dont depend on them
 * existing tho, its not guaranteed.
 *
 * NOTE: Never depend on its internal implementation being a particular kind of
 * lock.
 *
 *
 * WHAT THE LOCK DOES NOT PROMISE
 * ------------------------------
 *
 * The lock protects an individual stream operation. It does not make a
 * sequence of several calls one indivisible operation.
 *
 * For example, if one thread prints "Hello " and then prints "world" in second
 * print call, another thread may write to the same stream between those two
 * calls "How are you?".
 *
 * Each call s still protected, but the output can look like "Hello How are
 * you?World. Even though one thread intended to print "Hello world!".
 *
 * NOTE: POSIX provides explicit stream-locking functions such as flockfile()
 * and funlockfile() for code that must protect a larger sequence. They are not
 * needed in this exercise.
 *
 *
 * FILE STREAM LOCKS ARE NOT FILE LOCKS
 * ------------------------------------
 *
 * A stdio stream lock protects the C library's FILE object and its internal
 * state. It does not prevent another process from modifying the underlying
 * file. Filesystem-level locking is a separate mechanism, and often necessary.
 * We generally will stay away from files for now tho.
 *
 *
 * RAW FILE-DESCRIPTOR CALLS
 * -------------------------
 *
 * read() and write() do not use a stdio FILE buffer and do not take a stdio
 * FILE-stream lock, because they do not operate on FILE objects. The OS still
 * synchronizes its own kernel data structures, but that is not an
 * application-level mutex protecting your whole message.
 */

// Now to get some use out of our newly gained knowledge
// For functions you dont know, use MANpages or in last resort internet.

int main() {
  // GOAL : Use putchar() to draw a tiny smiley face using individual
  // characters.
  //        (Do not use printf() or puts())

  // GOAL : Print a sentence in two calls using fputs().
  //        (Notice that neither call adds a newline. Add one yourself)

  // GOAL : Print a tiny scoreboard using printf(), including an integer and a
  // character.
  //        (more on that later)

  // GOAL : Print a normal message to stdout and a warning to stderr using
  // fprintf().
  //        (Flush stdout before printing the warning. Notice that they are
  //        separate streams)

  // GOAL : Print "Press Enter to continue..." without a newline.
  //        (Explicitly flush stdout, then wait for the user to press Enter
  //        using getchar())

  // GOAL : Read two characters using getchar() and print them back using
  // putchar().
  //        (Enter both characters on the same line. Consume the remaining
  //        newline too, explain how the buffering works there, try it from your
  //        memory)

  // GOAL : Ask the user for their name, read it using fgets(), and print it
  // back.
  //        (Use a fixed-size character array. Try a name containing spaces)

  // GOAL: Read characters with getchar() until the user presses Enter.
  //       Echo every character using putchar(), and count how many characters
  //       were entered. Do not include the newline in your count. Handle EOF
  //       too. (Remember that a typical terminal echoes characters itself while
  //       typing. Your program will echo them again after Enter)

  // GOAL: Use fgets() with a tiny 8-byte buffer and ask for a sentence longer
  // than 7 characters.
  //       Detect whether the entire line was read. If not, consume the
  //       remaining characters until '\n'. Then read another line to verify
  //       that stdin is clean.

  // GOAL: Read one character, push it back into stdin, and read it again.
  //       Verify that you received the same character without typing twice.
  //       (hint: ungetc(), explain what it does, dont forget to check it
  //       succeded, try to explain when it could fail)

  // You can think of other things to try out.
}
