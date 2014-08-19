libstage
===================

Tool to stage dynamic libraries to every node in a job

## Usage

 In Interactive Job:
 * Load the libstage module using "module load libstage"
 * Type and run as follows "libstage -f/--filename  /path/to/library"
 * That will load the library that you need to use on to every node that you requested in your job

In Submit Script:
 * Before running the program that needs the library, load the libstage module in your bash commands section using "module load libstage"
 * On the following line, enter the command as follows "libstage -f/--filename  /path/to/library"
 * Once your job begins, it will load the denoted library onto every node that you requested


