#ifndef SECCOMP_H
#define SECCOMP_H

// basic filter 
// this mode should not interfere with normal operations
// only some dangerous syscalls are blacklisted
int enableSandboxBasic(void);

// enhanced filter 
// this mode allows only the necessary syscalls used during normal operation
// extensive testing required !!!
int enableSandboxFull(void);


#endif
