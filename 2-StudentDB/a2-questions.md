## Assignment 2 Questions

#### Directions
Please answer the following questions and submit in your repo for the second assignment.  Please keep the answers as short and concise as possible.

---

1. In this assignment I asked you provide an implementation for the `get_student(...)` function because I think it improves the overall design of the database application.   After you implemented your solution do you agree that externalizing `get_student(...)` into it's own function is a good design strategy?  Briefly describe why or why not.

    > **Answer**:  
    Yes. Centralizing the logic for locating and retrieving a student in one function reduces duplication and makes the code easier to maintain. It also makes it simpler to handle errors consistently in a single place.

---

2. Another interesting aspect of the `get_student(...)` function is how its function prototype requires the caller to provide the storage for the `student_t` structure. Notice that the last parameter is a pointer to storage **provided by the caller**. This is a common convention (called pass-by-reference) in the C programming language.  

    In other programming languages an approach like the one shown below would be more idiomatic for creating a function like `get_student()` (specifically the storage is provided by the `get_student(...)` function itself):

    ```c
    student_t *get_student(int fd, int id){
        student_t student;
        bool student_found = false;

        if (student_found)
            return &student;
        else
            return NULL;
    }
    ```

    Can you think of any reason why the above implementation would be a **very bad idea** using the C programming language?  Specifically, address why the above code introduces a subtle bug that could be hard to identify at runtime? 

    > **ANSWER:**  
    The function returns the address of a local variable that goes out of scope when the function ends. Accessing it later leads to undefined behavior. This bug can be very hard to detect and may cause intermittent crashes or data corruption.

---

3. Another way the `get_student(...)` function could be implemented is as follows:

    ```c
    student_t *get_student(int fd, int id){
        student_t *pstudent;
        bool student_found = false;

        pstudent = malloc(sizeof(student_t));
        if (pstudent == NULL)
            return NULL;
        
        if (student_found){
            return pstudent;
        }
        else {
            free(pstudent);
            return NULL;
        }
    }
    ```
    In this implementation the storage for the student record is allocated on the heap using `malloc()` and passed back to the caller when the function returns. What do you think about this alternative implementation of `get_student(...)`?  Address in your answer why it work work, but also think about any potential problems it could cause.  
    
    > **ANSWER:**  
    It can work because the data remains valid after the function returns. However, the caller must remember to free the memory, which can lead to memory leaks if forgotten. It also makes the function more complex than necessary and adds overhead for every lookup.

---

4. Lets take a look at how storage is managed for our simple database.  

    - **(a)** Please explain why the file size reported by the `ls` command was 128 bytes after adding student with ID=1, 256 after adding student with ID=3, and 4160 after adding the student with ID=64?

        > **ANSWER:**  
        Each student record is stored at `(id * 64)`, plus 64 bytes for the record itself, which sets the file’s end position. `ls` reports this logical size, so when you add ID=64, the file end moves to `64*64 + 64 = 4160`.

    - **(b)** Why did the total storage used on the disk remain unchanged when we added the student with ID=1, ID=3, and ID=63, but increased from 4K to 8K when we added the student with ID=64?

        > **ANSWER:**  
        The filesystem only allocates blocks as needed. Until the data crosses a new 4K boundary, no additional physical blocks are used, so `du` shows the same disk usage. Adding ID=64 crossed another 4K boundary, requiring an extra block.

    - **(c)** We see from above adding a student with a very large student ID (ID=99999) increased the file size to 6400000 as shown by `ls` but the raw storage only increased to 12K as reported by `du`.  Can provide some insight into why this happened?

        > **ANSWER:**  
        Large IDs create a hole in the file at offset `(99999 * 64)`, which doesn’t consume real disk blocks until data is actually written there. `ls` shows the logical file size, while `du` shows only the actual blocks allocated. Linux filesystems handle sparse files efficiently, so the disk usage is still small.
