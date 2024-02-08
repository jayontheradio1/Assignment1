Instructions for navigating and executing commands in a project using the Terminal, you can follow these steps:

1. Navigate to the root directory of the project, named `assign1`, using your Terminal application.

2. Use the `ls` command to display the list of files. This helps confirm that you're in the correct directory.

3. Enter the command `make clean` which removes any previously compiled `.o` files, ensuring a fresh start for the build process.

4. Compile all the files in the project, including the `test_assign1_1.c` file, by typing the command `make`.

5. Execute the command `make run_test1` to run the compiled version of the `test_assign1_1.c` file.

6. To compile a custom test file named `test_assign1_2.c`, input the command `make test2`.

7. Finally, launch the `test_assign1_2.c` file by entering `make run_test2`.

=========================================================================================================================================================
Managing Page Files

1. **`createPageFile(char *fileName)`**:
   - Purpose: Creates a new page file.
   - Process:
     - Opens (or creates if it doesn't exist) a file with the provided `fileName` in write-plus mode (`"w+"`), which allows both reading and writing.
     - Allocates memory for an empty page initialized with zeros.
     - Writes this empty page to the file to ensure the file has an initial size equal to one page.
     - Handles success and failure cases, including memory allocation failure and write operation failure.

2. **`openPageFile(char *fileName, SM_FileHandle *fHandle)`**:
   - Purpose: Opens an existing page file.
   - Process:
     - Opens the specified file in read mode.
     - If successful, stores the file's name, sets the current page position to 0, and stores the file pointer in the file handle (`fHandle`).
     - Uses `fstat` to determine the size of the file and calculates the total number of pages based on this size.
     - Closes the file after obtaining necessary information.
     - Handles the case where the file does not exist.

3. **`closePageFile(SM_FileHandle *fHandle)`**:
   - Purpose: Closes an open page file.
   - Process:
     - Checks if the file handle (`fHandle`) and its management information (`mgmtInfo`) are initialized.
     - Closes the file associated with the file handle.
     - Sets the file handle's management information to `NULL` after closing the file.
     - Handles the case where the file handle is not initialized.

4. **`destroyPageFile(char *fileName)`**:
   - Purpose: Deletes a page file.
   - Process:
     - Checks if the provided `fileName` is valid (not `NULL`).
     - Attempts to remove the file with the specified name.
     - Handles success (file successfully removed) and failure (file not found or unable to delete) cases.
=========================================================================================================================================================
Reading Blocks from the disk

1. **`readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Reads a specified block (page) from a file.
   - Process:
     - Checks if the requested page number is valid (within the range of existing pages).
     - Opens the file in binary read mode.
     - Seeks to the position corresponding to `pageNum`.
     - Reads the block of data into `memPage`.
     - Updates `fHandle->curPagePos` to reflect the read position.
     - Handles errors such as file opening failure, seek failure, and incomplete read.

2. **`getBlockPos(SM_FileHandle *fHandle)`**:
   - Purpose: Retrieves the current position (page number) in the file.
   - Process:
     - Returns the current page position stored in `fHandle`.
     - Includes error handling for cases where the current position is not set or `fHandle` is `NULL`.

3. **`readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Reads the first block of the file.
   - Process:
     - Checks if `fHandle` and `memPage` are valid.
     - Calls `readBlock` for the first page (page number 0).

4. **`readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Reads the block preceding the current position in the file.
   - Process:
     - Calculates the index of the previous block based on the current position.
     - Calls `readBlock` for the previous page if it exists.

5. **`readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Reads the block at the current position in the file.
   - Process:
     - Calculates the index of the current block based on the current position.
     - Calls `readBlock` for the current page if it is within the valid range.

6. **`readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Reads the block following the current position in the file.
   - Process:
     - Determines the index of the next block based on the current position.
     - Calls `readBlock` for the next page if it exists and is within the valid range.

7. **`readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Reads the last block of the file.
   - Process:
     - Determines the index of the last block.
     - Calls `readBlock` for the last page.
=========================================================================================================================================================
Writing blocks to page file

1. **`writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Writes a block (or page) of data to a specific location in a file.
   - Process:
     - Opens the file specified by `fHandle->fileName` in read/write mode.
     - Checks for several conditions: valid file descriptor, valid page number, and successful seek operation to the desired page.
     - Writes the content from `memPage` to the specified page in the file.
     - Handles errors through a switch statement and a `goto` label for error handling.
     - Updates `fHandle->curPagePos` after successful write and closes the file.

2. **`writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)`**:
   - Purpose: Writes data to the current block in the file.
   - Process:
     - Determines the current page to write to by checking `fHandle->curPagePos`.
     - Adjusts the current page if it is out of valid bounds.
     - Calls `writeBlock` to write to the adjusted current page.

3. **`appendEmptyBlock(SM_FileHandle *fHandle)`**:
   - Purpose: Appends an empty block to the end of the file.
   - Process:
     - Allocates memory for an empty block and initializes it to zero.
     - Opens the file in read/write mode and seeks to the end.
     - Writes the empty block to the file, extending its size.
     - Handles errors in file opening, seeking, and writing.
     - Increments the `totalNumPages` in `fHandle` after a successful write.

4. **`ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)`**:
   - Purpose: Ensures that the file has at least a specified number of pages.
   - Process:
     - Iteratively checks if the file has fewer pages than `numberOfPages`.
     - Calls `appendEmptyBlock` to append empty pages until the file reaches the desired capacity.
     - Handles errors in appending blocks.
