# CS153
## Copy-Paste Bank
``kill $(pgrep qemu)``  
``find . -type f -name \*.c -exec sed -i.bak 's|TOREPLACE|REPLACEWITH|g' {} +``     
``grep -rnw '/path/to/somewhere/' -e 'pattern``  
``echo "add-auto-load-safe-path $HOME/xv6/.gdbinit" > ~/.gdbinit``  
``make qemu-nox-gdb``  
``make qemu-ddb``  
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
[find all occurences of x](https://stackoverflow.com/questions/16956810/how-do-i-find-all-files-containing-specific-text-on-linux)  

Basic global read/write command for ease of copy-paste.  **CAPS = PLACEHOLDER**

``find . -type f -name \*.c -exec sed -i.bak 's|TOREPLACE|REPLACEWITH|g' {} +``   
``-type f`` : type of thing to find = file(s)  
``-name \*.c``:name of the file will be anything (because * matches anything b/c regex) ending with a .c extension  
``-i.bak``: creates a backup file with the same name as the original   
``'s|TOREPLACE|REPLACEWITH|g'``: s = substitute g = global   


Alternatively, to just find all occurences of something  
``grep -rnw '/path/to/somewhere/' -e 'pattern'``  
