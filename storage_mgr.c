#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>
#include <fcntl.h>
#include <stdbool.h>

#include "storage_mgr.h"

FILE *file;

extern void initStorageManager (void) {
	// Initialising file pointer i.e. storage manager.
	file = NULL;
}

extern RC createPageFile (char *fileName) {
    
    file = fopen(fileName, "w+");  // More secure alternative to fopen

    // Check if file was successfully opened.
    if (file == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Allocate memory for an empty page using calloc, which initializes memory to zero.
    SM_PageHandle emptyData = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));

    // Check if memory allocation was successful.
    if(emptyData == NULL){
        fclose(file);
        return RC_MEMORY_ALLOCATION_FAIL;
    }

     // Write the empty page to the file.
    size_t writeSize = fwrite(emptyData, sizeof(char), PAGE_SIZE, file);
    if(writeSize == PAGE_SIZE) {
        printf("Write suceeded.\n");
        free(emptyData);
        fclose(file);
        return RC_OK;
    }
        printf("Write Failed. \n");
        free(emptyData);
        fclose(file);
        return RC_WRITE_FAILED;
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle){

    file = fopen(fileName, "r");

    if(file != NULL){
        fHandle->fileName = fileName;
        fHandle->curPagePos = 0;
        fHandle->mgmtInfo = file; 
        
        // Store the file pointer

        //Moving to the end of the file to calculate its size
        /*fseek(file, 0L, SEEK_END);

        long sizeFile = ftell(file);

        //Moving back to the starting of the file
        fseek(file, 0L, SEEK_SET);*/

        // Using fstat to get the information of the file
        // Then using st_size pointer to get the size
        struct stat fileInfo;
        if (fstat(fileno(file), &fileInfo) == 0) {
            fHandle->totalNumPages = fileInfo.st_size / PAGE_SIZE;
        }

        fclose(file);
        return RC_OK;
    }

  return RC_FILE_NOT_FOUND;
}

extern RC closePageFile (SM_FileHandle *fHandle) {
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT; // Or a similar error code
    }

    fclose((FILE*)fHandle->mgmtInfo);
    fHandle->mgmtInfo = NULL;
    return RC_OK;
}

extern RC destroyPageFile(char *fileName) {
    if (!fileName) {
        return RC_INVALID_FILENAME; //There is no file with this name
    }

    int status = remove(fileName);

    if (status != 0) {
        return RC_FILE_NOT_FOUND; //File doesn't exist
    }

    return RC_OK;
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {	
    int file_descriptor;
    int action = 0; // 0 for default, 1 for error handling

    file_descriptor = open(fHandle->fileName, O_RDWR);
    action = (file_descriptor < 0) ? 1 : action;
    action = (pageNum < 0 || pageNum >= fHandle->totalNumPages) ? 1 : action;
    action = (lseek(file_descriptor, pageNum * PAGE_SIZE, SEEK_SET) < 0) ? 1 : action;

    switch(action) {
        case 1: 
            goto error;
        default:
            break;
    }

    int write_result = write(file_descriptor, memPage, PAGE_SIZE);
    close(file_descriptor);

    if (write_result < 0) {
        return RC_WRITE_FAILED;
    }

    fHandle->curPagePos = pageNum;
    return RC_OK;

error:
    close(file_descriptor);
    return (pageNum < 0 || pageNum >= fHandle->totalNumPages) ? RC_WRITE_FAILED : RC_FILE_NOT_FOUND;
}

//------------------------------------------------------------------------------------------------------------------------------------

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int currentPage = fHandle->curPagePos;

    // Preprocessing step: Adjusting the currentPage if needed
    currentPage = (currentPage < 0) ? 0 : currentPage;
    currentPage = (currentPage >= fHandle->totalNumPages) ? fHandle->totalNumPages - 1 : currentPage;

    // Now, call writeBlock with the adjusted currentPage
    return writeBlock(currentPage, fHandle, memPage);
}
//------------------------------------------------------------------------------------------------------------------------------------


