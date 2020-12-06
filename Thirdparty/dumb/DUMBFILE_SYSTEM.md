Specification of DUMBFILE_SYSTEM
================================

DUMB is designed filesystem-agnostic, even though the C standard library
already defines an abstraction over files on a disk. This is useful because
Allegro 4 and 5 define their own abstractions.

To register your own filesystem abstraction with DUMB, you must create an
instance of struct `DUMBFILE_SYSTEM`, fill in your own function pointers
according to the specification below, and call `register_dumbfile_system` on
your instance.

The header `dumb.h` defines `DUMBFILE_SYSTEM` as a struct of function pointers:

```
typedef struct DUMBFILE_SYSTEM
{
    void *(*open)(const char *filename);
    int (*skip)(void *f, dumb_off_t n);
    int (*getc)(void *f);
    dumb_ssize_t (*getnc)(char *ptr, size_t n, void *f);
    void (*close)(void *f);
    int (*seek)(void *f, dumb_off_t n);
    dumb_off_t (*get_size)(void *f);
}
DUMBFILE_SYSTEM;
```

Here, `dumb_off_t` is a signed integer at least 64 bits wide, it is intended
to measure file offsets. The return type `dumb_ssize_t` is a signed integer
exactly as wide as `size_t`, it is intended to store either a `size_t` or a
negative error code. Both `dumb_*_t` are defined in `dumb.h`.

The function pointers `skip` and `getnc` are optional, i.e., you may set
some of these to `NULL` in your struct instance. DUMB will then try to
mimick the missing functions' behavior by calling your `getc` several times.
If DUMB is built with debugging flags, it will assert that all other
functions are not `NULL`. In release mode, DUMB will silently fail.

Your non-`NULL` function pointers must conform to the following specification.



open
----

```
void *(*open)(const char *filename);
```

Open a file for reading.

Arguments:

* `const char *filename`: A normal filename as understood by the operating
    system. Will be opened for reading.

Returns as `void *`:

* the address of a file handle on successfully opening the file.
    DUMB will pass this file handle as argument to other functions of
    the `DUMBFILE_SYSTEM`.

* `NULL` on error during opening the file.

Each file has a *position* internally managed by DUMB. A newly opened file
has a position of 0. Other functions from the `DUMBFILE_SYSTEM` can move
this position around.

DUMB allocates memory for the successfully opened file, and will store opaque
information in that memory, e.g., the DUMB-internal file position. This memory
be freed when DUMB calls `close` on the file's handle. The memory is separate
from your own filesystem implementation: You are responsible for supplying the
data, and DUMB is responsible for storing anything about interpreting that
data.



skip
----

```
int (*skip)(void *f, dumb_off_t n);
```

Advance the position in the file.

Arguments:

* `void *f`: A file handle that `open` returned. Guaranteed non-`NULL`.

* `dumb_off_t n`: Number of bytes to advance in the file. DUMB will only
call this with `n >= 0`. For `n < 0`, the behavior of `skip` is undefined.

Returns as `int`:

* `0` on successfully skipping ahead by `n` bytes.

* `-1` on error.

It is legal to set `skip = NULL` in a `DUMBFILE_SYSTEM`. DUMB will then call
`getc` a total of `n` times to skip ahead in a file. For speed, it is
advisable to supply a proper `skip` implementation.



getc
----

```
int (*getc)(void *f);
```

Read a byte from the file.

Arguments:

* `void *f`: A file handle that `open` returned. Guaranteed non-`NULL`.

Returns as `int`:

* the value of the byte read, on successfully reading one byte.

* `-1` on error.

After a succesful read, DUMB will treat the file as advanced by one byte.



getnc
-----

```
dumb_ssize_t (*getnc)(char *ptr, size_t n, void *f);
```

Read up to the given number of bytes from the file into a given buffer.

* `char *ptr`: The start of a buffer provided by DUMB.

* `size_t n`: The length of the number of bytes to be read.

* `void *f`: A file handle that `open` returned. Guaranteed non-`NULL`.

Returns as `dumb_ssize_t`:

* the number of bytes successfully read, if it was possible to read at least
one byte.

* `-1` on error, when it was not possible to read even a single byte.

This function shall bytes from the file `f` and store them in sequence in the
buffer beginning at `ptr`. It shall read fewer than `n` bytes if end of file
is encountered before `n` bytes could have been read, otherwise it should read
`n` bytes.

It is legal to set `skip = NULL` in a `DUMBFILE_SYSTEM`. DUMB will then call
`getc` a total of `n` times and store the results in its buffer.



close
-----

```
void (*close)(void *f);
```

Closes a file that has been opened before with `open`.

Arguments:

* `void *f`: A file handle that `open` returned. Guaranteed non-`NULL`.

DUMB will deallocate the memory that it used to interpret the file. You are
free to treat your resource however you would like: You may deallocate it, or
keep it around for other things. For example, Allegro 5's implementation
of `close` takes a void pointer and does nothing with it at all.



seek
----

```
int (*seek)(void *f, dumb_off_t n);
```

Jump to an arbitrary position in the file.

Arguments:

* `void *f`: A file handle that `open` returned. Guaranteed non-`NULL`.

* `dumb_off_t n`: The position in the file, relative to the beginning.
    There is no guarantee whether `n >= 0`.

Returns as `int`:

* `0` on successfully seeking in the file.

* `-1` on error.

DUMB will modify its internal position of the file accordingly.

A value of `n < 0` shall set the file into an erroneous state from which no
bytes can be read.



get_size
--------

```
dumb_off_t (*get_size)(void *f);
```

Get the length in bytes, i.e., the position after the final byte, of a file.

Arguments:

* `void *f`: A file handle that `open` returned. Guaranteed non-`NULL`.

Returns as `dumb_off_t`:

* the length of the file in bytes.
