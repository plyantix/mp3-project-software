import numpy as np
import csv
from pathlib import Path


def analyze_log_bin():
    """
    Analyze a binary PCM audio file containing a square wave.
    Extracts the left channel from stereo data and calculates amplitude for each period.
    """
    # Path to the log.bin file
    log_file = Path(__file__).parent / 'log.bin'
    
    if not log_file.exists():
        print(f"Error: {log_file} not found")
        return
    
    # Read binary file as 16-bit PCM audio (little-endian)
    with open(log_file, 'rb') as f:
        audio_data = np.frombuffer(f.read(), dtype=np.int16)
    
    # Extract left channel from stereo interleaved data (L0, R0, L1, R1, ...)
    left_channel = audio_data[1 ::2]
    
    print(f"Total samples in left channel: {len(left_channel)}")
    
    # Find zero crossings to detect period boundaries
    zero_crossings = []
    for i in range(1, len(left_channel)):
        # Detect when signal crosses zero (from positive to negative or vice versa)
        if (left_channel[i-1] < 0 and left_channel[i] >= 0) or \
           (left_channel[i-1] >= 0 and left_channel[i] < 0):
            zero_crossings.append(i)
    
    print(f"Found {len(zero_crossings)} zero crossings")
    
    # Calculate amplitude for each period
    # For a square wave, two consecutive zero crossings define one complete period
    periods_data = []
    for i in range(0, len(zero_crossings) - 1, 2):
        period_num = i // 2
        period_start = zero_crossings[i]
        period_end = zero_crossings[i + 1]
        
        # Get the samples in this period
        period_samples = left_channel[period_start:period_end]
        
        # Calculate amplitude as the maximum absolute value
        amplitude = np.max(np.abs(period_samples))
        
        periods_data.append({
            'period': period_num,
            'amplitude': amplitude
        })
    
    print(f"Detected {len(periods_data)} complete periods")
    
    # Write results to CSV
    csv_file = Path(__file__).parent / 'log_analysis.csv'
    with open(csv_file, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=['period', 'amplitude'])
        writer.writeheader()
        writer.writerows(periods_data)
    
    print(f"Analysis complete. Results written to {csv_file}")


if __name__ == '__main__':
    analyze_log_bin()
