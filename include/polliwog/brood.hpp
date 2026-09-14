// polliwog/brood.hpp - design note for the parallel batch engine.
// Stability: Experimental .
//
// brood is a parallel batch squeeze/swell engine. it takes a list of
// jobs (each job is a span of bytes + a format + a level), runs them
// through the one-shot layer on a thread pool, and merges the results
// in job order. it holds no compression state; every job gets its own
// compressor instance through the one-shot layer.
//
// this header is a design note, not an implementation. it exists so
// the design is reviewed against the adoption ladder before code
// lands. the implementation target is froglet (2026.4.0).
//
// adoption step: this is a froglet feature. it cannot move to
// in-progress until the streaming layer is Stable and the jthread
// floor is resolved (clang libc++ lists jthread as partial until
// clang 20; see versions.md).
//
// design points:
//
// - brood calls squeeze() per job, never touches backends directly.
//   this means brood inherits the determinism contract: parallelism
//   changes speed, never bytes.
//
// - the thread pool is std::jthread with a stop_token. the caller
//   controls thread count (default: hardware_concurrency).
//
// - results are merged in job order. a job that fails carries its
//   error; the caller checks per-job, not globally.
//
// - memory: one result buffer per job, caller-visible. brood does
//   not allocate internally beyond the thread pool and the result
//   vector.
//
// what brood is not:
// - not a streaming engine. it calls the one-shot layer per job.
// - not a job scheduler. the caller provides the job list.
// - not a generic parallel_for. it is specifically compression.

#pragma once
