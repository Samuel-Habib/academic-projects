1. Project Members
    Samuel Habib smh389, Victor Peng vmp134

2. Testing Plan
    - Unit Tests (compareTest.c)
        - hasSuffix()
            - Tested a matching suffix, a non-matching suffix, an exact match, and a string shorter than the suffix.
        - insert()
            - Inserted three unique words and one duplicate into an empty fileData struct.
            - Verified uniqueWords, totalWords, alphabetical ordering, and duplicate count.
        - wfd()
            - Inserted a known word distribution (spam x3, eggs x1) and verified frequencies summed to 1.0.
        - jsd()
            - Verified the spec example ("hi there hi there" vs "hi hi out there") produces ~0.3945.
            - Verified two identical files produce JSD = 0.
            - Verified two fully disjoint files produce JSD = 1.

    - Integration Tests (./compare)
        - Ran ./compare hello.txt bylo.txt on the spec example files and confirmed output of ~0.39451.
        - Ran ./compare on a directory containing .txt files and confirmed recursive traversal collected all files and skipped dotfiles.
        - Ran ./compare on a single file and confirmed the program exits cleanly without output.
        - Ran make asan and re-executed all above tests under AddressSanitizer and UBSan with no reported errors.

3. Design Notes
    - structure
        - Our project consists of the following structs to keep track of one file's data:
            - struct word 
                - char *name
                - int count
                - double frequency
            - struct fileData 
                - char *name
                - struct word *words
                - int uniqueWords
                - int totalWords
                - int capacity
            - struct comparison
                - char *f1
                - char *f2
                - double jsd
                - int totalwords

        - Here are our notes on the three:
            - struct word
                - Stores the name of the word, how many times it appears, and frequency.
            - struct fileData
                - Stores the name of the file, an array of word structs, total amount of unique words, total words, and the capacity of the struct word array.
            - struct comparison
                - Stores the names of two compared files, their Jenson-Shannon Distance, and the total words between the two.

        - Our project performs the following in main():
            - Searches directories and subdirectories in the given path names for .txt files.
            - Creates and compares fileData structs.
            - Sorts the fileData structs in descending order, and prints those results.
            - Cleans up used memory.

    - insert()
        - insert() uses binary search to find the correct insertion point of a word.
            - insert() takes a fileData struct and the name of the word being inserted.
            - This function ensures fileData.words remains alphabetically sorted.

        - Cases
            - Regardless of case, fileData.totalWords is incrememnted by 1.
            - Case 1: The word exists in fileData:
                - We increment word.count and fileData.totalWords.
            - Case 2: The word does not exist:
                - We check if our fileData has enough allocated memory.
                - If not, we realloc to double capacity, and set capacity *= 2.
                - We create the new word struct to hold our word, and increment fileData.uniqueWords and fileData.totalWords.

        - We chose a dynamic array to avoid the cost of a struct word *next pointer per struct word for LinkedList.
            - Instead of having an O(n) node traversal to find our word, we get an O(logn) find. 
            - Unfortunately, we take the hit for inserting unique words. we must move a large portion of memory, rather than updating a *next pointer.
            - This also gives us the freedom to search up any word, as we get indices instead of LinkedList pointer traversal. 

        - Our expected runtime is:
            - O(1), assuming binary search reaches best case
            - O(logn) for a word already in struct
            - o(n + logn) for a unique word

        - insert() does not return.

    - create() 
        - create() initializes a new fileData struct for a given file.
            - create() takes the path of the file, and initializes a fileData struct.
            - create() then performs a tokenizing step on the file.
                - We have a temporary string that we "push" characters into, and "pop" out a word when we reach a space.
                - For each word in the file, create() calls insert().
            - create() runs one final check to ensure the last word in the file is inserted, as it may not have been followed by a non-word character to trigger insertion.
        
        - Upon completion, create() returns the fileData struct for a file, but has all word frequencies set to 0.0.  

    - hasSuffix()
        - hasSuffix() checks to see if a string's last few characters match the given suffix.
            - hasSuffix() is later used in fileSearch().

        - Upon completion, hasSuffix returns 1 if the string matches, and 0 otherwise.

    - fileSearch()
        - fileSearch() either:
            - Takes a file path and adds it to the list of file paths initialized in main()
            - Takes a directory path and recursively searches it for more directories or files.

        - Upon completion, fileSearch() does not return.

    - wfd()
        - wfd() calculates the word frequency distribution of a file.
            - wfd() takes a pointer to a fileData struct.
            - For each word in fileData.words, wfd() sets word.frequency = word.count fileData.totalWords.

        - Our expected runtime is O(n).

        - Upon completion, wfd() does not return.

    - jsd() 
        - jsd() calculates the Jenson-Shannon Distance between two files.
            - jsd() takes two pointers to two fileData structs.
            - jsd() runs the calculations outlined in p2.pdf.
        
        - Upon completion, jsd() returns a double.

    - compare()
        - compare() calculates the difference between two comparison structs' totalWords.
            - compare() is used later in qsort() in main(). 
        
        - Cases
            - Case 1: c1 has more words, so the difference is negative. c1 will be placed first.
            - Case 2: c2 has more words, so the difference is positive. c2 will be placed first.
            - Case 3: totalWords are equal, and qsort() handles sorting.

        - Upon completion, compare() returns the difference as an int.

    - destroyFile() and destroyComparison()
        - destroyFile() and destroyComparison() frees up all memory used for one struct fileData or one struct comparison. 

        - Notably, since we set struct comparison's f1 and f2 equal to the struct fileData's name field, we only have to free the pointer to struct comparison, otherwise we run into a double free error.

        - Upon completion, destroyFile() and destroyComparison() do not return.
