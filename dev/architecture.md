# polliwog architecture

what the library is, layer by layer. the why lives in philosophy.md;
the reading rules live in style.md. stability tier on this doc:
Growing - the contracts below are firm, the exact signatures may still
move before frog.

## the layer map

```
        caller code
             |
  +----------v-----------+
  |  convenience layer   |  files.hpp, string overloads (allocating, documented)
  +----------+-----------+
             |
  +----------v-----------+
  |  one-shot layer      |  squeeze() / swell()  span in, span out
  +----------+-----------+  a public loop over the streaming layer
             |
  +----------v-----------+
  |  streaming layer     |  compressor / decompressor: push, pull, finish
  +----------+-----------+
             |
  +----------v-----------+
  |  backend contract    |  concept Backend: checked at compile time
  +-----+-----+-----+----+
        |     |     |
      zlib   zstd   lz4      detail:: C shims, raii handles, no leaks
                              (zlib-ng as a swap-in zlib variant)
             |
        brood.hpp           parallel batch engine; sits beside, not below
```

brood is drawn beside the stack, not inside it: it calls the one-shot
layer per job and merges results. it holds no compression state.

## the backend contract

a backend is a type that satisfies one concept. the concept is the
whole integration surface; a new backend touches nothing above it.

```
concept Backend:
  static format identity            (zlib, gzip, raw_deflate, zstd, lz4_frame, ...)
  level type                        (typed per backend: zlib_level, zstd_level, ...)
  make_squeezer(level) -> handle    returns raii streaming handle
  make_sweller()        -> handle
  push(handle, in, out) -> expected<progress>   progress: bytes_in, bytes_out, done
  flush(handle, out)    -> expected<progress>   sync_flush only until proven need
  finish(handle, out)   -> expected<progress>   ends the stream, emits trailer
  bound(level, in_size) -> size_t              honest upper bound or max()
```

rules:

- handles are move-only raii types; the c api never leaks past detail/.
- a handle is single-threaded. concurrent use of one handle is a
  contract violation; brood gives every job its own.
- push may consume less than the whole input span; progress reports
  what was consumed. no hidden buffering.
- bound() may return max() when the backend cannot bound (lz4 block);
  the one-shot layer then documents its own growth policy.

## memory rules (the whole story)

| function            | allocates                                            |
|---------------------|------------------------------------------------------|
| squeeze(span,span)  | never (backend internal state only, via its handle)  |
| swell(span,span)    | never                                                |
| compressor ctor     | backend state, size documented per backend            |
| string overloads    | the returned string, exactly once                    |
| files.hpp helpers   | the file buffer the caller passes in                  |
| brood jobs          | one result buffer per job, caller-visible             |

the backend internal allocation (zlib window, zstd context) is owned
by the handle and freed by its destructor. every function that
allocates says so in its doc. this table is tested by the allocator
counting tests in the pond corpus.

## span and thread conventions

- inputs are std::span<const std::byte>; outputs are
  std::span<std::byte>. no char*, no unsigned char*, no void* with a
  length parameter, anywhere in the public api.
- one-shot squeeze/swell are pure functions (decision D11): no
  globals, no statics, no hidden shared state. concurrent one-shot
  calls on distinct buffers are safe by contract.
- handles are single-threaded by contract (decision D6/D11): one
  compressor or decompressor object, one thread at a time, no internal
  locking. concurrent use of one handle is
  internal_contract_violation territory if we ever detect it in our
  own tests; in user code it is undefined, and the docs say so
  plainly. brood exists precisely because parallelism belongs above
  this line, where the caller's job structure is visible.
- callbacks: none. the api is call-and-return; anything that looks
  like a sink or source is the caller's loop over push/pull.

## the error type

one taxonomy, like tadpole's TadpoleError:

```
struct error {
  kind        k;        // the enum below
  backend_id  b;        // which backend produced it, or none
  std::uint64_t pos;    // byte position in the stream, if known
  std::string note;     // human text, backend message verbatim when forwarded
};

enum class kind {
  stream_corrupt,       // bad header, bad checksum, truncated
  buffer_too_small,     // caller's output span could not hold the result
  level_unsupported,    // level outside the backend's range
  format_mismatch,      // swelling zlib bytes through a zstd sweller
  backend_failure,      // the backend returned an error this library
                        //   forwards honestly (code in note)
  out_of_memory,
  internal_contract_violation  // our bug; the note asks for a report
};
```

rules:

- errors never throw. std::expected<T, error> everywhere.
- pos is the position where the stream failed, not where the call
  started, and 0 when meaningless.
- note carries the backend's message verbatim when forwarding; our own
  notes are lowercase, plain, no punctuation shouting.
- internal_contract_violation is reserved for library bugs found at
  runtime. it is never returned for bad caller input.

## determinism contract

squeeze of the same input, same format, same level, same backend
version, yields byte-identical output, on every platform we ship:

- gzip headers written with mtime=0 and os=unknown.
- no timestamps, no random dictionary ids.
- brood merges per-job outputs in job order; parallelism changes
  speed, never bytes.

the pond corpus pins this contract across the ci matrix. when a
backend bump threatens it, the bump is a milestone, with the drift
documented and the fixtures updated the same day.

## module map (the index of truth)

| header                | tier          | job                                        |
|-----------------------|---------------|--------------------------------------------|
| polliwog/polliwog.hpp | Stable (hatch)| umbrella; includes everything public        |
| polliwog/squeeze.hpp  | Stable (hatch)| one-shot compress/decompress                |
| polliwog/stream.hpp   | Growing (hatch)| compressor / decompressor streaming types  |
| polliwog/error.hpp    | Stable (hatch)| error + kind taxonomy                       |
| polliwog/format.hpp   | Stable (hatch)| format + typed level enums                  |
| polliwog/brood.hpp    | Experimental (froglet) | parallel batch engine              |
| polliwog/files.hpp    | Growing (froglet) | file conveniences over spans            |
| detail/backend.hpp    | internal      | the Backend concept + backend registry      |
| detail/zlib.hpp       | internal      | zlib/zlib-ng shim                          |
| detail/zstd.hpp       | internal      | zstd shim (tadpole milestone)              |
| detail/lz4.hpp        | internal      | lz4 shim (froglet milestone)               |

a header that is not in this table does not exist. adding a row is a
milestone event, never a drive-by.

## cmake shape

```
polliwog::polliwog          the installed interface target
POLLIWOG_WITH_ZLIB=ON       default; required for now (the only backend)
POLLIWOG_WITH_ZLIB_NG=OFF   swaps the shim to zlib-ng, symbol-mangled
POLLIWOG_WITH_ZSTD=OFF      until tadpole, then ON when found
POLLIWOG_WITH_LZ4=OFF       until froglet, then ON when found
POLLIWOG_WARNINGS=ON        warning-free hard gate on our code
POLLIWOG_SANITIZERS=OFF     ci uses it; users may too
```

backends are found via find_package with a FetchContent fallback that
is pinned to the versions in versions.md. the fallback exists so a
cold clone builds; the docs name it honestly as a convenience, not a
recommendation.

## what is deliberately absent

- iostreams. the streaming layer is push/pull spans; adapting to
  std::ostream is three lines in the caller's code and stays there.
- coroutines. a promise-based reader is a nice future experiment, not
  a dependency of the streaming design. parking lot.
- exceptions. error is a value; exception paths exist only inside
  stdlib containers we already trust.
- runtime backend registration. compile-time options; the concept
  checked at build time. a plugin system would be an opinion core
  does not hold.