RC appendEmptyBlock (SM_FileHandle *fHandle) {
    // Initialize a file descriptor and set the default return code to RC_OK
    int file_descriptor;
    RC returnCode = RC_OK;

    // Allocate memory for an empty page and initialize to zero
    SM_PageHandle emptyPage = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char));
    if (emptyPage == NULL) {
        // If memory allocation fails, return memory allocation error
        return RC_ERR;
    }

    // Open the file in read/write mode
    file_descriptor = open(fHandle->fileName, O_RDWR);
    if (file_descriptor < 0) {
        // If opening the file fails, free allocated memory and return file not found
        free(emptyPage);
        return RC_FILE_NOT_FOUND;
    }

    // Seek to the end of the file
    if (lseek(file_descriptor, 0, SEEK_END) < 0) {
        // If seeking fails, update the return code
        returnCode = RC_WRITE_FAILED;
    } else {
        // Write the empty page to the file
        if (write(file_descriptor, emptyPage, PAGE_SIZE) < PAGE_SIZE) {
            // If writing fails, update the return code
            returnCode = RC_WRITE_FAILED;
        } else {
            // On successful write, increment the total number of pages
            fHandle->totalNumPages += 1;
        }
    }

    // Clean up resources
    free(emptyPage);
    close(file_descriptor);

    // Return the final return code
    return returnCode;
}

//------------------------------------------------------------------------------------------------------------------------------------


RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    // Initialize a result variable for function calls
    RC result;

    // Loop to check and append pages only if needed
    for (int currentPage = fHandle->totalNumPages; currentPage < numberOfPages; ++currentPage) {
        // Append an empty block
        result = appendEmptyBlock(fHandle);
        if (result != RC_OK) {
            // If append operation fails, return the error code
            return result;
        }
    }

    // In case the current total number of pages was already adequate
    if (fHandle->totalNumPages >= numberOfPages) {
        return RC_OK; // No further action required
    }

    // Return success if the loop completes without errors
    return RC_OK;
}


extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

    bool pageResult = (pageNum < 0 || pageNum >= fHandle->totalNumPages);
    if ( !pageResult ) {
      FILE *file_pointer = fopen(fHandle->fileName, "rb");
    if (file_pointer == NULL) {
        return RC_ERR;
    }

    int seek_result = fseek(file_pointer, pageNum * PAGE_SIZE, SEEK_SET);
    if (seek_result != 0) {
        fclose(file_pointer);
        return RC_ERR;
    }

    size_t bytes_read = fread(memPage, sizeof(char), PAGE_SIZE, file_pointer);
    if (bytes_read != PAGE_SIZE) {
        fclose(file_pointer);
        return RC_ERR;
    }

    fHandle->curPagePos = pageNum * PAGE_SIZE;

    fclose(file_pointer);

    return RC_OK;
    }//IF

    return RC_ERR;

}



extern int getBlockPos(SM_FileHandle *fHandle) {
    int PagePosition = 0;
    PagePosition = fHandle->curPagePos;
    if(PagePosition == 0 ){
        return RC_ERR;
    }
    return fHandle->curPagePos;
}



extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {

    if(fHandle == NULL && memPage == NULL){
        return RC_ERR;
    }
    return readBlock(0, fHandle, memPage);
}


extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Calculate the index of the previous block, considering the possibility of underflow
    int preceding_block_index = (fHandle != NULL && memPage != NULL && fHandle->curPagePos > 0) ? ((fHandle->curPagePos - 1) / PAGE_SIZE) : -1;

    // Return appropriate status depending on the calculated index
    return (preceding_block_index >= 0) ? readBlock(preceding_block_index, fHandle, memPage) : RC_READ_NON_EXISTING_PAGE;
}

extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Calculate the index of the active (current) block, adjusting for page alignment
    int active_block_index = (fHandle != NULL && memPage != NULL) ? ((fHandle->curPagePos + PAGE_SIZE - 1) / PAGE_SIZE) : -1;

    // Return appropriate status depending on the calculated index
    return (active_block_index >= 0 && active_block_index < fHandle->totalNumPages) ? readBlock(active_block_index, fHandle, memPage) : RC_READ_NON_EXISTING_PAGE;
}



extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Determine next block index, considering NULL pointers and wrapping around the file size
    int next_block_index = (fHandle != NULL && memPage != NULL) ? ((fHandle->curPagePos + 1) % (fHandle->totalNumPages + 1)) : -1;

    // Use ternary operator for determining the return value
    RC status = (next_block_index < 0 || next_block_index >= fHandle->totalNumPages) ? RC_READ_NON_EXISTING_PAGE : readBlock(next_block_index, fHandle, memPage);

    return status;
}

extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Determine last block index, considering NULL pointers and adjusting for empty files
    int last_block_index = (fHandle != NULL && memPage != NULL) ? ((fHandle->totalNumPages > 0) ? fHandle->totalNumPages - 1 : 0) : -1;

    // Use ternary operator for determining the return value
    RC status = (last_block_index < 0) ? RC_READ_NON_EXISTING_PAGE : readBlock(last_block_index, fHandle, memPage);

    return status;
}


