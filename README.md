# RISC-V xv6 kernel

School project for the OS1 course, at the School of Electrical Engineering in Belgrade.

I implemented the following:
- memory allocation
- synchronous context switching
- asynchronous user-mode context switching, caused by timer interrupts
- 15+ syscalls
- Console IO
without nested ecalls.

To begin the project, I was provided with a hardware library(`hw.lib`), which is in use in the final version of the project, and a context-switching skeleton for the kernel.

To run, [download the VM](https://drive.google.com/file/d/1edGYFcvdnV0pbKws_1G1vePtEec0qC0G/view?usp=sharing) and run `make qemu`.
