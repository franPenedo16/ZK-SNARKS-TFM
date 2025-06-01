import json

def calculate_avg_time_values(framework):
    total = 0
    total_percent = 0
    total_samples = 0
    peak_values = []
    peak_percent_values = []

    with open(f"{framework}/stats/execution_times.json", "r") as f:
        data = json.load(f)

    values = [run[f"elapsed_secs"] for run in data]

    average = sum(values) / len(values)

    print(f"----------------------------- ELAPSED TIME -----------------------------")
    print("========================================> In Seconds")
    print(f"Average execution: {average:.2f} s")
    

def calculate_avg_memory_values(framework, type):
    total = 0
    total_percent = 0
    total_samples = 0
    peak_values = []
    peak_percent_values = []

    with open(f"{framework}/stats/memory_{type}_usage.json", "r") as f:
        data = json.load(f)

    for run in data:
        samples = run["samples"]
        values = [sample[f"kib"] for sample in samples]
        values_percent = [sample[f"percent"] for sample in samples]
        
        total += sum(values) / len(values)
        total_percent += sum(values_percent) / len(values_percent)
        total_samples += 1
        
        peak = max(values)
        peak_percent = max(values_percent)
        peak_values.append(peak)
        peak_percent_values.append(peak_percent)

    average_all_samples = total / total_samples
    average_peaks = sum(peak_values) / len(peak_values)

    average_all_samples_percent = total_percent / total_samples
    average_peaks_percent = sum(peak_percent_values) / len(peak_percent_values)

    print(f"----------------------------- {type.upper()}  -----------------------------")
    print("========================================> In KiB")
    print(f"Average of {type}: {average_all_samples:.2f} KiB")
    print(f"Average of the {type} peaks: {average_peaks:.2f} KiB")
    print("========================================> In %")
    print(f"Average of {type}: {average_all_samples_percent:.2f} %")
    print(f"Average of the {type} peaks: {average_peaks_percent:.2f} %")


def main():
    framework = input("Enter the framework ( circom / libsnark / noir / risczero ): ")
    print("")
    print(f"###################################################################################")
    print(f"                                 {framework.upper()}                               ")
    print(f"###################################################################################")
    print("")
    calculate_avg_memory_values(framework, 'pss')
    print("")
    calculate_avg_memory_values(framework, 'rss')
    print("")
    calculate_avg_time_values(framework)
    print("")



if __name__ == "__main__":
    main()