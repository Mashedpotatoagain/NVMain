# Integration of gem5 (v23) and NVMain

This repository provides instructions and resources for integrating gem5 (v23.0.0.1) with NVMain.

## Tested Environment

* **Python:** 3.12.3
* **GCC:** 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1)
* **G++:** 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1)
* **SCons:** v4.5.2

## Directory Structure

```text
.
├── disk-image    # disk and binaries
├── gem5
└── NVMain
```

## Prerequisites

Install the dependencies of gem5 and NVMain. Please visit the official [gem5 website](https://www.gem5.org/documentation/general_docs/building) to install the necessary dependencies for your system.

---

## 1. Installing gem5 v23

Clone the gem5 repository and checkout version 23:

```bash
git clone https://github.com/gem5/gem5.git gem5-v23
cd gem5-v23
git checkout v23.0.0.1
```

### Build gem5
Make sure you are in the `gem5-v23` directory and build it using `scons`:

```bash
scons build/X86/gem5.opt -j <no_of_threads>
```

---

## 2. Installing NVMain

Download NVMain from the GitHub repository and ensure the directory is named `NVMain`:

```bash
git clone https://github.com/Mashedpotatoagain/nvmain-gem5v23.git NVMain
cd NVMain
```

### Build NVMain
From the `NVMain` directory, run:

```bash
scons --build-type=fast -j <No_of_Threads_in_your_CPU>
```

### NVMain Test
Verify that NVMain was built successfully by running a test trace:

```bash
./nvmain.fast Config/PCM_ISSCC_2012_4GB.config Tests/Traces/hello_world.nvt 1000000
```

---

## 3. Integration of gem5 and NVMain

Navigate back to the gem5 directory and apply the NVMain patch options:

```bash
cd ../gem5-v23
python3 /path/to/NVMain/patches/gem5/apply_nvmain_v23_options.py /path/to/gem5/gem5-v23
```

Rebuild gem5 with NVMain included as an extra component:

```bash
scons EXTRAS=/path/to/NVMain build/X86/gem5.opt -j$(nproc)
```

---

## 4. Testing gem5-NVMain Integration

From the gem5 root directory, run a test program (like `hello`) to ensure the integration works properly. Make sure your memory configuration points to the correct NVMain config file:

```bash
build/X86/gem5.opt configs/deprecated/example/se.py   -c tests/test-progs/hello/bin/x86/linux/hello   --mem-type=NVMainMemory   --caches   --l2cache   --l1i_size 32kB   --l1d_size 32kB   --l2_size 2MB   --cpu-type=TimingSimpleCPU   --nvmain-config=../NVMain/Config/2D_DRAM_example.config
```
