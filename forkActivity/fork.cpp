#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    int pid = fork();
    std::cout << "Fork Returned " << pid << std::endl;
    if (pid == 0) { //We are the child
        std::cout << "Child about to exec" << std::endl;
        execl("/bin/ls","/bin/ls",(char*)0);
        std::cout << "Child should never reach this (replaced with ls)" << std::endl;
    }
    else {
        int status;
        wait(&status);
        std::cout << "Parent after wait" << std::endl;
    }
    return 0;
}
