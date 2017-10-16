NAME(s) 
<Include real name & sunet username for both you and partner (if any)>
Name: Tiantian Tang
Sunet:azure3t

DESIGN 
<Give an overview of your allocator implementation (what data structures/algorithms/features)>

My heap allocator will implement mymalloc, myrealloc and myfree that mimics the system malloc, realloc and free.  The allocator maintains heap as collection of variable sized blocks which are either allocated or free.
 
The heap allocator utilizes explicit free lists. The free lists are segregated and head pointers of each list is stored in an array of size 29.  Maximum payload of each list is calculated as : 8 * 2^idx where idx is free list index.  Thus, even free block with payload close to INT_MAX can be stored in the list.
  
Each block comprises of header, footer and payload section. The upper 60 bits of header/footer indicates the payload of the block, and the 1st bit of header/footer indicates allocation status.  For free block, it also has prev, next section which stores pointer to previous or next free block in the list.
   
Coalese policy:  bidirectional coalescing using boundary tags proposed by Knuth. Since each block has header and footer. When coalesce a free block x, it will check its adjacent left and right block. If any of them is free, allocator will first delete x and those neighbor free blocks, merge them into a larger free block, then insert to free list.
    
Free block insertion policy: LIFO. it always insert a free block to the beginning of corresponding free list then run coalesce on it. 
     
Allocation policy: first-fit. To allocate a block of size n, it will search appropriate free list of size m>=n, if block is found, it will try to split the block and place fragment on appropriate list. If no block is found, it will try next larger free list. If still no block is found, it will request additional heap memory from OS then allocate.
      
Additionally, heap starts with a prologue and ends with epilogue section which are always marked allocated.  As it requested more page from OS, the allocator wil update epilogue. Prologue and eiplogue are here to prevent accesing dangerous area in the heap.


RATIONALE 
<Provide rationale for your design choices. Describe motivation for the initial selection of base design and support for the choices in parameters and features incorporated in the final design.>

I came up with my deisn by finding data structure and algorithms with a balance of high throughput and utilization. I start with making heap allocator work for one free list in one page and keep priming it until I get final result. I ask myself following questions to get started.
 
Q: How to store and access data? 
A: use some header/footer to indicate free/allocation. Also header and footer will be useful for O(1) coalescing. High throughput.
  
Q: How to manage free blocks and why?
A: use a doubly linked lists. Because I anticipate that this list will be update(insertion/deletion) very often. Doubly linked list has O(1) such operation. High throughput!
   
Q: How to find available spot?
A: First-fit in segregated list! Because this will increase memory utilization - first-fit search of segregated list is approximately a best-fit search of entire heap.
    
Q: Other mechanisms to increase utilization?
A: When allocate a block, will allocate as much as needed. Rest chunk of free block will be collected and insert to free list node. High utilization. 


OPTIMIZATION 
<Describe how you optimized-- what tools/strategies, where your big gains came from. Include at least one specific example of introducing a targeted change with supporting before/after data to demonstrate the (in)effectiveness of your efforts.>

1. Optimize free block insertion policy:  Initially it inserts free block using address-ordered policy. However, this introduces O(n) search for each insertion. Which makes throughput really really low. Later I changed to LIFO policy - always insert to begining of list. Thus, insert becomes O(1). Which brings up throughput by 30%.

2. Request extra pages from OS each time instead needed pages. Page request is expensive, so do it less frequently. This increase my throughput by 10%.  

3. Realloc to block with size bigger than requested. Beause if a block is realloced, it has bigger chance to be realloced in the future. Before: realloc whatever is neede. After: realloc 1.5 * needed size. This brings up throughput by 5%.


EVALUATION 
<Give strengths/weaknesses of your final version. What are the characteristics of the workloads it performs well (and not as well) on? What are most promising opportunities for future enhancements?>

Strength: 
Overall utilization is high ~70%, extreme case can be 90%.  
Throughput for realloc-patterns are relative high. 
 
Weakness: 
Low overall throughput.  
Reassemble and binary-pattern has lower utilziation 26%-30%.
  
Future improvement: 
Need to improve throughput. Coalescing each time when insert a free block might lower throughput. Instead, we can try to keep track of external fragementation in heap and free counts, then decide when to coalesce from those prameters.
To improve utilization, we can try to reduce internal fragmentaion by removing footer.
Implement buddy memory allocation - splitting memory into halves to try to give a best-fi.


WHAT WOULD YOU LIKE TO TELL US ABOUT YOUR PROJECT?
<Of what are you particularly proud: the effort you put into it? the results you obtained? the process you followed? what you learned along the way? Tell us about it! We are not scoring this section of the readme, but what you offer here provides context for the grader who is reviewing your work. Share with us what you think we should know!>

Thank you Cynthia and all awsome TAs!
I am proud that I almost finish CS107 and my heap allocator turned out to work given limited time. This class not only taught me knowledge, but also make me more patient and strong...haha :)  
 

REFERENCES
<If any external resources (books, websites, people) were influential in shaping your design, they must be properly cited here>

Randal E. Bryant, David R. O'Hallaron. Computer Systems. Reading,Mass.: Pearson, 2010.Print
Knuth, Donald E. The Art of Computer Programming. Reading,Mass.: Addison-Wesley, 2008. Print.
http://www.cs.cmu.edu/afs/cs/academic/class/15213-f10/www/lectures/17-allocation-basic.pdf
http://www.cs.cmu.edu/afs/cs/academic/class/15213-f10/www/lectures/18-allocation-advanced.pdf
 
  

