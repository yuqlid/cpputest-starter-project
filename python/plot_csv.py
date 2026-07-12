# plot_csv.py

import sys
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) < 2:
        print("usage: python plot_csv.py <csv_path>")
        return 1

    csv_path = Path(sys.argv[1])
    df = pd.read_csv(csv_path)

    x = df.columns[0]

    plt.figure()
    for col in df.columns[1:]:
        plt.plot(df[x], df[col], label=col)

    plt.xlabel(x)
    plt.grid(True)
    plt.legend()

    png_path = csv_path.with_suffix(".png")
    plt.savefig(png_path, dpi=150)
    plt.close()

    print(f"saved: {png_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
