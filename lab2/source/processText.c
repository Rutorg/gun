#if !defined(MAX_SIZE)
#define MAX_SIZE 1024
#endif // MAX_SIZE

#include <string.h>

int processText(char *sourceText, char *resultText, int fileSize, const int MAX_REPLACES) {
    int replaceCounter = 0;
    int currentLineStart = 0;
    int maxReplacementsFlag = 0;
    
    for (size_t i = 0; i < fileSize; i++) {
        resultText[i] = sourceText[i];
        if (!maxReplacementsFlag && resultText[i] == '\n') {
            char lastSymbolInLine = resultText[i - 1];
            
            // if lastSymbolInLine is space then there will be no replaces.
            if (lastSymbolInLine != ' ') {
                for (size_t j = currentLineStart; j < i; j++) {
                    if (resultText[j] == lastSymbolInLine) {
                        resultText[j] = ' ';
                        replaceCounter++;
                        if (replaceCounter == MAX_REPLACES) {
                            maxReplacementsFlag = 1;
                            break;
                        }
                    }
                }
            }
            
            currentLineStart = i + 1;
        }
    }
    
    // process last line
    if (!maxReplacementsFlag && resultText[fileSize - 1] != '\n') {
        char lastSymbolInLine = resultText[fileSize - 1];
        
        // if lastSymbolInLine is space then there will be no replaces.
        if (lastSymbolInLine != ' ') {
            for (size_t j = currentLineStart; j < fileSize; j++) {
                if (resultText[j] == lastSymbolInLine) {
                    resultText[j] = ' ';
                    replaceCounter++;
                }
            }
        }
    } 
    
    
    // printf("%d\n", replaceCounter);
    return replaceCounter;
}