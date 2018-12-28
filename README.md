# hw4-simple-memory-allocator-shaojason999

### chunk header結構
1. prev chunk: long int(8-byte)
2. next chunk: long int(8-byte)
3. prev chunk size: int(31-bit)
4. allocated flag: int(1-bit)
5. current chunk size: int(31-bit)
6. mmap flag: int(1-bit)
* 3,4合併為一個int，5,6合併為一個int(前31個bit是size部分，最後一個bit是flag)
* 所以int=size部分*2(<<1)+flag bit
