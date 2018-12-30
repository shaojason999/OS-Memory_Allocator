# hw4-simple-memory-allocator-shaojason999

### chunk header結構
1. prev chunk: long int(8-byte)
* 指的是所有切割出來的chunk(allocated, free)中，相鄰自己，地址較低的chunk的size(有可能是allocated, free)
2. next chunk: long int(8-byte)
3. prev chunk size: int(31-bit)
4. allocated flag: int(1-bit)
5. current chunk size: int(31-bit)
6. mmap flag: int(1-bit)
* 3,4合併為一個int，5,6合併為一個int(前31個bit是size部分，最後一個bit是flag)
* 所以int=size部分*2(<<1)+flag bit


### Note
1. 今天有一個地址A，今天若A=A+8，則結果A=A+sizeof(A)\*8，不是單純的+8，如果想要單純的+8則寫成:A=(void*)A+8
2. a<<1+1，等同於a<<(1+1)，括號要記得: (a<<1)+1
