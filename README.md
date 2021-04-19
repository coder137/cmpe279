# cmpe279

CMPE279 assignments

# Assignments

## Assignment 1

- Folder **assignment1**
    - Read `assignment1/README.md` for build and testing steps
- Privilege seperation
    - `fork` and `setuid` server.c
    - Parent should setup socket connection
    - Child should receive data from client.c

## Assignment 2

- Folder **assignment2**
  - Read `assignment2/README.md` for testing steps
  - Build steps are same as assignment1
- Privilege seperation
  - `execv` server.c
  - After parent sets up socket connection and forks
  - Child after fork must execv for address space randomization
