#include "student.h"
#include "../common/validation.h"
#include <iostream>
#include <iomanip>
using namespace std;

static Student* studentRoot = nullptr;
static int studentCount = 0;

static Student* findStudentNode(int id)
{
    Student* current = studentRoot;

    while (current != nullptr)
    {
        if (id == current->id)
        {
            return current;
        }

        current = (id < current->id) ? current->left : current->right;
    }

    return nullptr;
}

static Student* insertStudent(Student* root, Student* node, bool& inserted)
{
    if (root == nullptr)
    {
        inserted = true;
        return node;
    }

    if (node->id < root->id)
    {
        root->left = insertStudent(root->left, node, inserted);
    }
    else if (node->id > root->id)
    {
        root->right = insertStudent(root->right, node, inserted);
    }
    else
    {
        inserted = false;
    }

    return root;
}

static Student* minimumNode(Student* root)
{
    while (root != nullptr && root->left != nullptr)
    {
        root = root->left;
    }

    return root;
}

static Student* deleteStudentNode(Student* root, int id, bool& deleted)
{
    if (root == nullptr)
    {
        return nullptr;
    }

    if (id < root->id)
    {
        root->left = deleteStudentNode(root->left, id, deleted);
    }
    else if (id > root->id)
    {
        root->right = deleteStudentNode(root->right, id, deleted);
    }
    else
    {
        deleted = true;

        if (root->left == nullptr)
        {
            Student* rightChild = root->right;
            delete root;
            return rightChild;
        }

        if (root->right == nullptr)
        {
            Student* leftChild = root->left;
            delete root;
            return leftChild;
        }

        Student* successor = minimumNode(root->right);
        root->id = successor->id;
        root->name = successor->name;
        root->course = successor->course;
        root->phone = successor->phone;

        bool ignored = false;
        root->right = deleteStudentNode(root->right, successor->id, ignored);
    }

    return root;
}

static void displayInOrder(Student* root)
{
    if (root == nullptr)
    {
        return;
    }

    displayInOrder(root->left);

    cout << left
         << setw(6) << root->id
         << setw(24) << fitText(root->name, 23)
         << setw(20) << fitText(root->course, 19)
         << setw(15) << fitText(root->phone, 14) << endl;

    displayInOrder(root->right);
}

static void collectInOrder(Student* root, StudentInfo* output, int capacity, int& index)
{
    if (root == nullptr || index >= capacity)
    {
        return;
    }

    collectInOrder(root->left, output, capacity, index);

    if (index < capacity)
    {
        output[index].id = root->id;
        output[index].name = root->name;
        output[index].course = root->course;
        output[index].phone = root->phone;
        ++index;
    }

    collectInOrder(root->right, output, capacity, index);
}

static void clearTree(Student* root)
{
    if (root == nullptr)
    {
        return;
    }

    clearTree(root->left);
    clearTree(root->right);
    delete root;
}

bool addStudent(int id, const string& name, const string& course,
                const string& phone, string& error)
{
    if (id <= 0)
    {
        error = "Student ID must be a positive integer.";
        return false;
    }

    if (trim(name).empty())
    {
        error = "Student name cannot be empty.";
        return false;
    }

    if (findStudentNode(id) != nullptr)
    {
        error = "A student with this ID already exists.";
        return false;
    }

    Student* node = new Student();
    node->id = id;
    node->name = trim(name);
    node->course = trim(course);
    node->phone = trim(phone);

    bool inserted = false;
    studentRoot = insertStudent(studentRoot, node, inserted);

    if (!inserted)
    {
        delete node;
        error = "A student with this ID already exists.";
        return false;
    }

    ++studentCount;
    return true;
}

bool updateStudent(int id, const string& name, const string& course,
                   const string& phone, string& error)
{
    if (trim(name).empty())
    {
        error = "Student name cannot be empty.";
        return false;
    }

    Student* student = findStudentNode(id);

    if (student == nullptr)
    {
        error = "Student not found.";
        return false;
    }

    student->name = trim(name);
    student->course = trim(course);
    student->phone = trim(phone);
    return true;
}

bool deleteStudent(int id, string& error)
{
    bool deleted = false;
    studentRoot = deleteStudentNode(studentRoot, id, deleted);

    if (!deleted)
    {
        error = "Student not found.";
        return false;
    }

    --studentCount;
    return true;
}

const Student* findStudentById(int id)
{
    return findStudentNode(id);
}

void displayStudentsInOrder()
{
    if (studentRoot == nullptr)
    {
        cout << "No students registered." << endl;
        return;
    }

    cout << left
         << setw(6) << "ID"
         << setw(24) << "Name"
         << setw(20) << "Course"
         << setw(15) << "Phone" << endl;

    cout << string(65, '-') << endl;
    displayInOrder(studentRoot);
}

int getStudentCount()
{
    return studentCount;
}

int collectStudents(StudentInfo* output, int capacity)
{
    int index = 0;
    collectInOrder(studentRoot, output, capacity, index);
    return index;
}

void clearStudents()
{
    clearTree(studentRoot);
    studentRoot = nullptr;
    studentCount = 0;
}
