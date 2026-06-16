# Maintainer Guide: Updating Documentation Figures

This guide is for NEST developers who need to update the figures in this example's documentation.

## Background

The figures in the documentation are generated from pre-computed reference data stored in `reference_data/`. This allows documentation builds (e.g., on ReadTheDocs) to generate figures without requiring NEST to be installed.

## Directory Structure

```
ignore_and_fire/
├── reference_data/                  # Committed to repo (no NEST needed)
│   ├── iaf_psc_alpha/
│   │   ├── parameters.json
│   │   ├── spikes.dat
│   │   ├── connectivity_presim.dat
│   │   └── connectivity_postsim.dat
│   ├── ignore_and_fire/
│   │   └── (same structure)
│   └── scaling_results.json
├── data/                            # Generated locally (gitignored)
├── figures/                         # Generated locally (gitignored)
├── generate_figures_for_docs.py     # NEST-free figure generation
└── extract_reference_data.py        # Extracts data after simulation
```

## Updating Reference Data

When you modify the simulation code or parameters:

1. **Run the full Snakemake workflow** (requires NEST):
   ```bash
   cd pynest/examples/ignore_and_fire
   snakemake -c4
   ```

2. **Extract reference data** for documentation builds:
   ```bash
   python extract_reference_data.py
   ```

3. **Test figure generation** (no NEST required):
   ```bash
   python generate_figures_for_docs.py
   ```

4. **Commit** the updated `reference_data/` directory.

## ReadTheDocs Integration

Add the following to your `.readthedocs.yaml` to generate figures during doc builds:

```yaml
build:
  os: ubuntu-22.04
  tools:
    python: "3.11"
  jobs:
    pre_build:
      - pip install matplotlib numpy
      - cd pynest/examples/ignore_and_fire && python generate_figures_for_docs.py
```

Alternatively, integrate into your Sphinx `conf.py` or documentation build script.

## What Gets Committed

- `reference_data/` - Yes (small text files, versioned)
- `data/` - No (large simulation outputs)
- `figures/` - No (generated at build time)
