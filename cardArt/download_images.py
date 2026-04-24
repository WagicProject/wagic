#!/usr/bin/env python3
"""
Download Scryfall card images into ~/.Wagic/sets/{SET}/{ID}.jpg
CSV format: set;id;link (header row included)
"""

import csv
import os
import sys
import time
import urllib.request
import urllib.error
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
CSV_FILE = SCRIPT_DIR / "CardImageLinks.csv"
DEST_ROOT = Path.home() / ".Wagic" / "sets"
WORKERS = 20
TIMEOUT = 15

def download_one(row):
    set_code, mtg_id, url = row
    dest_dir = DEST_ROOT / set_code
    dest_file = dest_dir / f"{mtg_id}.jpg"

    if dest_file.exists():
        return "skip", set_code, mtg_id

    dest_dir.mkdir(parents=True, exist_ok=True)

    try:
        req = urllib.request.Request(url, headers={"User-Agent": "WagicCardDownloader/1.0"})
        with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
            data = resp.read()
        dest_file.write_bytes(data)
        return "ok", set_code, mtg_id
    except Exception as e:
        return "err", set_code, mtg_id, str(e)

def main():
    rows = []
    with open(CSV_FILE, newline="", encoding="utf-8") as f:
        reader = csv.reader(f, delimiter=";")
        next(reader)  # skip header
        for r in reader:
            if len(r) >= 3:
                rows.append((r[0].strip(), r[1].strip(), r[2].strip()))

    total = len(rows)
    done = skipped = errors = 0
    start = time.time()

    print(f"Downloading {total} card images to {DEST_ROOT}")
    print(f"Using {WORKERS} parallel workers\n")

    with ThreadPoolExecutor(max_workers=WORKERS) as executor:
        futures = {executor.submit(download_one, r): r for r in rows}
        for future in as_completed(futures):
            result = future.result()
            if result[0] == "skip":
                skipped += 1
            elif result[0] == "ok":
                done += 1
            else:
                errors += 1
                print(f"  ERROR {result[1]}/{result[2]}: {result[3]}", file=sys.stderr)

            completed = done + skipped + errors
            if completed % 500 == 0 or completed == total:
                elapsed = time.time() - start
                rate = completed / elapsed if elapsed > 0 else 0
                remaining = (total - completed) / rate if rate > 0 else 0
                print(f"  {completed}/{total}  downloaded={done}  skipped={skipped}  errors={errors}"
                      f"  {rate:.0f}/s  ETA {remaining/60:.1f}min")

    elapsed = time.time() - start
    print(f"\nDone in {elapsed/60:.1f}min — downloaded={done}, skipped={skipped}, errors={errors}")

if __name__ == "__main__":
    main()
