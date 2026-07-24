"""The two device states every Variant is rendered at.

A State names every input the Studio injects, and all of them are set whenever
there is any doubt about what the emulator holds. That is the whole reset
mechanism: a Variant rendered after a Stress render must not inherit its 4%
battery.

Two things a designer might expect to find here are deliberately absent, because
injecting them does not reach what a watchface reads on this emulator, and a
State that named them would be promising a reset that never happens:

  - Step count and heart rate. `pebble emu-steps` does not move what
    `health_service_sum_today` returns; the complication renders zero whatever
    is injected.
  - Bluetooth. `pebble emu-bt-connection --connected no` does not move either
    `connection_service_peek_pebble_app_connection` or
    `connection_service_peek_pebblekit_connection`; both stay connected.

Both were checked by injecting the state and screenshotting a Variant that
displays it. Until that changes, no Variant should lean on a health or
connectivity complication — it will render the same in both states and the
Stress render will not stress it.
"""

from dataclasses import dataclass
from datetime import datetime


@dataclass(frozen=True)
class State:
    name: str
    when: datetime  # wall-clock time the watch should show, in the host's zone
    battery_percent: int
    charging: bool
    time_format: str  # 12h | 24h


# The single fixed state every Variant in a Batch is rendered at, so that a
# Contact sheet compares designs rather than circumstances.
#
# 10:09 gives four distinct digits, and for analogue Variants an open balanced
# hand position rather than hands overlapping. The date is fixed too — a Batch
# rendered across midnight would otherwise not be comparable with itself — and
# uses a two-digit day, since a single-digit one is unrepresentatively narrow.
CANONICAL = State(
    name='canonical',
    when=datetime(2026, 6, 16, 10, 9, 0),  # Tue Jun 16
    battery_percent=80,
    charging=False,
    time_format='24h',
)

# The worst case, rendered per Variant to expose clipping and contrast failures.
# Never appears on a Contact sheet — it feeds the measurements only.
#
# Wednesday 30 September is the longest weekday and month names in combination.
# 23:58 is the widest digits in combination. 4% battery puts that complication
# in its least forgiving form.
STRESS = State(
    name='stress',
    when=datetime(2026, 9, 30, 23, 58, 0),  # Wed Sep 30
    battery_percent=4,
    charging=False,
    time_format='24h',
)

BY_NAME = {state.name: state for state in (CANONICAL, STRESS)}


def by_name(name):
    try:
        return BY_NAME[name]
    except KeyError:
        raise KeyError('No state named {!r}. Known states: {}'.format(
            name, ', '.join(sorted(BY_NAME)))) from None
