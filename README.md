# NVMain — gem5 v23 Integration

This repository is based on the original [NVMain](http://nvmain.org/) memory simulator 
(originally by Matt Poremba, Penn State University), with modifications for integration 
with **gem5 v23**.

## Changes from Original NVMain
- changes in the patch file 

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
