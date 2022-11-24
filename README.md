# A header only, super simple, command line flags library

Inspired from gflags style of declaring flags and auto parsing them. This lib
is dead simple, much lighter than gflags (180 lines in a single header file).

Unlike `boost_options`, it doesn't require insane amount of boilerplate code
from its users.

## How to use in a super simple way:

```C++

// At any header file (if we need to use it in multile TUs):
DECLARE_MFLAG(int, price);

// At any cpp file (or corresponding cpp file if declared in header).
DEFINE_MFLAG(int, price, 15, "Price of the phone"); // 15 is default value

// At the main file:
MFLAG_ODR_INIT();
int main(int argc, char* argv[]) {
  mflags::ParseFlags(argc, argv);
  ...
}

// Use the flag anywhere in the program like a global variable:
std::cout << (MFLAGS_price + 1) << std::endl;

```

At Command line: `./program --price 200 --name ABC`

Display all options : `./program --help` (It will print options and exit(0))


- Declare and define a lot of flags across different files arbitrarily. All of them will be
  auto combined.

- Supports 4 basic types - int / double / bool / string for declaring flags.

- Some of the flags could be declared in dynamically loaded shared lib, i.e. the libs loaded after
  entering `int main()` and completing `mflags::ParseFlags`. In that case, value of the flags
  declared in shared-lib will also be initialized at the library load time (when `static-init` runs)

## Advance Usage:

More complex use cases are enumerated in `mflags_test.cpp`

