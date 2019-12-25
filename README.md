# Procmonel
Procmonel is `Procmon` like monitoring system implemented using Microsoft WDK.

## Architecture
- `Kernel Mode Filter Driver` that registers to kernel notifications
- `Console Application` for sending user commands to kernel driver and to receive driver notifications

## Features
Kernel mode filter registers to following notifications:  
  
1. Process Notification Routine 
    a. Create
    b. Terminate
2. Thread Notification Routine
    a. Create
    b. Terminate
3. Image Notification Routine 
    a. Image load inside a process (`.dll`)
4. Registry Notification Routine
    a. Create
    b. Set Value
    c. Delete Key
    d. Delete Value
5. File operations
    a. Create
    b. Close
    c. Cleanup
    d. Read
    e. Write
    f. Set Attributes

## Built with
- C
- WDK (Windows Driver Kit)
- Win32API
- Microsoft Visual Studio

## Setup
1. Compile project for `x64` platform
2. Test filter driver using `WinDbg` and connect to remote machine
3. Copy the output of compiled project including `.sys`, `Setup Information` and `Security Certificate` files
4. Install the driver by right-clicking on the `.sys` file and selecting `Install`

## Usage
- User is allowed to input `exit` command to stop monitoring
- Console Application shows notifications from the driver in following form:  
> ProcMonel: [Time] | [Operation] | [PID] | [Path] | [Result] | [Details] |