#include "libsec.h"
#include <stdio.h>

#define HAVE_LIBSECCOMP
#ifdef HAVE_LIBSECCOMP

#include <seccomp.h> /* libseccomp */
#include <sys/prctl.h> /* prctl */
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#define DENY_RULE(call) { if (seccomp_rule_add (ctx, SCMP_ACT_KILL, SCMP_SYS(call), 0) < 0) goto out; }
#define ALLOW_RULE(call) { if (seccomp_rule_add (ctx, SCMP_ACT_ALLOW, SCMP_SYS(call), 0) < 0) goto out; }

scmp_filter_ctx ctx;


int enableSandboxBasic(void){

    // prevent child processes from getting more priv e.g. via setuid, capabilities, ...
    //prctl(PR_SET_NO_NEW_PRIVS, 1);

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl SET_NO_NEW_PRIVS");
        exit(EXIT_FAILURE);
    }


    // prevent escape via ptrace
    //prctl(PR_SET_DUMPABLE, 0);

    if(prctl (PR_SET_DUMPABLE, 0, 0, 0, 0)){
        perror("prctl PR_SET_DUMPABLE");
        exit(EXIT_FAILURE);
    }


    // initialize the filter
    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (ctx == NULL)
        return 1;

    DENY_RULE (_sysctl);
    DENY_RULE (acct);
    DENY_RULE (add_key);
    DENY_RULE (adjtimex);
    DENY_RULE (chroot);
    DENY_RULE (clock_adjtime);
    DENY_RULE (create_module);
    DENY_RULE (delete_module);
    DENY_RULE (fanotify_init);
    DENY_RULE (finit_module);
    DENY_RULE (get_kernel_syms);
    DENY_RULE (get_mempolicy);
    DENY_RULE (init_module);
    DENY_RULE (io_cancel);
    DENY_RULE (io_destroy);
    DENY_RULE (io_getevents);
    DENY_RULE (io_setup);
    DENY_RULE (io_submit);
    DENY_RULE (ioperm);
    DENY_RULE (iopl);
    DENY_RULE (ioprio_set);
    DENY_RULE (kcmp);
    DENY_RULE (kexec_file_load);
    DENY_RULE (kexec_load);
    DENY_RULE (keyctl);
    DENY_RULE (lookup_dcookie);
    DENY_RULE (mbind);
    DENY_RULE (nfsservctl);
    DENY_RULE (migrate_pages);
    DENY_RULE (modify_ldt);
    DENY_RULE (mount);
    DENY_RULE (move_pages);
    DENY_RULE (name_to_handle_at);
    DENY_RULE (open_by_handle_at);
    DENY_RULE (perf_event_open);
    DENY_RULE (pivot_root);
    DENY_RULE (process_vm_readv);
    DENY_RULE (process_vm_writev);
    DENY_RULE (ptrace);
    DENY_RULE (reboot);
    DENY_RULE (remap_file_pages);
    DENY_RULE (request_key);
    DENY_RULE (set_mempolicy);
    DENY_RULE (swapoff);
    DENY_RULE (swapon);
    DENY_RULE (sysfs);
    DENY_RULE (syslog);
    DENY_RULE (tuxcall);
    DENY_RULE (umount2);
    DENY_RULE (uselib);
    DENY_RULE (vmsplice);


    // blocking dangerous syscalls that file should not need

    DENY_RULE (execve);
    DENY_RULE (socket);
    //...

    
    //applying filter...
    if (seccomp_load (ctx) >= 0){
	// free ctx after the filter has been loaded into the kernel
	seccomp_release(ctx);
        return 0;
    }
    
  out:
    //something went wrong
    //printf("something went wrong\n");
    seccomp_release(ctx);
    return 1;
}


