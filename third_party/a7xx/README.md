# A7XX Adreno register XML (vendored)

This directory holds the Adreno A7XX perf-counter register definitions used
by `scripts/generate_adreno_definitions.py` to generate the `a7xx.{h,c}`
files under `include/hpc/gpu/adreno/` and `lib/gpu/adreno/`.

## Provenance

- **Upstream source**: `mesa/src/freedreno/registers/adreno/a7xx_perfcntrs.xml`
  in the [freedreno/mesa](https://gitlab.freedesktop.org/freedreno/mesa)
  repository.
- **Vendored on**: 2026-06-27 from local checkout at the time of the A7XX
  support plan (`a7xx_support.plan`).
- **Why not from the envytools submodule?**: the
  `third_party/envytools/` submodule (pinned at `d88cafa3`, 2020 era)
  has no `a7xx.xml` and no A7XX-related commits in its history. The mesa
  freedreno tree is the upstream of record for A7XX register definitions
  until envytools catches up.

## Why this is not inside `third_party/envytools/`

Files placed inside a git submodule are **wiped** by
`git submodule update --init`. Keeping the vendored XML in a
superproject-tracked path here (sibling to the submodule) means the data
source is part of the regular git tree and survives submodule refreshes.

## How to refresh

```sh
# from the repo root, with mesa checked out at ../mesa:
cp ../mesa/src/freedreno/registers/adreno/a7xx_perfcntrs.xml \
   third_party/a7xx/a7xx.xml

# the script does not resolve the XML imports — strip them:
sed -i.bak '/<import file=/d' third_party/a7xx/a7xx.xml
rm third_party/a7xx/a7xx.xml.bak
```

Verify the file still parses cleanly:

```sh
python3 -c "
import sys; sys.path.insert(0, 'scripts')
from generate_adreno_definitions import parse_xml_file
d = parse_xml_file('third_party/a7xx/a7xx.xml')
print(f'{len(d.groups)} groups, {sum(len(c) for c in d.groups.values())} counters')"
```

Expected: `18 groups, 966 counters`.

## What changed vs. the mesa original

- File renamed: `a7xx_perfcntrs.xml` → `a7xx.xml` (so the existing
  `parse_xml_file` in `generate_adreno_definitions.py` derives the
  series name `"a7xx"` from the filename, matching the enum-name
  prefix `a7xx_<group>_perfcounter_select`).
- The three `<import file="..."/>` directives stripped: the script
  parses only `<enum>` elements and does not resolve XInclude-style
  imports, so they are dead weight. The copyright header that the
  imports would normally pull in is replaced by the file's own
  history in git.
- Encoding: kept as UTF-8 XML, no other changes.

## License

The original mesa file carries the Freedreno / Qualcomm copyright header
that the imports referenced. That header is preserved in the git history
of this file and in the upstream source. The library itself is Apache
2.0; the A7XX register data is a derivative of Mesa, which is MIT.
