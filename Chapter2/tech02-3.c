/* Задача, аналогичная print-list-posix, но требуется решение под Windows.

Программе в аргументе командной строки передается имя файла с бинарными данными в Little-Endian.

Файл хранит внутри себя односвязный список элементов:

struct Item
{
  int value;
  uint32_t next_pointer;
};
Поле value храние значение элемента списка, поле next_pointer - позицию в файле (в байтах), указывающую на следующий элемент. Признаком последнего элемента является значение next_pointer, равное 0.

Расположение первого элемента списка (если он существует) - строго в нулевой позиции в файле, расположение остальных - случайным образом.

Выведите на экран значения элементов в списке в текстовом представлении.

Для работы с файлом использовать только низкоуровневый ввод-вывод WinAPI.
*/

#include <windows.h>
#include <stdint.h>
#include <stdio.h>

enum Error {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FILE_OPEN_ERROR = 2,
    BAD_FILE_SIZE = 3,
    CANNOT_GET_NEXT_ITEM = 4
};

struct Item {
    int value;
    uint32_t next_pointer;
};

int getNextItem(HANDLE fd, struct Item* item, uint32_t pointer) {
    SetFilePointer(fd, pointer, NULL, FILE_BEGIN);
    DWORD read = 0;
    ReadFile(fd, item, sizeof(*item), &read, NULL);
    if (read != sizeof(*item))
        return -1;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2)
        return LACK_OF_ARGS;

    HANDLE input = CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (input == INVALID_HANDLE_VALUE)
        return FILE_OPEN_ERROR;

    struct Item item = {0, 0};
    if (GetFileSize(input, NULL) < sizeof(item)) {
        CloseHandle(input);
        return BAD_FILE_SIZE;
    }

    do {
        int errcode = getNextItem(input, &item, item.next_pointer);
        if (errcode == -1) {
            CloseHandle(input);
            return CANNOT_GET_NEXT_ITEM;
        }

        printf ("%d ", item.value);
    } while (item.next_pointer != 0);

    CloseHandle(input);
    return ALLRIGHT;
}
