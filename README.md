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

## What it does

- finds all `.h` files
- stores functions written in header files
- finds all `.c` files
- checks which headers each c file includes
- checks which functions call other functions
- checks main function calls also
- marks unused headers red
- marks functions declared but not defined red
- makes a makefile

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
Already done in main
---

## 📦 Output

- `dep1.dot` → Dependency graph
- `dep1.png`
- `Makefile` → Auto-generated build file  

---


## Important constraints

This is not a full C parser. It works only for simple format.

### Function declaration format

Use this type of format in `.h`:

```c
int add (int a,int b);
int sub (int a,int b);
```

Do not write complicated return types like:

```c
unsigned int add (int a,int b);
static int add (int a,int b);
```

Because I am using simple awk and space splitting, so function name should basically come as second word.

### Header include format

Use:

```c
#include "file1.h"
```

Not:

```c
#include <file1.h>
```

### File names

Do not use spaces in file names.

Good:

```text
file1.c
file2.h
```

Bad:

```text
my file.c
```

### Function definitions

Try to keep definition same style as header:

```c
int add (int a,int b)
{
    return a+b;
}
```

### Main format

This works:

```c
int main()
{
    stg(1,2);
}
```

This may miss the call:

```c
int main() { stg(1,2); }
```

because main processing starts scanning calls from next lines.

Also main checking is basic, it searches for `"main"` in line, so weird names can confuse it.

### Function calls

Function calls are found by searching for `(` and reading backwards.

Works:

```c
add(a,b);
sub(a,b);
```

Can be wrong:

```c
// add(a,b);
printf("add(a,b)");
```

because comments and strings are not properly ignored.

### Braces

Braces should be balanced.

```c
int add (int a,int b)
{
    return a+b;
}
```

If `{` and `}` are wrong then function body scanning can break.

---

## Limitations

- not a real C parser
- macros not handled
- comments/strings can confuse function call detection
- long lines may break because max line size is 100
- filenames with spaces may break
- complex signatures may break
- function pointers are not supported

---

## Small example

`file2.h`

```c
int sub (int a,int b);
int stg (int a,int b);
```

`file2.c`

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

Graph will have stuff like:

```dot
"file2.c"->"file2.h";
"file2.h"->"int sub (int a,int b);";
"int stg (int a,int b);"->"int sub (int a,int b);";
"file2.c"->"main_of_file2.c";
"main_of_file2.c"->"stg";
```


