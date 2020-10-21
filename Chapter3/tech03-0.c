void normalize_path(char* path) {
    char* currentPtr = path;
    char* normalPtr = path;
    char* beginning = path;
 
    if (*currentPtr == '/') {
        currentPtr++;
        normalPtr++;
    }

    while (*currentPtr != '\0') {
        while (*currentPtr == '/' && *currentPtr != '\0')
            currentPtr++;

        while (*currentPtr == '.') {
            if (*(currentPtr + 1) == '/')
                currentPtr += 2;

            if (*(currentPtr + 1) == '.')
                if (*(currentPtr + 2) == '/' || *(currentPtr + 2) == '\0') {
                    if (*(normalPtr - 1) == '/' && (normalPtr - 1) != beginning) {
                        normalPtr -= 2;

                        while (normalPtr != beginning && *(normalPtr - 1) != '/')
                            normalPtr--;
                    }

                    if (*(currentPtr + 2) != '\0')
                        currentPtr += 3;
                    else currentPtr += 2;
                }
        }

        while (*currentPtr != '\0' && *currentPtr != '/') {
            *normalPtr = *currentPtr;
            currentPtr++;
            normalPtr++;
        }

        if (*currentPtr != '\0') {
            *normalPtr = *currentPtr;
            currentPtr++;
            normalPtr++;
        }
    }
    *normalPtr = '\0';
}
