#!/usr/bin/env python3
"""
Flipper Zero Signal Pack Builder
=================================
Collects, validates, and packages all IR remote and SubGHz signal files
into a single downloadable ZIP archive.

Usage:
    python3 scripts/pack_signals.py [--output <path>] [--validate-only]

Output:
    dist/flipper-signals-pack.zip  (default)

The ZIP contains:
    infrared/<category>/<Brand>.ir
    subghz/<category>/<Name>.sub
    README.txt
"""

import argparse
import os
import sys
import zipfile
import re
from pathlib import Path
from datetime import datetime, timezone

REPO_ROOT = Path(__file__).resolve().parent.parent
IR_ROOT = REPO_ROOT / "assets" / "resources" / "infrared"
SUBGHZ_ROOT = REPO_ROOT / "assets" / "resources" / "subghz"
DIST_DIR = REPO_ROOT / "dist"

IR_FILETYPE = "IR signals file"
IR_VERSION = 1
SUBGHZ_KEY_FILETYPE = "Flipper SubGhz Key File"
SUBGHZ_RAW_FILETYPE = "Flipper SubGhz RAW File"
VALID_SUBGHZ_FILETYPES = {SUBGHZ_KEY_FILETYPE, SUBGHZ_RAW_FILETYPE}

VALID_IR_PROTOCOLS = {
    "NEC", "NECext", "NEC42", "NEC42ext",
    "Samsung32", "RC5", "RC5X", "RC6",
    "SIRC", "SIRC15", "SIRC20",
}

VALID_SUBGHZ_PROTOCOLS = {
    "Princeton", "CAME", "Nice FLORs", "Keeloq",
    "RAW", "Star Line", "Nero Radio", "Nero Sketch",
    "CAME TWEE", "CAME Atomo", "Hormann", "Doitrand",
    "Faac SLH", "Somfy Keytis", "Somfy Telis",
    "BFT Mitto", "An-Motors", "Unilarm",
}

VALID_SUBGHZ_PRESETS = {
    "FuriHalSubGhzPresetOok270Async",
    "FuriHalSubGhzPresetOok650Async",
    "FuriHalSubGhzPreset2FSKDev238Async",
    "FuriHalSubGhzPreset2FSKDev476Async",
    "FuriHalSubGhzPresetMSK99_97KbAsync",
    "FuriHalSubGhzPresetGFSK9_99KbAsync",
}


class ValidationError(Exception):
    pass


def parse_flipper_format(text):
    """Parse Flipper Format key=value pairs from file text."""
    result = {}
    current_key = None
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if ":" in line:
            key, _, value = line.partition(":")
            result[key.strip()] = value.strip()
    return result


def validate_ir_file(path):
    """Validate a .ir file. Returns list of warning strings (empty = OK)."""
    warnings = []
    errors = []
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()

    # Check header
    if not lines:
        raise ValidationError(f"{path}: file is empty")

    filetype_line = lines[0].strip()
    if not filetype_line.startswith("Filetype:"):
        raise ValidationError(f"{path}: missing Filetype header on line 1")
    filetype_val = filetype_line.split(":", 1)[1].strip()
    if filetype_val != IR_FILETYPE:
        raise ValidationError(f"{path}: unexpected Filetype '{filetype_val}', expected '{IR_FILETYPE}'")

    if len(lines) < 2 or not lines[1].strip().startswith("Version:"):
        raise ValidationError(f"{path}: missing Version header on line 2")

    version_val = lines[1].strip().split(":", 1)[1].strip()
    try:
        if int(version_val) != IR_VERSION:
            warnings.append(f"unexpected version {version_val}")
    except ValueError:
        raise ValidationError(f"{path}: Version is not an integer: '{version_val}'")

    # Parse signals
    signal_count = 0
    i = 2
    while i < len(lines):
        line = lines[i].strip()
        i += 1
        if not line or line.startswith("#"):
            continue

        if line.startswith("name:"):
            signal_count += 1
            name_val = line.split(":", 1)[1].strip()
            if not name_val:
                errors.append(f"signal #{signal_count}: empty name at line {i}")
                continue

            # Read signal fields
            fields = {}
            while i < len(lines):
                fline = lines[i].strip()
                if not fline or fline.startswith("#"):
                    i += 1
                    if not fline:
                        break
                    continue
                if fline.startswith("name:"):
                    break
                if ":" in fline:
                    k, _, v = fline.partition(":")
                    fields[k.strip()] = v.strip()
                i += 1

            sig_type = fields.get("type", "")
            if sig_type == "parsed":
                proto = fields.get("protocol", "")
                if proto not in VALID_IR_PROTOCOLS:
                    warnings.append(
                        f"signal '{name_val}': unknown protocol '{proto}'"
                    )
                if "address" not in fields:
                    errors.append(f"signal '{name_val}': missing 'address' field")
                if "command" not in fields:
                    errors.append(f"signal '{name_val}': missing 'command' field")
            elif sig_type == "raw":
                for field in ("frequency", "duty_cycle", "data"):
                    if field not in fields:
                        errors.append(f"signal '{name_val}': missing '{field}' field")
            else:
                errors.append(f"signal '{name_val}': unknown type '{sig_type}'")

    if signal_count == 0:
        warnings.append("no signals found in file")

    if errors:
        raise ValidationError(f"{path}: {'; '.join(errors)}")

    return warnings, signal_count


