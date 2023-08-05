#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h> // added by me
#include <sys/types.h>  // added by me
#include <unistd.h> // added by me
#include <sys/wait.h>  // added by me
#include <errno.h> // added by me
#include <fcntl.h> // added by me





bool do_system(const char *command);

bool do_exec(int count, ...);

bool do_exec_redirect(const char *outputfile, int count, ...);
