#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include "../common/models.h"
using namespace std;

bool addStudent(int id, const string& name, const string& course,
                const string& phone, string& error);
bool updateStudent(int id, const string& name, const string& course,
                   const string& phone, string& error);
bool deleteStudent(int id, string& error);
const Student* findStudentById(int id);
void displayStudentsInOrder();
int getStudentCount();
int collectStudents(StudentInfo* output, int capacity);
void clearStudents();

#endif
