# CS153
## Copy-Paste Bank
``kill $(pgrep qemu)``  
``find . -type f -name \*.c -exec sed -i.bak 's|TOREPLACE|REPLACEWITH|g' {} +``     
``grep -rnw '/path/to/somewhere/' -e 'pattern``  
``echo "add-auto-load-safe-path $HOME/xv6/.gdbinit" > ~/.gdbinit``  
``make qemu-nox-gdb``  
``make qemu-ddb``  

## Adding System Calls
The prototypes for the user's system calls should appear in defs.h and user.h

[Adding a basic system call](https://stackoverflow.com/questions/8021774/how-do-i-add-a-system-call-utility-in-xv6?rq=1)

[Adding a system call with user passed parameters](https://stackoverflow.com/questions/46870509/how-to-pass-a-value-into-system-call-xv6)

## Writing Test Files
When writing user programs only use lib functions found in user.h and end the main function with an exit(status) instead of return as pictured

```C

#include "types.h"
#include "user.h"


int main(){
  printf(1, "Hello World \n");
  exit(0);
}

```

[Adding a program to run in qemu shell](https://ampleux.wordpress.com/2018/02/22/how-to-add-a-user-program-to-xv6/)

## Sed & Find basic usage
[Sed detailed tutorial here](http://www.grymoire.com/Unix/Sed.html#uh-0)  
[Sed basic stack overflow answer here](https://stackoverflow.com/questions/13364514/batch-replace-text-inside-text-file-linux-osx-commandline)   
[decent find tutorial here; make sure to view -xargs vs -exec](https://danielmiessler.com/study/find/)   

Basic global read/write command for ease of copy-paste.  **CAPS = PLACEHOLDER**

``find . -type f -name \*.c -exec sed -i.bak 's|TOREPLACE|REPLACEWITH|g' {} +``   
``-type f`` : type of thing to find = file(s)  
``-name \*.c``:name of the file will be anything (because * matches anything b/c regex) ending with a .c extension  
``-i.bak``: creates a backup file with the same name as the original   
``'s|TOREPLACE|REPLACEWITH|g'``: s = substitute g = global   


Alternatively, to just find all occurences of something  [Source](https://stackoverflow.com/questions/16956810/how-do-i-find-all-files-containing-specific-text-on-linux)  
``grep -rnw '/path/to/somewhere/' -e 'pattern'``  
* `-r` or `-R` is recursive, 
* `-n` is line number, and 
* `-w` stands for match the whole word. 
* `-l` (lower-case L) can be added to just give the file name of matching files.

Along with these, `--exclude`, `--include`, `--exclude-dir` flags could be used for efficient searching:

  - This will only search through those files which have .c or .h extensions:

        grep --include=\*.{c,h} -rnw '/path/to/somewhere/' -e "pattern"

  - This will exclude searching all the files ending with .o extension:

        grep --exclude=*.o -rnw '/path/to/somewhere/' -e "pattern"

  -  For directories it's possible to exclude a particular directory(ies) through `--exclude-dir` parameter. For example, this will exclude the dirs dir1/, dir2/ and all of them matching *.dst/:

        grep --exclude-dir={dir1,dir2,*.dst} -rnw '/path/to/somewhere/' -e "pattern"

This works very well for me, to achieve almost the same purpose like yours.

For more options check `man grep`.
