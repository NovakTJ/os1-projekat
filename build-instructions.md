# Build Instructions

## Standard Build & Run
```bash
make          # Build the kernel (produces: kernel, kernel.asm)
make qemu     # Run kernel in QEMU
```

## Test Build & Run
```bash
make test1        # Build with test1.cpp as main
make qemu-test1   # Build and run test1
```

## Debugging
```bash
make qemu-gdb     # Start QEMU paused, waiting for GDB on port 26000
# Then in another terminal:
gdb-multiarch     # Connects via .gdbinit (auto-generated from .gdbinit.tmpl-riscv)
```

### CLion Remote Debug Configuration
Found in `.idea/workspace.xml` (inline, not in runConfigurations/):
- **Name**: RemoteDebug
- **Type**: CLion_Remote (GDB Remote Debug)
- **Remote command**: `localhost:26000`
- **Symbol file**: `$PROJECT_DIR$/kernel`
- **Sysroot**: `$PROJECT_DIR$`
- **Debugger**: `/usr/bin/gdb-multiarch`

### GDB Port
Port is calculated as `id -u % 5000 + 25000` = **26000** for current user.

### Note on .gdbinit
The generated `.gdbinit` has `symbol-file kernel/kernel` (from xv6 template) but the actual kernel binary is at `./kernel`. CLion's config correctly uses `$PROJECT_DIR$/kernel`. If using CLI GDB, may need to fix the symbol-file path.

## Clean
```bash
make clean    # Remove all build artifacts
```

## Key Makefile Variables
- `MAIN_SRC` - Which main file to compile (default: `src/main.cpp`)
- `DEBUG_FLAG` - Debug print flag (default: `-D DEBUG_PRINT=0`)
- `CPU_CORE_COUNT` - QEMU CPU cores (default: 1)

## NEVER Do
- Do NOT try to compile files manually with gcc/g++ — always use `make`
- Do NOT use standard library headers or functions
