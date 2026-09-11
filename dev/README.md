# polliwog dev index

the dev folder is the project's working memory. it is written to be read
top-to-bottom by a maintainer or an ai agent in one sitting. every file
states its job in the first line. docs are lowercase, direct, plain -
the wetland voice, inherited from tadpole.

## reading order

| order | file                 | job                                                        |
|-------|----------------------|------------------------------------------------------------|
| 0     | ../AGENTS.md         | the operating procedure. the honesty law lives here         |
| 0     | dev/manifest.json    | machine-readable project summary; parse this first          |
| 1     | philosophy.md        | why. the constitution. stable; amend rarely                 |
| 2     | architecture.md      | what. layers, types, backend contract, memory rules         |
| 3     | style.md             | the c++ dialect. module shape, naming, errors, tests        |
| 4     | versions.md          | pinned backend versions, toolchain floor, upgrade policy    |
| 5     | differentiation.md   | why this exists; head-to-head with existing wrappers        |
| 6     | roadmap.md           | growth-stage milestones, triggers, and what is not planned  |
| 7     | github.md            | branches, commits, issues, pull requests, releases, ci      |
| 8     | research-log.md      | provenance for every freshness claim in these docs          |
| 9     | near-term.md         | working memory: the sitting-by-sitting plan for the runway  |
| 10    | decisions.md         | append-only decision log; every non-obvious call, its cost  |

## the name

polliwog is a second word for tadpole. tadpole and polliwog are two
names for the same small creature; compressed and uncompressed are two
forms of the same bytes. the round-trip is in the name. compress
shrinks a frog back into a polliwog; decompress lets it grow. the
wetland theme governs: packages are pond life, versions are growth
stages, the org is the water.

rejected names and why (do not relitigate without new evidence):

- froglet - good, but implies "almost a frog", wrong for a finished form
- frogspawn - great archive metaphor, but reads as a build system
- clam - clamming shut is compression, but the word is closed, not alive
- brood - kept as a component name (the parallel batch engine), too
  narrow for the whole library
- tad - kept as an internal size hint concept only; too small a word
  for a repo name

## glossary

| term      | meaning                                                       |
|-----------|---------------------------------------------------------------|
| squeeze   | compress. the frog becomes a polliwog                         |
| swell     | decompress. the polliwog grows back                           |
| polliwog  | the compressed form of data; also the library name            |
| brood     | the parallel batch engine; a brood is a group of tadpoles      |
| pond      | the test corpus; also the ecosystem of wetland packages        |
| growth stages | milestone names: spawn, hatch, tadpole, froglet, frog      |
| tier      | stability label on every public module: Stable, Growing, Experimental |

## session ritual for ai agents

1. parse manifest.json, then philosophy.md, then architecture.md
2. before proposing a feature, find its place on the adoption ladder
   in philosophy.md; features that cannot name a step are ideas, and
   ideas live in the parking lot section of roadmap.md
3. before proposing an api, check style.md; one way per task
4. every decision that is not obvious earns two or three sentences in
   a decision record the day it is made (github.md, decision records)
5. conflicts resolve upward: style yields to architecture, architecture
   yields to philosophy, philosophy yields to the premise
