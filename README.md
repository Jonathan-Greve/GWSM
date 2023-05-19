# GWSM - A Shared Memory Library for Guild Wars
Welcome to GWSM, a no-fuss shared memory library created specifically for Guild Wars. I've designed it with the primary goal of easing the sharing of Guild Wars client data among different processes, utilizing the capabilities of Windows shared memory and interprocess synchronization mechanisms.

## About Shared Memory
Shared memory is a method that allows multiple processes to communicate by accessing the same memory space. It's known for its speed in interprocess communication as it bypasses the kernel involvement. Windows shared memory, a feature of the Windows operating system, is known for its performance, hence it's used in this library.

## Main Features of GWSM
- **Windows Shared Memory Use**: GWSM employs Windows shared memory to facilitate access to a shared memory space across different processes. This encourages faster prototype development for external applications.

- **Interprocess Synchronization**: To prevent data inconsistencies and race conditions, GWSM uses Windows' built-in interprocess synchronization mechanisms, ensuring accurate shared data across all processes.

- **Data Reading with Google Flatbuffers**: GWSM makes data reading easier by integrating a simple-to-use API using Google Flatbuffers. For a more detailed guide, please refer to the Google Flatbuffers Documentation. Please note that there are currently no plans to add botting capabilities (i.e., game-client control). Flatbuffers supports multiple programming languages.

## How to Use GWSM
Start by injecting the GWSM.dll file. Then, read the data stored in the shared memory in another process using the flatbuffer.

## Want to Help?
While I do appreciate pull requests, I am not actively developing this library, and therefore, might not merge your contributions. However, you're more than welcome to fork the repository and modify it to suit your needs.

## Giving Credit Where It's Due
This library has been influenced significantly by the work done on GWCA. A big thanks to the authors and contributors of [GWCA](https://github.com/GregLando113/GWCA) for their contributions to the Guild Wars modding community.
