## **Step-by-Step Instructions of input/ directory**

### **Step 1. Generate a simple list of process names (`list_{era}.txt`)**
### This step extracts all process names for a given **era**.

```bash
cd /pnfs/knu.ac.kr/data/cms/store/user/jelee/crab_NanoAOD/2022preEE/
find . -maxdepth 1 -type d -not -path '.' | sed 's|^\./||' >> list_2022preEE.txt

cd -  # Return to original directory
cp -r /pnfs/knu.ac.kr/data/cms/store/user/jelee/crab_NanoAOD/2022preEE/list_2022preEE.txt . 
```

### **Step 2. Generate a .list file containing full paths of .root files**
### This step generate full paths for all .root files for each process in a given -e {era}
### Paths are formatted with the dcap:// prefix for remote access.
### run makeList.py with five different options
### -b (default --base_dir='/pnfs/knu.ac.kr/data/cms/store/user/jelee/crab_NanoAOD')
### -l (default --list_dir='.')
### -o (default --output_dir='.')
### -e (default --era='2022preEE') you should change if you get list from other era like 2022postEE ..
```bash
python3 makeList.py -e 2022preEE
```

### A new folder 2022preEE/ is created and each process has its own .list file inside.
### Example file structure:
```bash
head -n 3 2022preEE/WW.list
dcap://cluster142.knu.ac.kr//pnfs/knu.ac.kr/data/cms/store/user/jelee/crab_NanoAOD/2022preEE/WW/WW_TuneCP5_13p6TeV_pythia8/WW/250223_193434/0000/tree_35.root
dcap://cluster142.knu.ac.kr//pnfs/knu.ac.kr/data/cms/store/user/jelee/crab_NanoAOD/2022preEE/WW/WW_TuneCP5_13p6TeV_pythia8/WW/250223_193434/0000/tree_45.root
dcap://cluster142.knu.ac.kr//pnfs/knu.ac.kr/data/cms/store/user/jelee/crab_NanoAOD/2022preEE/WW/WW_TuneCP5_13p6TeV_pythia8/WW/250223_193434/0000/tree_49.root
```

## **Step 3. Split {process}.list files into smaller chunks** 
### Some .list files are too large for batch processing so we split them into smaller parts (e.g., 20 lines per file).
### -b (default --base_dir='./')
### -l (default --list_dir='.')
### -o (default --output_dir='.')
### -e (default --era='2022preEE') you should change if you get list from other era like 2022postEE ..
### -n (default --split_n='20') you should change if you want different num. of splitting ..
```bash
python3 splitList.py -e 2022preEE -n 20
```
### Example structure:
```bash
ls -1 2022preEE/WW/WW_
WW_0.list  
WW_1.list 
WW_2.list  
WW_3.list
```


