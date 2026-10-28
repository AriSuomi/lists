# Lists
Various data structures in C.

These data structures were originally implemented as part of my RTOS (LwO) but
I decided to put them in a separate repository, because they are quite generic
and could be used anytime a list is needed.

They are used as priority queues in the LwO RTOS, so they might lack some
features needed for other purposes.

They are made for embedded systems but will work on any system. The
code aims to be highly portable and uses only standard C99. The only target
dependent parts are in lwl_port.h.
