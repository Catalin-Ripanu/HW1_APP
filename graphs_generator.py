import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

serial_times = {"nehalem": 143.125, "haswell": 87.167}

parallel_times_openmp = {
    "nehalem": {
        "batch_size": {
            10: {2: 148.257, 4: 87.084, 8: 41.73, 16: 24.026},
            100: {2: 143.948, 4: 76.816, 8: 43.726, 16: 26.017},
            1000: {2: 161.435, 4: 76.766, 8: 43.328, 16: 25.06},
            "auto": {2: 142.63, 4: 89.555, 8: 42.222, 16: 23.656},
            "static": {2: 141.916, 4: 90.011, 8: 46.101, 16: 22.542},
            # "oversubscribe": {
            #     32: 23.047,
            #     64: 23.14,
            #     128: 23.373,
            #     256: 23.484,
            #     512: 23.507,
            #     1024: 23.797,
            # },
        }
    },
    "haswell": {
        "batch_size": {
            10: {2: 115.28, 4: 60.727, 8: 33.862, 16: 19.299, 32: 10.556},
            100: {2: 118.624, 4: 57.926, 8: 36.297, 16: 24.152, 32: 13.855},
            1000: {2: 107.127, 4: 60.922, 8: 38.177, 16: 20.975, 32: 14.449},
            "auto": {2: 102.299, 4: 61.44, 8: 34.019, 16: 19.407, 32: 10.101},
            "static": {2: 104.413, 4: 61.315, 8: 32.389, 16: 19.29, 32: 10.117},
            # "oversubscribe": {
            #     64: 9.944,
            #     128: 9.984,
            #     256: 10.041,
            #     512: 10.138,
            #     1024: 10.323,
            # },
        }
    },
}

parallel_times_pthreads = {
    "nehalem": {
        "100_000 x 1000": {
            2: 134.794,
            4: 59.74,
            8: 35.38,
            16: 18.943,
            # 32: 20.011,
            # 64: 19.334,
            # 128: 19.306,
            # 256: 20.275,
            # 512: 21.279,
            # 1024: 26.137,
        },
        "50_000 x 1000": {
            2: 67.277,
            4: 29.942,
            8: 16.563,
            16: 10.344,
        },
        "50_000 x 500": {
            2: 30.185,
            4: 17.178,
            8: 9.197,
            16: 5.545,
        },
    },
    "haswell": {
        "100_000 x 1000": {
            2: 89.367,
            4: 51.804,
            8: 28.695,
            16: 14.375,
            32: 9.655,
            # 64: 9.04,
            # 128: 9.228,
            # 256: 9.899,
            # 512: 11.593,
            # 1024: 14.494,
        },
        "50_000 x 1000": {
            2: 44.689,
            4: 25.043,
            8: 14.043,
            16: 11.878,
            32: 5.128,
        },
        "50_000 x 500": {
            2: 23.754,
            4: 12.893,
            8: 7.601,
            16: 3.73,
            32: 2.653,
        },
    },
}

parallel_times_mpi = {
    "nehalem": {
        "100_000 x 1000": {
            2: 77.469,
            4: 39.392,
            8: 20.708,
            16: 16.726,
            32: 17.046,
            64: 18.957,
            128: 20.937,
            512: 61.669,
        },
        "50_000 x 1000": {
            2: 39.148,
            4: 20.440,
            8: 11.173,
            16: 9.261,
            32: 10.114,
            64: 11.059,
            128: 13.234,
            512: 47.971,
        },
        "50_000 x 500": {
            2: 20.687,
            4: 11.249,
            8: 6.484,
            16: 6.012,
            32: 6.145,
            64: 7.217,
            128: 9.897,
            512: 38.113,
        },
    },
    "haswell": {
        "100_000 x 1000": {
            2: 45.781,
            4: 24.748,
            8: 14.793,
            16: 9.613,
            32: 9.501,
            64: 16.242,
            128: 19.344,
            512: 49.467,
        },
        "50_000 x 1000": {
            2: 24.095,
            4: 14.026,
            8: 8.386,
            16: 6.039,
            32: 6.260,
            64: 10.367,
            128: 14.173,
            512: 44.060,
        },
        "50_000 x 500": {
            2: 13.410,
            4: 8.011,
            8: 5.443,
            16: 4.501,
            32: 4.631,
            64: 8.326,
            128: 11.119,
            512: 35.298,
        },
    },
}


for cluster_name in parallel_times_pthreads:
    dfs = []

    for input_size in parallel_times_pthreads[cluster_name]:
        print(input_size, cluster_name)
        df = pd.DataFrame(
            # [(1, serial_times[cluster_name])] +
            list(parallel_times_pthreads[cluster_name][input_size].items()),
            columns=["Threads", "Time"],
        )
        dfs.append((input_size, df))

    # Set Seaborn style
    sns.set(style="whitegrid")

    colors = ["blue", "red", "orange", "green", "purple"]

    for i in range(len(parallel_times_pthreads[cluster_name])):
        ax = sns.lineplot(
            x="Threads",
            y="Time",
            data=dfs[i][1],
            marker="o",
            color=colors[i],
            label=f"Input Size {dfs[i][0]}",
        )

    # Set plot title and labels
    plt.title("Pthreads on " + cluster_name + " Cluster")
    plt.xlabel("Number of Cores")
    plt.ylabel("Number of Seconds")

    plt.axhline(
        y=serial_times[cluster_name],
        xmin=0,
        xmax=1,
        color="black",
        linestyle="dashed",
        label="Serial",
    )

    labels = [
        "Input Size 100_000 x 1000",
        "Input Size 50_000 x 1000",
        "Input Size 50_000 x 500",
        "Serial",
    ]
    handles, _ = ax.get_legend_handles_labels()

    # Slice list to remove first handle
    plt.legend(handles=handles, labels=labels)

    # Show the plot
    plt.show()


