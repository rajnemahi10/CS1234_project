# 🔍 C Dependency Analyzer (ana.c)

A lightweight C-based tool that analyzes `.c` and `.h` files, builds a dependency graph, tracks function relationships, and generates a Makefile automatically.

---

## 🚀 What it does

- Finds all `.c` and `.h` files in a directory  
- Identifies which `.c` files include which headers  
- Extracts function declarations from header files  
- Tracks function definitions and function calls  
- Detects recursion and missing definitions  

### Highlights:
- Marks unused headers  
- Marks declared but undefined functions  
- Generates a dependency graph (`dep1.dot`)  
- Creates a Makefile  

---

## 🎨 Graph Representation

- `.c` files → Blue  
- `.h` files → Yellow  
- Functions → Green  
- Unused / Missing elements → Red  

---

## ⚙️ How it works

- Uses `awk` with `popen` to extract function declarations from `.h` files  
- Parses `.c` files line-by-line to detect function definitions and track function calls  
- Builds a graph using linked structures (`cnode`, `hnode`, `fnode`)  
- Outputs the graph in `.dot` format (Graphviz)

---

## 🛠️ Setup

### Compile

```bash
gcc ana.c -o ana
```

### Install Graphviz

#### macOS
```
brew install graphviz
```

#### Ubuntu / Debian
```
sudo apt install graphviz
```

---

## ▶️ How to run

1. Put all `.c` and `.h` files in one folder  
2. Navigate into the folder:

```
cd test
```

3. Run:

```
./ana
```

---

## 📊 View the graph

```
dot -Tpng dep1.dot -o dep1.png
```

---

## 📦 Output

- `dep1.dot` → Dependency graph  
- `Makefile` → Auto-generated build file  

---

## ⚠️ Important Constraints

This is **not a full C parser**. It works best on clean, simple C code.

- Use simple function declarations in headers  
- Avoid complex signatures and macros  
- Keep function definitions multi-line  
- Avoid function pointers and one-line code  
- Ensure braces are balanced  
- Keep filenames simple (no spaces)  

---

## 🚫 Limitations

- Not a full C parser  
- Macros not handled  
- Comments/strings may confuse parsing  
- Function pointers not supported  
- Complex syntax may fail  

---

## 🧪 Example

### file2.h

```c
int sub (int a,int b);
int stg (int a,int b);
```

### file2.c

```c
#include "file2.h"

int sub (int a,int b)
{
    return a-b;
}

int stg (int a,int b)
{
    return sub(a,b);
}

int main()
{
    stg(5,2);
}
```

---

