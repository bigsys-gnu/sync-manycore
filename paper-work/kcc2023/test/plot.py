import matplotlib.pyplot as plt
import pandas as pd


DATA_TABLES = [
    ("Uniform", "uniform.csv"),
    ("Zipf (s=0.3)", "zipf-03.csv"),
    ("Zipf (s=0.6)", "zipf-06.csv"),
    ("Zipf (s=0.9)", "zipf-09.csv"),
]

SCALEING = 1_000_000


def formating(plotter):
    def wrapper(*args, **kwargs):
        fig = plt.figure(figsize=(6, 5))
        plt.grid(axis='y')
        plt.ylim(0, 450)
        plt.xlabel("#Threads", fontsize=20)
        plt.ylabel("Operations/sec (Million)", fontsize=20)
        plt.xticks(fontsize=12)
        plt.yticks(fontsize=12)

        plotter(*args, **kwargs)

        plt.legend(loc="upper left", fontsize=17)
        fig.tight_layout()
    return wrapper
        

@formating
def plot(title, csv_name):
    csv = pd.read_csv(csv_name)

    plt.title(title, fontsize=20)
    plt.plot(csv["Threads"], csv["RCU"] / SCALEING, "-*",\
             linewidth=5, label="RCU", markersize=12)
    plt.plot(csv["Threads"], csv["MV-RLU"] / SCALEING, "-o",\
             linewidth=5, label="MV-RLU", markersize=10)


if __name__ == '__main__':
    for title, csv_name in DATA_TABLES:
        plot(title, csv_name)
        plt.savefig(f"{title}.png")
        plt.show()
