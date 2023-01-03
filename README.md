# MFLAGS C++

https://github.com/mohitmv/mflags

## A powerful yet super intuitive Command Line Arguments parsing library for C++

This is inspired by python's argparse library, used by 100s of thousands of people
for its ease and intuitiveness.

mflags implements most features of `argparse` in C++ context, and even more
easy to use stuff for C++. It parse the command line flags in strongly-typed
way, and auto-populates C++ objects, which could be core types like
(int, string etc.) or **vector-of-core-types**, or **tuple/pair-of-core-types**,
or **vector-of-tuple/pair-of-core-types**. All of this without requiring insane
amount of boilerplate code from users of mflags, and without doing
untuitive magical things internally.


## How to use mflags:

There are two ways of using it, and these two ways can be combined as well:

### First way of using it:

```C++

int main(int argc, const char* const* argv) {
  int f1 = 0;
  std::pair<std::string, int> f2 {};
  std::vector<std::pair<int, std::string>> f3;
  std::vector<int> f4;
  mflags::ArgsDescriptor args_desc{"Help message about this program."};
  args_desc.AddArg({.names={"-f1"}, .help_text="For F1"}, &f1);
  args_desc.AddArg({.names={"-f2", "--field2"}, .help_text="For F2"}, &f2);
  args_desc.AddArg({.names={"-f3"}, .help_text="For F3"}, &f3);
  args_desc.AddArg({.names={"-f4", "--field4"}, .help_text="For F4"}, &f4);
  args_desc.ParseFlags(argc, argv);
  // Continue to use @f1, @f2... assuming everything was fine.
}
```

This will automatically generate help text (`--help`, `-h`), and will crash the
program if params passed are invalid at `args_desc.ParseFlags` step itself.

```
Help message about this program.

Command line options:

  -h, --help              Show this help message and exit. Type: bool
  -f1 VALUE               For F1. Type: int
  -f2, --field2 VALUE1 VALUE2
                          For F2. Type: pair<string, int>
  ( -f3 VALUE1 VALUE2 )*
                          For F3. Type: vector<pair<int, string>>
  -f4, --field4 VALUES...
                          For F4. Type: vector<int>
```

Possible params:

```
./program -f3 10 ABC -f3 11 DEF -field2 ABC 100 --field4 1 3 4 3 4 3 -f1 30
```

### Second way of using it:

This way requires much less code but depends on global variables. This way is somewhat similar to google's gflags, but way more powerful.

```C++
// At any cpp file
ADD_GLOBAL_MFLAG(int, g_price, 10, {.names={"--price"}, .help_text="Price of phone"});

// and use the flag anywhere in the program like a global variable:
std::cout << (g_price + 1) << std::endl;

// To use g_price in some other TU, we need to declare as:
extern int g_price;

```

```C++
// At the main cpp TU
int main(int argc, char* argv[]) {
  mflags::ParseFlags(argc, argv);
  ...
}

```

## Advance Usage:

More complex use cases are enumerated in `mflags_test2.cpp`
