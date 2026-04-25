# BZip2 Compression Project

## Complete Feature Description
This project is a custom version of the BZip2 compression tool written in C. It reads a file, processes the data to group similar characters together, and shrinks the file size. 

**Current Features (Stage 1):**
* **Block Manager:** Divides large files into smaller chunks for safe processing.
* **Enhanced RLE-1:** Uses a smart "threshold" rule. It only compresses characters if they repeat many times in a row (configurable in `config.ini`). This stops text files from accidentally getting bigger.
* **Suffix Array BWT:** Uses an advanced, high-speed sorting method (`qsort` with Prefix-Doubling) to perform the Burrows-Wheeler Transform much faster than the standard method.
* **Configurable Settings:** Reads block size and threshold rules from a `config.ini` file without needing to change the code.

---

## Implementation Details
**Stage 1: Preprocessing & Sorting**
The program first reads the `config.ini` file to set the rules. Then, it runs the data through Run-Length Encoding (RLE-1) to shrink any long sequences of identical characters. Next, it passes that data into the Burrows-Wheeler Transform (BWT). The BWT uses a Suffix Array to alphabetically sort the data in a cyclic way, which groups similar characters together and prepares the file for deeper compression in the next stages.

---

## Build Instructions

**For Windows (Visual Studio):**
1. Download the code and open the folder in Visual Studio.
2. Make sure all `.c` files in the `src/` folder are included in your project.
3. Build the solution and run the program (Press F5).

**For Linux (GCC Terminal):**
Open your terminal in the project folder and run this command to compile all the C files together:
`gcc -I./include src/*.c -o bzip2_project`
Then run it with:
`./bzip2_project`

---

## Usage Examples
To run the compression, you must follow these simple steps:
1. Take any file you want to compress and rename it to `input.txt`.
2. Place `input.txt` and `config.ini` in the same folder as the running program.
3. Run the program. 

**What the program will do:**
* It will print the progress on the terminal screen.
* It will create a binary file named `compressed`.
* It will create a file named `restored_output.txt` (This is the decompressed file used to prove the code works perfectly without losing data).
* It will add a new line to `results/results.csv` to keep a history of your file size, ratio, and speed.

---

## Performance Results and Graphs
These are the verified results for Stage 1 running on the standard benchmark text file:

## Performance Results (Canterbury Corpus)
These are the verified Stage 1 results across different file types using a threshold of 6. All files successfully passed the Encode -> Decode round-trip verification.

| File Name | Type | Original Size | Compressed Size | Ratio | Time |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `alice29.txt` | English Text | 149 KB (148,481 bytes) | ~147 KB (146,804 bytes) | 1.0114 | ~1.00s |
| `asyoulik.txt` | English Play | 122 KB (125,179 bytes) | ~122 KB (125,116 bytes) | 1.0005 | ~0.73s |
| `lcet10.txt` | Technical Doc | 417 KB (426,754 bytes) | ~407 KB (417,124 bytes) | 1.0231 | ~3.19s |
| `plrabn12.txt` | Poetry/Text | 471 KB (481,861 bytes) | ~470 KB (481,271 bytes) | 1.0012 | ~3.46s |

*(Note: Final compression ratios will be significantly higher upon the completion of Stage 2 Move-to-Front and Stage 3 Huffman coding).*


*Note: Graphs will be added upon the completion of Stage 3 

## Team Members and Contributions
* Eliya Haider (22L-6970): (Implemented Config Parser, Block Manager, Enhanced RLE-1, Suffix Array BWT, and File I/O).\
---

