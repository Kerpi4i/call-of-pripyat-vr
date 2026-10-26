"""Summarize a CoPVR capture without confusing runtime waits with CPU work."""
import argparse
import csv
import math
from pathlib import Path
from statistics import mean, median


def percentile(values, fraction):
    values = sorted(values)
    pos = (len(values) - 1) * fraction
    lo = math.floor(pos)
    hi = math.ceil(pos)
    return values[lo] + (values[hi] - values[lo]) * (pos - lo)


def read_capture(directory):
    rows = []
    for component in ("engine", "bridge", "renderer"):
        path = directory / f"{component}.csv"
        if not path.exists():
            continue
        with path.open(newline="", encoding="utf-8") as source:
            for row in csv.DictReader(source):
                try:
                    start, duration = float(row["start_qpc_ms"]), float(row["duration_ms"])
                    frame = int(row["frame"])
                except (ValueError, KeyError, TypeError):
                    continue  # A still-running capture can end with a partial line.
                if not math.isfinite(start) or not math.isfinite(duration):
                    continue
                rows.append((component, frame, row["stage"], start, duration))
    return rows


def summarize(directory, skip=0, seconds=None):
    rows = read_capture(directory)
    if not rows:
        raise ValueError("No profiling rows found; enable -Profile and close the game after capture.")
    origin = min(row[3] for row in rows)
    begin = origin + skip * 1000
    end = begin + seconds * 1000 if seconds is not None else math.inf
    groups = {}
    counters = {}
    for component, frame, stage, start, duration in rows:
        if not begin <= start < end:
            continue
        key = (component, stage)
        if duration < 0:
            counters[key] = counters.get(key, 0) + 1
        else:
            groups.setdefault(key, []).append(duration)
    lines = [f"Capture: {directory}", f"Window: skip {skip:g}s, duration {seconds if seconds is not None else 'all'}s",
             "Times are milliseconds; stages are inclusive and must NOT be added together.",
             "GPU elapsed includes command-stream gaps/preemption, not pure GPU busy time.",
             "Engine and bridge frame IDs are separate; correlate by start_qpc_ms.", "",
             f"{'component / stage':44s} {'count':>7s} {'mean':>9s} {'median':>9s} {'p95':>9s} {'max':>9s}"]
    for (component, stage), values in sorted(groups.items()):
        lines.append(f"{component + ' / ' + stage:44s} {len(values):7d} {mean(values):9.3f} "
                     f"{median(values):9.3f} {percentile(values, .95):9.3f} {max(values):9.3f}")
    for (component, stage), count in sorted(counters.items()):
        lines.append(f"{component} / {stage}: {count} unavailable samples")
    if not groups:
        lines.append("No samples in the selected time window.")
    lines += ["", "xr_wait and swapchain waits are synchronization, not CPU computation.",
              "CPU render durations may include driver/GPU backpressure.",
              "xr_period is the runtime's predicted frame budget, not a measured stage.",
              "These traces alone do not measure compositor drops, ASW, or Link encoding latency."]
    return "\n".join(lines) + "\n"


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, nargs="?")
    parser.add_argument("--skip", type=float, default=0, help="Seconds from capture start to omit")
    parser.add_argument("--seconds", type=float, help="Analyze only this many seconds")
    args = parser.parse_args()
    if args.skip < 0 or (args.seconds is not None and args.seconds <= 0):
        parser.error("Use nonnegative skip and positive seconds")
    directory = args.directory
    if directory is None:
        root = Path(__file__).resolve().parents[1] / "reports" / "performance"
        captures = sorted(path for path in root.glob("*") if path.is_dir())
        if not captures:
            parser.error("No captures found")
        directory = captures[-1]
    try:
        result = summarize(directory, args.skip, args.seconds)
    except ValueError as error:
        parser.error(str(error))
    print(result)
    (directory / "summary.txt").write_text(result, encoding="utf-8")
