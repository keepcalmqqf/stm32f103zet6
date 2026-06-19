#!/usr/bin/env python3
"""Add FSMC 16-bit 8080 LCD configuration to stm32f103zet6.ioc.

This script is meant to be run once after a clean CubeMX code generation,
to inject the FSMC pin and peripheral configuration that is required for
the onboard ILI9486 LCD (16-bit 8080 parallel interface on FSMC Bank4 NE4 / A10).

Usage:
    python scripts/configure_fsmc_ioc.py
    python scripts/configure_fsmc_ioc.py --ioc path/to/project.ioc
"""

import argparse
import re
import shutil
from pathlib import Path

# FSMC pins for 16-bit 8080 LCD on Bank4 NE4 / A10
# See HARDWARE_PINOUT.md for the board schematic.
FSMC_PINS = {
    # Data bus
    "PD14": "FSMC_D0",
    "PD15": "FSMC_D1",
    "PD0":  "FSMC_D2",
    "PD1":  "FSMC_D3",
    "PE7":  "FSMC_D4",
    "PE8":  "FSMC_D5",
    "PE9":  "FSMC_D6",
    "PE10": "FSMC_D7",
    "PE11": "FSMC_D8",
    "PE12": "FSMC_D9",
    "PE13": "FSMC_D10",
    "PE14": "FSMC_D11",
    "PE15": "FSMC_D12",
    "PD8":  "FSMC_D13",
    "PD9":  "FSMC_D14",
    "PD10": "FSMC_D15",
    # Control
    "PG12": "FSMC_NE4",
    "PD4":  "FSMC_NOE",
    "PD5":  "FSMC_NWE",
    "PG0":  "FSMC_A10",
}

# FSMC peripheral parameters for NOR/PSRAM 1 (LCD mode)
FSMC_PARAMS = """FSMC.AccessMode1=FSMC_ACCESS_MODE_A
FSMC.AddressHoldTime1=1
FSMC.AddressSetupTime1=0
FSMC.AsynchronousWait1=FSMC_ASYNCHRONOUS_WAIT_DISABLE
FSMC.BurstAccessMode1=FSMC_BURST_ACCESS_MODE_DISABLE
FSMC.BusTurnAroundDuration1=0
FSMC.CLKDivision1=16
FSMC.DataAddressMux1=FSMC_DATA_ADDRESS_MUX_DISABLE
FSMC.DataLatency1=17
FSMC.DataSetupTime1=26
FSMC.ExtendedAccessMode1=FSMC_ACCESS_MODE_A
FSMC.ExtendedAddressHoldTime1=1
FSMC.ExtendedAddressSetupTime1=0
FSMC.ExtendedBusTurnAroundDuration1=0
FSMC.ExtendedCLKDivision1=16
FSMC.ExtendedDataLatency1=17
FSMC.ExtendedDataSetupTime1=6
FSMC.ExtendedMode1=FSMC_EXTENDED_MODE_ENABLE
FSMC.IPParameters=AccessMode1,AddressHoldTime1,AddressSetupTime1,AsynchronousWait1,BurstAccessMode1,BusTurnAroundDuration1,CLKDivision1,DataAddressMux1,DataLatency1,DataSetupTime1,ExtendedAccessMode1,ExtendedAddressHoldTime1,ExtendedAddressSetupTime1,ExtendedBusTurnAroundDuration1,ExtendedCLKDivision1,ExtendedDataLatency1,ExtendedDataSetupTime1,ExtendedMode1,MemoryType1,NSBank1,NSMemoryDataWidth1,WaitSignal1,WaitSignalActive1,WaitSignalPolarity1,WrapMode1,WriteBurst1,WriteOperation1
FSMC.MemoryType1=FSMC_MEMORY_TYPE_SRAM
FSMC.NSBank1=FSMC_NORSRAM_BANK4
FSMC.NSMemoryDataWidth1=FSMC_NORSRAM_MEM_BUS_WIDTH_16
FSMC.WaitSignal1=FSMC_WAIT_SIGNAL_DISABLE
FSMC.WaitSignalActive1=FSMC_WAIT_TIMING_BEFORE_WS
FSMC.WaitSignalPolarity1=FSMC_WAIT_SIGNAL_POLARITY_LOW
FSMC.WrapMode1=FSMC_WRAP_MODE_DISABLE
FSMC.WriteBurst1=FSMC_WRITE_BURST_DISABLE
FSMC.WriteOperation1=FSMC_WRITE_OPERATION_ENABLE
"""


