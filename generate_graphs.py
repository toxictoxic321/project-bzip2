#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import os

# 1. Read the results.csv file
try:
    # Defining the columns based on our C code output
    cols = ['File', 'OriginalSize', 'BlockSize', 'Threshold', 'Ratio', 'Time', 'CompressedSize']
    df = pd.read_csv('results/results.csv', names=cols)

    # 2. Extract the latest result
    original_kb = df['OriginalSize'].iloc[-1] / 1024
    compressed_kb = df['CompressedSize'].iloc[-1] / 1024
    ratio = df['Ratio'].iloc[-1]

    # 3. Create the Bar Chart
    plt.figure(figsize=(8, 6))
    sizes = [original_kb, compressed_kb]
    labels = ['Original Size', 'Compressed Size']
    colors = ['#e74c3c', '#2ecc71'] # Red for original, Green for compressed

    bars = plt.bar(labels, sizes, color=colors, width=0.5)
    plt.title(f'BZip2 Compression Performance\nFinal Ratio: {ratio:.2f}x', fontsize=14, fontweight='bold')
    plt.ylabel('File Size (KB)', fontsize=12)

    # Add the exact numbers on top of the bars
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval + (max(sizes)*0.02), 
                 f'{yval:.1f} KB', ha='center', va='bottom', fontweight='bold')

    # 4. Save the graph to the results folder
    output_path = os.path.join('results', 'performance_graph.png')
    plt.savefig(output_path)
    print(f"Success! Graph saved to: {output_path}")

except Exception as e:
    print("Error: Could not generate graph. Make sure results/results.csv exists!")
    print(e)