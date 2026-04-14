# 🔍 C Dependency Analyzer (ana.c)

This is a C-based tool that analyzes `.c` and `.h` files and builds a dependency graph between them. It also tracks function calls and generates a Makefile automatically.

---

## 🚀 What it does

* Finds all `.c` and `.h` files in the directory
* Links which `.c` files include which headers
* Extracts function declarations from headers
* Tracks function calls inside definitions
* Detects things like recursion and missing definitions
* Generates a dependency graph (`dep1.dot`)
* Creates a basic Makefile

---

## ⚙️ How it works (rough idea)

* Uses `awk` + `popen` to extract function signatures
* Reads `.c` files line by line to find function definitions and calls
* Builds a graph using linked structures (`cnode`, `hnode`, `fnode`)
* Outputs everything into a `.dot` file for visualization

---

## 🛠️ Setup

### Compile

```bash
gcc ana.c -o ana
```

### Install Graphviz (for visualization)

#### macOS

```bash
brew install graphviz
```

#### Ubuntu / Debian

```bash
sudo apt install graphviz
```

---

## ▶️ How to run

1. Put all your `.c` and `.h` files inside a folder (for example `test`)
2. Go into that folder:

```bash
cd test
```

3. Run:

```bash
./ana
```

---

## 📊 View the graph

```bash
dot -Tpng dep1.dot -o dep1.png
```

This will generate an image of the dependency graph.

---

## 📦 Output

* `dep1.dot` → graph file
* `Makefile` → auto-generated build file

---
## ⚠️ Constraints (Important)

* All functions used should have a declaration in a `.h` file
* Function definitions should roughly match the declaration (same name + return type)
* Function format should be simple (like `int func(...)`) — complex cases may not work
* Function calls should be in normal form (`foo(a, b)`) — function pointers/macros not handled
* Braces `{}` should be properly balanced
* Avoid writing full functions in one line
* All `.c` and `.h` files must be in the same directory
* Assumes one definition per function
* Heavy macros / advanced C syntax may not be detected properly

👉 Basically, this works best on clean, standard C code — not very complex or heavily macro-based code.

---

## 🧪 Examples

### ✅ Works well

```c
// file.h
int add(int a, int b);
```

```c
// file.c
#include "file.h"

int add(int a, int b) {
    return a + b;
}

int main() {
    int x = add(2, 3);
}
```

---

### ❌ May not work properly

**1. No header declaration**

```c
int add(int a, int b) {   // not declared in .h
    return a + b;
}
```

**2. Complex signature**

```c
static inline int add(int a, int b);
```

**3. Function pointer call**

```c
(*add)(2, 3);
```

**4. Everything in one line**

```c
int add(int a, int b) { return a + b; }
```

**5. Macro usage**

```c
#define CALL(x,y) add(x,y)
CALL(2,3);
```

---

