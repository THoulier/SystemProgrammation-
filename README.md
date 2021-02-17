# System Programmation Project
The application allows parallel execution of a program on several remote machines. 
The technologies used are :
- ssh
- threads
- network sockets
- fork/pipe
- signal handlers



Installation
------------
To install the application, you have to clone the repo and compile the files :

    $ git clone https://github.com/THoulier/SystemProgrammation-.git
    $ cd Phase2
    $ make
    
To launch the game, execute the next command line in a terminal :

    $ ./bin/dsmexec machine_file example arg1 arg2
    
The machine_file contains the name of the machine which will be used for the parallel execution of the program example.
The arguments arg1, arg2 ... can be added to complete the execution of the example file. 


