# NVMain — gem5 v23 Integration

This repository is based on the original [NVMain](http://nvmain.org/) memory simulator
(originally by Matt Poremba, Penn State University), with modifications for integration
with **gem5 v23**.

## Changes from Original NVMain

- added a script in the patches/gem5/apply_nvmain_v23_options.py
- to apply in the gem5 directory - python3 /path/to/directory/NVmain/patches/gem5/apply_nvmain_v23_options.py ~/gem5-v23

- scons EXTRAS=/path/to/directory/NVmain build/X86/gem5.opt -j$(nproc)

## Building with gem5 v23

```bash
scons EXTRAS=../NVMain build/X86/gem5.opt
```

## Running

```bash
./build/X86/gem5.opt configs/example/se.py \
  --mem-type=NVMainMemory \
  --nvmain-config=../NVMain/Config/PCM_ISSCC_2012_4GB.config
```

## Original NVMain

- Website: http://www.cse.psu.edu/~poremba/nvmain/
- Paper: Please cite the original NVMain paper if using for research.
