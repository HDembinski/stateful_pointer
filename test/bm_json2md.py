#!/usr/bin/env python
import argparse
import json
import sys

parser = argparse.ArgumentParser()
parser.add_argument("header", nargs="+")

bm = json.load(sys.stdin)

args = parser.parse_args()

columns = []

names = ["Benchmark"]
for d in bm["benchmarks"]:
    names.append("`" + d["name"] + "`")

columns.append(names)

for arg in args.header:
    print arg
    if arg == "c":
        col = ["CPU [%s]" % bm["benchmarks"][0]["time_unit"]]
        for d in bm["benchmarks"]:
            col.append("%.0f" % d["cpu_time"])
    elif arg == "r":
        col = ["Real [%s]" % bm["benchmarks"][0]["time_unit"]]
        for d in bm["benchmarks"]:
            col.append("%.0f" % d["real_time"])
    elif arg == "i":
        col = ["Iterations"]
        for d in bm["benchmarks"]:
            col.append("%i" % d["iterations"])
    columns.append(col)

widths = [max([len(x) for x in col]) for col in columns]

for i,n in enumerate(names):
    names[i] = n + " "*(widths[0]-len(n))

for irow in range(len(names)):
    line = "|" + "|".join([("%%%is" % widths[icol]) % col[irow] for icol, col in enumerate(columns)]) + "|"
    sys.stdout.write(line + "\n")
    if irow == 0:
        line = "|:" + "-"*(widths[0]-1) + "|" + ":|".join(["-"*(w-1) for w in widths[1:]]) + ":|"
        sys.stdout.write(line + "\n")
