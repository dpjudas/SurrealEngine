# Updating your projects

## Transition from 0.9.3 to 2.0.0 and beyond

### The Basics

* Build your DUMB-using project with warnings that show function pointer mismatches. Treat these warnings extremely carefully, most of them imply porting work.
* `DUMBFILE_SYSTEM` needs 7 pointers instead of 5, and the signatures have changed. Look in include/dumb.h for a quick overview, and in DUMBFILE_SYSTEM.md for a more complete spec. Notable changes include seeking support, file length reporting, and using 64-bit safe seek offsets and file sizes.
* Some functions return `dumb_off_t` instead of `long`. Most of that is backwards compatible unless you have function pointers to these.

### Shortcomings of the 2.0 API

* `DUH.length` is `dumb_off_t`, but this is not the clearest type. It's good that this is 64-bit, but it doesn't measure disk file offsets. `length` measures the total time of the DUH in 65536-ths of a second, where a DUH is a format-agnostic abstraction of a tracked song.
* A sigrenderer is like a slider, or iterator, that moves along a DUH and decodes the music. The *position* of a sigrenderer is stored in `pos`, and this is exposed in the API as `long`. It should ideally have the same type as `DUH.length`.

What's the worst impact of these shortcomings? If your `long` is 32-bit (should only happen on Windows when targeting 32-bit processors), your `pos` might be broken 2^15 seconds into a song, which is about 9 hours. It's really minimal impact, and this bug was already in DUMB 0.9.3. As of DUMB 2.0.0, this is even less of an impact, since the newer timekeeping arrays will automatically keep the `pos` within the actual detected length of a song, dropping back down to 0 again if it loops completely, or slightly higher if the loop is partway into the song. It's not perfect, but at least I tried, right?