def validate_subghz_file(path):
    """Validate a .sub file. Returns (warnings, protocol) tuple."""
    warnings = []
    errors = []
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()

    if not lines:
        raise ValidationError(f"{path}: file is empty")

    # Check header
    filetype_line = None
    version_line = None
    for line in lines[:5]:
        stripped = line.strip()
        if stripped.startswith("Filetype:"):
            filetype_line = stripped
        elif stripped.startswith("Version:"):
            version_line = stripped

    if not filetype_line:
        raise ValidationError(f"{path}: missing Filetype header")
    filetype_val = filetype_line.split(":", 1)[1].strip()
    if filetype_val not in VALID_SUBGHZ_FILETYPES:
        raise ValidationError(
            f"{path}: unexpected Filetype '{filetype_val}'"
        )
    if not version_line:
        raise ValidationError(f"{path}: missing Version header")

    fields = {}
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("#") or not stripped:
            continue
        if ":" in stripped:
            k, _, v = stripped.partition(":")
            fields[k.strip()] = v.strip()

    # Validate Frequency
    freq_str = fields.get("Frequency", "")
    try:
        freq = int(freq_str)
        if not (300_000_000 <= freq <= 930_000_000):
            warnings.append(f"unusual frequency {freq} Hz (expected 300-930 MHz)")
    except ValueError:
        errors.append(f"invalid or missing Frequency: '{freq_str}'")

    # Validate Preset
    preset = fields.get("Preset", "")
    if preset not in VALID_SUBGHZ_PRESETS:
        warnings.append(f"unknown Preset '{preset}'")

    # Validate Protocol
    protocol = fields.get("Protocol", "")
    if not protocol:
        errors.append("missing Protocol field")

    if errors:
        raise ValidationError(f"{path}: {'; '.join(errors)}")

    return warnings, protocol


def collect_ir_files(ir_root):
    """Walk IR directory tree and return list of (path, category) tuples."""
    results = []
    for p in sorted(ir_root.rglob("*.ir")):
        # Category = parent dir name, or 'assets' for bundled originals
        category = p.parent.name
        results.append((p, category))
    return results


def collect_subghz_files(subghz_root):
    """Walk SubGHz directory tree and return list of (path, category) tuples."""
    results = []
    for p in sorted(subghz_root.rglob("*.sub")):
        category = p.parent.name
        results.append((p, category))
    return results


