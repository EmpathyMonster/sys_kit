dir := "exercises"

# Run an exercise
# Usage: just run 001_stdio.c
run name:
    zig cc -Wall -Werror -o {{ name }} {{ dir }}/{{ name }}
    @./{{ name }}
    @rm -f {{ name }}

# List all exercises
list:
    @ls {{dir}}/*.c | sed 's|^{{dir}}/||; s|\.c$$||'