def resolve_ioc_path(arg: str | None) -> Path:
    """Resolve the .ioc path from CLI argument or project root."""
    if arg:
        path = Path(arg).resolve()
    else:
        script_dir = Path(__file__).resolve().parent
        project_root = script_dir.parent
        path = project_root / "stm32f103zet6.ioc"

    if not path.exists():
        raise FileNotFoundError(f"IOC file not found: {path}")
    return path


def backup_ioc(ioc: Path) -> Path:
    """Create a timestamped backup of the IOC file."""
    backup = ioc.with_suffix(f"{ioc.suffix}.backup")
    shutil.copy2(ioc, backup)
    return backup


def patch_ioc(text: str, ioc: Path) -> None:
    """Patch the IOC file with FSMC configuration."""
    lines = text.splitlines()

    # Parse current Mcu.Pin entries to find max index and existing pins
    pin_re = re.compile(r"^Mcu\.Pin(\d+)=(.+)$")
    existing_pins: dict[str, int] = {}
    max_pin_idx = -1
    for line in lines:
        m = pin_re.match(line)
        if m:
            idx = int(m.group(1))
            name = m.group(2)
            existing_pins[name] = idx
            if idx > max_pin_idx:
                max_pin_idx = idx

    # Determine which pins are new
    new_pins = [pin for pin in FSMC_PINS if pin not in existing_pins]

    # Build simple replacements
    replacements: dict[str, str] = {}

    # Add FSMC to IP list if not present
    if "Mcu.IP4=FSMC" not in text:
        replacements["Mcu.IPNb=4"] = "Mcu.IPNb=5\nMcu.IP4=FSMC"

    # Update pin count
    pins_nb_match = re.search(r"^Mcu\.PinsNb=(\d+)$", text, re.M)
    if not pins_nb_match:
        raise ValueError("Mcu.PinsNb not found in IOC file")
    old_pins_nb = int(pins_nb_match.group(1))
    new_pins_nb = old_pins_nb + len(new_pins)
    replacements[f"Mcu.PinsNb={old_pins_nb}"] = f"Mcu.PinsNb={new_pins_nb}"

    # Add new Mcu.Pin entries
    pin_entries = "\n".join(
        f"Mcu.Pin{i}={pin}" for i, pin in enumerate(new_pins, start=max_pin_idx + 1)
    )
    replacements[f"Mcu.PinsNb={new_pins_nb}"] = (
        f"Mcu.PinsNb={new_pins_nb}\n{pin_entries}"
    )

    # Add per-pin configuration
    pin_cfg = "\n".join(
        f"{pin}.Mode={sig}\n{pin}.Signal={sig}" for pin, sig in FSMC_PINS.items()
    )

    # Update function list to include MX_FSMC_Init
    old_func = (
        "ProjectManager.functionlistsort=1-SystemClock_Config-RCC-false-HAL-false,"
        "2-MX_GPIO_Init-GPIO-false-HAL-true"
    )
    new_func = (
        "ProjectManager.functionlistsort=1-SystemClock_Config-RCC-false-HAL-false,"
        "2-MX_FSMC_Init-FSMC-false-HAL-true,3-MX_GPIO_Init-GPIO-false-HAL-true"
    )
    replacements[old_func] = new_func

    # Apply simple replacements
    new_text = text
    for old, new in replacements.items():
        if old in new_text:
            new_text = new_text.replace(old, new)
        else:
            print(f"WARNING: key not found, skipping: {old}")

    # Append pin config and FSMC params at end
    new_text = f"{new_text.rstrip()}\n{pin_cfg}\n{FSMC_PARAMS}"

    ioc.write_text(new_text, encoding="utf-8")
    print(f"Updated {ioc}: added {len(new_pins)} new pins, FSMC config")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Inject FSMC 16-bit 8080 LCD config into stm32f103zet6.ioc"
    )
    parser.add_argument(
        "--ioc",
        type=str,
        default=None,
        help="Path to stm32f103zet6.ioc (default: ../stm32f103zet6.ioc relative to script)",
    )
    args = parser.parse_args()

    ioc = resolve_ioc_path(args.ioc)

    text = ioc.read_text(encoding="utf-8")
    if "FSMC.NSBank1=FSMC_NORSRAM_BANK4" in text:
        print(f"FSMC config already present in {ioc}, nothing to do.")
        return

    backup = backup_ioc(ioc)
    print(f"Backup created: {backup}")
    patch_ioc(text, ioc)


if __name__ == "__main__":
    main()