def build_readme(ir_files, subghz_files):
    """Generate a README.txt summarizing the signal pack."""
    lines = [
        "Flipper Zero Signal Pack",
        "=" * 60,
        f"Generated: {datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M UTC')}",
        "",
        "HOW TO USE",
        "-" * 40,
        "1. Copy the 'infrared/' folder to /ext/infrared/ on your Flipper Zero SD card.",
        "2. Copy the 'subghz/' folder to /ext/subghz/ on your Flipper Zero SD card.",
        "3. Open the IR or Sub-GHz apps on your Flipper Zero.",
        "4. Browse to the file and select it to transmit.",
        "",
        "INFRARED REMOTES",
        "-" * 40,
    ]

    ir_by_cat = {}
    for path, cat in ir_files:
        ir_by_cat.setdefault(cat, []).append(path.name)

    for cat in sorted(ir_by_cat):
        lines.append(f"  [{cat}]")
        for name in sorted(ir_by_cat[cat]):
            lines.append(f"    - {name}")
    lines.append(f"  Total: {len(ir_files)} IR remote files")
    lines.append("")

    lines += [
        "SUB-GHZ SIGNALS",
        "-" * 40,
    ]
    subghz_by_cat = {}
    for path, cat in subghz_files:
        subghz_by_cat.setdefault(cat, []).append(path.name)

    for cat in sorted(subghz_by_cat):
        lines.append(f"  [{cat}]")
        for name in sorted(subghz_by_cat[cat]):
            lines.append(f"    - {name}")
    lines.append(f"  Total: {len(subghz_files)} SubGHz signal files")
    lines.append("")

    lines += [
        "SUPPORTED IR PROTOCOLS",
        "-" * 40,
        "  NEC, NECext, NEC42, NEC42ext",
        "  Samsung32",
        "  RC5, RC5X, RC6",
        "  SIRC, SIRC15, SIRC20",
        "",
        "SUPPORTED SUBGHZ PROTOCOLS",
        "-" * 40,
        "  Princeton (OOK, 24-bit fixed code)",
        "  And many more via Flipper Zero firmware",
        "",
        "DISCLAIMER",
        "-" * 40,
        "These files are provided for educational, research, and",
        "legitimate device control purposes only. Always ensure",
        "you have permission to control any device you target.",
        "Rolling-code (KeeLoq, HomeLink) devices cannot be replayed.",
        "",
        "SOURCE",
        "-" * 40,
        "Part of the Crashkeyz Flipper Zero firmware repository.",
        "https://github.com/Crashkeyz/flipperzero-firmware",
    ]
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(
        description="Build Flipper Zero IR + SubGHz signal pack ZIP"
    )
    parser.add_argument(
        "--output", "-o",
        default=str(DIST_DIR / "flipper-signals-pack.zip"),
        help="Output ZIP file path (default: dist/flipper-signals-pack.zip)"
    )
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="Only validate files, do not create ZIP"
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Treat warnings as errors"
    )
    args = parser.parse_args()

    print("Flipper Zero Signal Pack Builder")
    print("=" * 50)

    # Collect files
    ir_files = collect_ir_files(IR_ROOT)
    subghz_files = collect_subghz_files(SUBGHZ_ROOT)

    print(f"Found {len(ir_files)} IR files, {len(subghz_files)} SubGHz files")

    # Validate
    total_warnings = 0
    total_errors = 0
    total_signals = 0

    print("\nValidating IR files...")
    ir_valid = []
    for path, category in ir_files:
        try:
            warnings, sig_count = validate_ir_file(path)
            total_signals += sig_count
            status = "OK"
            if warnings:
                total_warnings += len(warnings)
                status = f"WARN ({', '.join(warnings)})"
                if args.strict:
                    total_errors += len(warnings)
            rel = path.relative_to(IR_ROOT)
            print(f"  {rel}: {sig_count} signals - {status}")
            ir_valid.append((path, category))
        except ValidationError as e:
            print(f"  ERROR: {e}")
            total_errors += 1

    print(f"\nValidating SubGHz files...")
    subghz_valid = []
    for path, category in subghz_files:
        try:
            warnings, protocol = validate_subghz_file(path)
            status = f"OK [{protocol}]"
            if warnings:
                total_warnings += len(warnings)
                status = f"WARN: {', '.join(warnings)} [{protocol}]"
                if args.strict:
                    total_errors += len(warnings)
            rel = path.relative_to(SUBGHZ_ROOT)
            print(f"  {rel}: {status}")
            subghz_valid.append((path, category))
        except ValidationError as e:
            print(f"  ERROR: {e}")
            total_errors += 1

    print(f"\nValidation summary:")
    print(f"  IR signals: {total_signals} total across {len(ir_valid)} files")
    print(f"  SubGHz:     {len(subghz_valid)} files")
    print(f"  Warnings:   {total_warnings}")
    print(f"  Errors:     {total_errors}")

    if total_errors > 0:
        print("\nBuild FAILED - fix errors above")
        return 1

    if args.validate_only:
        print("\nValidation PASSED (--validate-only, no ZIP created)")
        return 0

    # Build ZIP
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    print(f"\nBuilding ZIP: {output_path}")
    with zipfile.ZipFile(output_path, "w", zipfile.ZIP_DEFLATED) as zf:
        # Add IR files
        for path, category in ir_valid:
            # Determine archive path relative to infrared root
            rel = path.relative_to(IR_ROOT)
            arcname = f"infrared/{rel}"
            zf.write(path, arcname)
            print(f"  + {arcname}")

        # Add SubGHz files
        for path, category in subghz_valid:
            rel = path.relative_to(SUBGHZ_ROOT)
            arcname = f"subghz/{rel}"
            zf.write(path, arcname)
            print(f"  + {arcname}")

        # Add README
        readme_text = build_readme(ir_valid, subghz_valid)
        zf.writestr("README.txt", readme_text)
        print("  + README.txt")

    size_kb = output_path.stat().st_size / 1024
    print(f"\nBuild SUCCESS")
    print(f"  Output: {output_path}")
    print(f"  Size:   {size_kb:.1f} KB")
    print(f"  Files:  {len(ir_valid)} IR + {len(subghz_valid)} SubGHz + README")
    return 0


if __name__ == "__main__":
    sys.exit(main())
