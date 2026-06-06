#!/usr/bin/env python3
"""
Apply the gem5 v23 NVMain config-option edits without using git apply.

Run from anywhere:
    python3 /path/to/NVMain/patches/gem5/apply_nvmain_v23_options.py /path/to/gem5
"""

import argparse
from pathlib import Path


OPTIONS_BLOCK = '''\
    # NVMain options. These are used when gem5 is built with
    # EXTRAS=/path/to/NVMain and run with --mem-type=NVMainMemory.
    parser.add_argument(
        "--nvmain-config",
        action="store",
        type=str,
        default="",
        help="NVMain configuration file path",
    )
    parser.add_argument(
        "--nvmain-param",
        action="append",
        default=[],
        metavar="KEY=VALUE",
        help="Override an NVMain configuration key. May be used multiple times.",
    )
    parser.add_argument(
        "--nvmain-atomic",
        action="store_true",
        help="Use NVMain atomic mode",
    )
    parser.add_argument(
        "--nvmain-atomic-latency",
        action="store",
        type=str,
        default=None,
        help="NVMain atomic mode latency, e.g. 30ns",
    )
    parser.add_argument(
        "--nvmain-atomic-variance",
        action="store",
        type=str,
        default=None,
        help="NVMain atomic mode latency variance, e.g. 30ns",
    )
    parser.add_argument(
        "--nvmain-warmup",
        action="store_true",
        help="Warm up NVMain internal cache during atomic accesses",
    )

'''


MEMCONFIG_OPTIONS = '''\
    opt_nvmain_config = getattr(options, "nvmain_config", "")
    opt_nvmain_param = getattr(options, "nvmain_param", []) or []
    opt_nvmain_atomic = getattr(options, "nvmain_atomic", False)
    opt_nvmain_atomic_latency = getattr(options, "nvmain_atomic_latency", None)
    opt_nvmain_atomic_variance = getattr(options, "nvmain_atomic_variance", None)
    opt_nvmain_warmup = getattr(options, "nvmain_warmup", False)

'''


MEMCONFIG_HELPER = '''\
    def apply_nvmain_options(mem):
        if mem.__class__.__name__ != "NVMainMemory":
            return

        if not opt_nvmain_config:
            fatal(
                "--mem-type=NVMainMemory requires "
                "--nvmain-config=/path/to/nvmain.config"
            )

        mem.config = opt_nvmain_config
        mem.atomic_mode = opt_nvmain_atomic
        mem.NVMainWarmUp = opt_nvmain_warmup

        if opt_nvmain_atomic_latency is not None:
            mem.atomic_latency = opt_nvmain_atomic_latency

        if opt_nvmain_atomic_variance is not None:
            mem.atomic_variance = opt_nvmain_atomic_variance

        config_params = []
        config_values = []
        for override in opt_nvmain_param:
            if "=" not in override:
                fatal(
                    "--nvmain-param must be in KEY=VALUE form, got '%s'",
                    override,
                )
            key, value = override.split("=", 1)
            config_params.append(key)
            config_values.append(value)

        mem.configparams = ",".join(config_params)
        mem.configvalues = ",".join(config_values)

'''


def insert_before(text, marker, block, label):
    if block.strip() in text:
        return text, False
    if marker not in text:
        raise RuntimeError(f"Could not find insertion marker for {label}")
    return text.replace(marker, block + marker, 1), True


def insert_after(text, marker, block, label):
    if block.strip() in text:
        return text, False
    if marker not in text:
        raise RuntimeError(f"Could not find insertion marker for {label}")
    return text.replace(marker, marker + block, 1), True


def patch_options(path):
    text = path.read_text()
    marker = '    parser.add_argument("--memchecker", action="store_true")\n'
    text, changed = insert_before(text, marker, OPTIONS_BLOCK, "Options.py")
    if changed:
        path.write_text(text)
    return changed


def patch_memconfig(path):
    text = path.read_text()
    changed_any = False

    text, changed = insert_after(
        text,
        '    opt_xor_low_bit = getattr(options, "xor_low_bit", 0)\n',
        "\n" + MEMCONFIG_OPTIONS,
        "MemConfig.py NVMain option variables",
    )
    changed_any = changed_any or changed

    text, changed = insert_after(
        text,
        "    from m5.util import fatal\n",
        "\n" + MEMCONFIG_HELPER,
        "MemConfig.py NVMain helper",
    )
    changed_any = changed_any or changed

    text, changed = insert_before(
        text,
        "                # Create the controller that will drive the interface\n",
        "                apply_nvmain_options(dram_intf)\n\n",
        "MemConfig.py NVMain option call",
    )
    changed_any = changed_any or changed

    if changed_any:
        path.write_text(text)
    return changed_any


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("gem5_root", help="Path to a gem5 v23.0.0.1 checkout")
    args = parser.parse_args()

    gem5_root = Path(args.gem5_root).resolve()
    options = gem5_root / "configs" / "common" / "Options.py"
    memconfig = gem5_root / "configs" / "common" / "MemConfig.py"

    for path in (options, memconfig):
        if not path.exists():
            raise SystemExit(f"Missing expected gem5 file: {path}")

    changed = {
        str(options): patch_options(options),
        str(memconfig): patch_memconfig(memconfig),
    }

    for path, did_change in changed.items():
        state = "updated" if did_change else "already patched"
        print(f"{state}: {path}")


if __name__ == "__main__":
    main()