int enableSandboxFull(void){

    // prevent child processes from getting more priv e.g. via setuid, capabilities, ...
    //prctl(PR_SET_NO_NEW_PRIVS, 1);

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl SET_NO_NEW_PRIVS");
        exit(EXIT_FAILURE);
    }


    // prevent escape via ptrace
    //prctl(PR_SET_DUMPABLE, 0);

    if(prctl (PR_SET_DUMPABLE, 0, 0, 0, 0)){
        perror("prctl PR_SET_DUMPABLE");
        exit(EXIT_FAILURE);
    }

    
    // initialize the filter
    ctx = seccomp_init(SCMP_ACT_KILL);
    if (ctx == NULL)
        return 1;


    ALLOW_RULE (access);
    ALLOW_RULE (brk);
    ALLOW_RULE (close);
    ALLOW_RULE (dup2);
    ALLOW_RULE (exit);
    ALLOW_RULE (exit_group);
    ALLOW_RULE (fcntl);  
    ALLOW_RULE (fstat);
    ALLOW_RULE (getdents);
    ALLOW_RULE (ioctl);
    ALLOW_RULE (lseek);
    ALLOW_RULE (lstat);
    ALLOW_RULE (mmap);
    ALLOW_RULE (mprotect);
    ALLOW_RULE (mremap);
    ALLOW_RULE (munmap);
    ALLOW_RULE (open);
    ALLOW_RULE (openat);
    ALLOW_RULE (read);
    ALLOW_RULE (rt_sigaction);
    ALLOW_RULE (rt_sigprocmask);
    ALLOW_RULE (rt_sigreturn);
    ALLOW_RULE (select);
    ALLOW_RULE (stat);
    ALLOW_RULE (sysinfo);
    ALLOW_RULE (unlink);
    ALLOW_RULE (write);


    // needed by valgrind
    /* ALLOW_RULE (gettid); */
    /* ALLOW_RULE (getpid);	 */
    /* ALLOW_RULE (readlink); */
    /* ALLOW_RULE (pread64); */
    /* ALLOW_RULE (rt_sigtimedwait); */

    
    /* /\* special restrictions for socket, only allow AF_UNIX/AF_LOCAL *\/ */
    /* if (seccomp_rule_add (ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 1, */
    /* 			  SCMP_CMP(0, SCMP_CMP_EQ, AF_UNIX)) < 0) */
    /* 	goto out; */

    /* if (seccomp_rule_add (ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 1, */
    /* 			  SCMP_CMP(0, SCMP_CMP_EQ, AF_LOCAL)) < 0) */
    /* 	goto out; */


    /* /\* special restrictions for open, prevent opening files for writing *\/ */
    /* if (seccomp_rule_add (ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1, */
    /* 			  SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0)) < 0) */
    /* 	goto out; */

    /* if (seccomp_rule_add (ctx, SCMP_ACT_ERRNO (EACCES), SCMP_SYS(open), 1, */
    /* 			  SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_WRONLY, O_WRONLY)) < 0) */
    /* 	goto out; */

    /* if (seccomp_rule_add (ctx, SCMP_ACT_ERRNO (EACCES), SCMP_SYS(open), 1, */
    /* 			  SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_RDWR, O_RDWR)) < 0) */
    /* 	goto out; */


    /* /\* allow stderr *\/ */
    /* if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1, */
    /*                              SCMP_CMP(0, SCMP_CMP_EQ, 2)) < 0 ) */
    /*     goto out; */




    //applying filter...
    if (seccomp_load (ctx) >= 0){
	// free ctx after the filter has been loaded into the kernel
	seccomp_release(ctx);
        return 0;
    }

 out:
    //something went wrong
    seccomp_release(ctx);
    return 1;
}



#else /* HAVE_LIBSECCOMP */


int enableSandboxBasic(void){

    perror("No seccomp support compiled-in\n");
    return 1;
}

int enableSandboxFull(void){

    perror("No seccomp support compiled-in\n");
    return 1;
}


#endif /* HAVE_LIBSECCOMP */
