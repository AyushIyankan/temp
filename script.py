import re

def analyze_perf_logs(file_path):
    core_usage = [0, 0, 0, 0]  # For logical cores 0, 1, 2, 3
    with open(file_path, 'r') as f:
        for line in f:
            match = re.search(r'(\d+)\s+cycles', line)
            if match:
                cycles = int(match.group(1))
                core_idx = int(re.search(r'CPU(\d)', line).group(1))
                core_usage[core_idx] += cycles

    best_core = core_usage.index(min(core_usage))
    return best_core

# Example usage
best_core = analyze_perf_logs('perf_output.log')
print(f"Best core to pin thread: {best_core}")
