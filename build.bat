@echo off
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp book/book.cpp student/student.cpp transaction/transaction.cpp search/search.cpp storage/storage.cpp -o library.exe
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
echo Build successful: library.exe
