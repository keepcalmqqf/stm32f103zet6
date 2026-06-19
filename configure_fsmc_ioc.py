#!/usr/bin/env python3
"""Add FSMC 16-bit 8080 LCD configuration to stm32f103zet6.ioc."""

import re
from pathlib import Path

IOC = Path("stm32f103zet6.ioc")
BACKUP = Path("stm32f103zet6.ioc.backup")

# Read original content
if not IOC.exists():
    raise FileNotFoundError(f"{IOC} not found")
text = IOC.read_text(encoding="utf-8")
lines = text.splitlines()

# FSMC pins for 16-bit 8080 LCD on Bank4 NE4 / A10
# See HARDWARE_PINOUT.md for the board schematic.
# Data bus
fsmc_pins = {
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

# Parse current Mcu.Pin entries to find max index and existing pins
pin_re = re.compile(r"^Mcu\.Pin(\d+)=(.+)$")
existing_pins = {}
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
new_pins = []
for pin in fsmc_pins:
    if pin not in existing_pins:
        new_pins.append(pin)

# Build replacement dict for simple keys
replacements = {}

# Add FSMC to IP list if not present
if "Mcu.IP4=FSMC" not in text:
    replacements["Mcu.IPNb=4"] = "Mcu.IPNb=5\nMcu.IP4=FSMC"

# Update pin count
old_pins_nb = int(re.search(r"^Mcu\.PinsNb=(\d+)$", text, re.M).group(1))
new_pins_nb = old_pins_nb + len(new_pins)
replacements[f"Mcu.PinsNb={old_pins_nb}"] = f"Mcu.PinsNb={new_pins_nb}"

# Add new Mcu.Pin entries
pin_entries = ""
for i, pin in enumerate(new_pins, start=max_pin_idx + 1):
    pin_entries += f"Mcu.Pin{i}={pin}\n"
replacements[f"Mcu.PinsNb={new_pins_nb}"] = f"Mcu.PinsNb={new_pins_nb}\n{pin_entries.rstrip()}"

# Add per-pin configuration
pin_cfg = ""
for pin, sig in fsmc_pins.items():
    pin_cfg += f"{pin}.Mode={sig}\n"
    pin_cfg += f"{pin}.Signal={sig}\n"

# FSMC peripheral parameters for NOR/PSRAM 1 (LCD mode)
fsmc_params = """FSMC.AccessMode1=FSMC_ACCESS_MODE_A
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

# Update function list to include MX_FSMC_Init
old_func = "ProjectManager.functionlistsort=1-SystemClock_Config-RCC-false-HAL-false,2-MX_GPIO_Init-GPIO-false-HAL-true"
new_func = "ProjectManager.functionlistsort=1-SystemClock_Config-RCC-false-HAL-false,2-MX_FSMC_Init-FSMC-false-HAL-true,3-MX_GPIO_Init-GPIO-false-HAL-true"
replacements[old_func] = new_func

# Apply simple replacements
new_text = text
for old, new in replacements.items():
    if old in new_text:
        new_text = new_text.replace(old, new)
    else:
        print(f"WARNING: key not found: {old}")

# Append pin config and FSMC params at end
new_text = new_text.rstrip() + "\n" + pin_cfg + fsmc_params

# Write back
IOC.write_text(new_text, encoding="utf-8")
print(f"Updated {IOC}: added {len(new_pins)} new pins, FSMC config")
