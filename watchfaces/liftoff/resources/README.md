This directory is deliberately empty.

`pebble build` resolves every `resources.media` path in `package.json` relative
to it, so it has to exist even when a watchface keeps no resources of its own —
this one's font lives in `shared/resources/fonts/` and is reached from here as
`../../../shared/resources/fonts/...`. Without this directory the build fails
inside the SDK's own waf tooling with `'NoneType' object has no attribute
'relpath'`, which says nothing about the cause.

Git does not track empty directories, so this file is what keeps the directory
alive in a fresh clone.