for cluster_name in parallel_times_openmp:
    dfs = []

    for batch_size in parallel_times_openmp[cluster_name]["batch_size"]:
        df = pd.DataFrame(
            # [(1, serial_times[cluster_name])] +
            list(parallel_times_openmp[cluster_name]["batch_size"][batch_size].items()),
            columns=["Threads", "Time"],
        )
        dfs.append((batch_size, df))

    # Set Seaborn style
    sns.set(style="whitegrid")

    colors = ["blue", "red", "orange", "green", "purple", "black"]

    for i in range(len(parallel_times_openmp[cluster_name]["batch_size"])):
        ax = sns.lineplot(
            x="Threads",
            y="Time",
            data=dfs[i][1],
            marker="o",
            color=colors[i],
            label=f"Batch Size {dfs[i][0]}",
        )

    # Set plot title and labels
    plt.title("Dynamic Schedule on " + cluster_name + " Cluster")
    plt.xlabel("Number of Cores")
    plt.ylabel("Number of Seconds")

    plt.axhline(
        y=serial_times[cluster_name],
        xmin=0,
        xmax=1,
        color="black",
        linestyle="dashed",
        label="Serial",
    )

    labels = [
        "Batch Size 10",
        "Batch Size 100",
        "Batch Size 1000",
        "Batch Size auto",
        "Batch Size static",
        "Serial",
    ]
    handles, _ = ax.get_legend_handles_labels()

    plt.legend(handles=handles, labels=labels)

    # Show the plot
    plt.show()


for cluster_name in parallel_times_mpi:
    dfs = []

    for input_size in parallel_times_mpi[cluster_name]:
        print(input_size, cluster_name)
        df = pd.DataFrame(
            # [(1, serial_times[cluster_name])] +
            list(parallel_times_mpi[cluster_name][input_size].items())[:4],
            columns=["Processes", "Time"],
        )
        dfs.append((input_size, df))

    # Set Seaborn style
    sns.set(style="whitegrid")

    colors = ["blue", "red", "orange", "green", "purple"]

    for i in range(len(parallel_times_mpi[cluster_name])):
        ax = sns.lineplot(
            x="Processes",
            y="Time",
            data=dfs[i][1],
            marker="o",
            color=colors[i],
            label=f"Input Size {dfs[i][0]}",
        )

    # Set plot title and labels
    plt.title("MPI on " + cluster_name + " Cluster")
    plt.xlabel("Number of Processes")
    plt.ylabel("Number of Seconds")

    plt.axhline(
        y=serial_times[cluster_name],
        xmin=0,
        xmax=1,
        color="black",
        linestyle="dashed",
        label="Serial",
    )
    labels = [
        "Input Size 100_000 x 1000",
        "Input Size 50_000 x 1000",
        "Input Size 50_000 x 500",
        "Serial",
    ]
    handles, _ = ax.get_legend_handles_labels()

    # Slice list to remove first handle
    plt.legend(handles=handles, labels=labels)

    # Show the plot
    plt.show()


# Speedup Nehalem/Haswell 2/4/... threads
# parallel_times_openmp["nehalen"]["batch_size"]["static"]
# parallel_times_pthreads["nehalem"]["100_000 x 1000"]

for cluster_name in serial_times:
    dfs = []
    df = pd.DataFrame(
        [
            (x[0], serial_times[cluster_name] / x[1] * 1 / x[0])
            for x in list(parallel_times_mpi[cluster_name]["100_000 x 1000"].items())[
                :4
            ]
        ],
        columns=["Threads/Processes", "Speedup"],
    )
    dfs.append(("mpi", df))
    df = pd.DataFrame(
        [
            (x[0], serial_times[cluster_name] / x[1] * 1 / x[0])
            for x in list(
                parallel_times_pthreads[cluster_name]["100_000 x 1000"].items()
            )[:4]
        ],
        columns=["Threads/Processes", "Speedup"],
    )
    dfs.append(("pthreads", df))
    df = pd.DataFrame(
        [
            (x[0], serial_times[cluster_name] / x[1] * 1 / x[0])
            for x in list(
                parallel_times_openmp[cluster_name]["batch_size"]["static"].items()
            )[:4]
        ],
        columns=["Threads/Processes", "Speedup"],
    )
    dfs.append(("openmp", df))

    colors = ["blue", "red", "orange", "green", "purple"]
    for i in range(len(dfs)):
        sns.lineplot(
            x="Threads/Processes",
            y="Speedup",
            data=dfs[i][1],
            marker="o",
            color=colors[i % len(colors)],
            label=f"Speedup for {dfs[i][0]}",
        )
    plt.title("Speedup on " + cluster_name + " Cluster")
    plt.show()
