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
- Parses `.c` files line-by-line to:
  - detect function definitions  
  - track function calls  
- Builds a graph using linked structures:
  - `cnode` → C files  
  - `hnode` → Header files  
  - `fnode` → Functions  
- Outputs the graph in `.dot` format (Graphviz)

---

## 🛠️ Setup

### Compile

```bash
gcc ana.c -o ana
Install Graphviz (for visualization)
macOS
brew install graphviz
Ubuntu / Debian
sudo apt install graphviz
▶️ How to run
Put all .c and .h files in one folder (e.g., test)
Navigate into the folder:
cd test
Run:
./ana
📊 View the graph
dot -Tpng dep1.dot -o dep1.png
📦 Output
dep1.dot → Dependency graph
Makefile → Auto-generated build file
⚠️ Important Constraints

This is not a full C parser. It works best on clean, simple C code.

🔹 Function Declarations (.h files)

Use:

int add (int a,int b);

Avoid:

unsigned int add (int a,int b);
static int add (int a,int b);
🔹 Header Includes

Use:

#include "file.h"

Avoid:

#include <file.h>
🔹 Function Definitions

Preferred:

int add (int a,int b)
{
    return a+b;
}

Avoid:

int add(int a,int b){ return a+b; }
🔹 Function Calls

Works:

add(a,b);

May fail:

// add(a,b);
printf("add(a,b)");
(*add)(a,b);
🔹 Main Function

Preferred:

int main()
{
    stg(1,2);
}

Avoid:

int main() { stg(1,2); }
🔹 Braces

Braces must be balanced:

{
}
🔹 File Naming

Good:

file1.c
file2.h

Avoid:

my file.c
🚫 Limitations
Not a full C parser
Macros not handled
Comments/strings may interfere with parsing
Function pointers not supported
Complex signatures may fail
Max line length ≈ 100 characters
Assumes:
one definition per function
all files are in the same directory
🧪 Example
Header (file2.h)
int sub (int a,int b);
int stg (int a,int b);
Source (file2.c)
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
Sample Graph Output (simplified)
"file2.c"->"file2.h";
"file2.h"->"int sub (int a,int b);";
"int stg (int a,int b);"->"int sub (int a,int b);";
"file2.c"->"main_of_file2.c";
"main_of_file2.c"->"stg";
