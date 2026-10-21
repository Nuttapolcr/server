# Lumi Rainbow female Soul Linker sprite fix

This client patch restores the missing ACT file requested by the client:

`data\sprite\로브\lumi_rainbow\여\소울링커_여.act`

The file is copied byte-for-byte from the canonical
`data\sprite\로브\lumi_rainbow\lumi_rainbow.act` entry in `rdata.grf`.
All 252 ACT entries currently present for `lumi_rainbow` have the same SHA-256
digest, so this is the matching animation data rather than a substitute from a
different garment.

`../lumi_rainbow_female.grf` contains the same file at the client-facing path.
Load that GRF after `rdata.grf`, or merge its single entry into the client patch
GRF.
