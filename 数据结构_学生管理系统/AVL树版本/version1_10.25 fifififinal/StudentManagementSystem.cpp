#include "StudentManagementSystem.h"
#include <fstream>
#include <iostream>
#include <random>
#include <algorithm>


// 查询功能实现
Student* StudentManagementSystem::queryStudentById(int id) {
    auto node = studentById.FindNode(Pair<int, Student>(id));
    return node ? &(node->m_Val.m_Value) : nullptr;
}

Student* StudentManagementSystem::queryStudentByName(const CMyString& name) {
    auto node = studentByName.FindNode(Pair<CMyString, Student>(name));
    return node ? &(node->m_Val.m_Value) : nullptr;
}

Course* StudentManagementSystem::queryCourseById(int id) {
    auto node = courseById.FindNode(Pair<int, Course>(id));
    return node ? &(node->m_Val.m_Value) : nullptr;
}

Course* StudentManagementSystem::queryCourseByName(const CMyString& name) {
    auto node = courseByName.FindNode(Pair<CMyString, Course>(name));
    return node ? &(node->m_Val.m_Value) : nullptr;
}

void StudentManagementSystem::queryStudentsByCourseId(int courseId) {
    auto node = gradeByCourseId.FindNode(Pair<int, Grade>(courseId));
    if (!node) {
        std::cout << "未到该课程的选课记录" << std::endl;
        return;
    }

    std::cout << "选修程ID " << courseId << " 的学生：" << std::endl;
    while (node) {
        const Grade& grade = node->m_Val.m_Value;
        Student* student = queryStudentById(grade.getStudentId());
        if (student) {
            std::cout << "  学生ID: " << student->getId()
                << ", 姓名: " << student->getName()
                << ", 成绩: " << grade.getScore() << std::endl;
        }
        node = node->m_pNext;
    }
}

void StudentManagementSystem::queryCoursesByStudentId(int studentId) {
    auto node = gradeByStudentId.FindNode(Pair<int, Grade>(studentId));
    while (node) {
        const Grade& grade = node->m_Val.m_Value;
        Course* course = queryCourseById(grade.getCourseId());
        if (course) {
            std::cout << "课程ID: " << course->getId()
                << ", 名称: " << course->getName()
                << ", 成绩: " << grade.getScore() << std::endl;
        }
        node = node->m_pNext;
    }
}

// 插入功能实现
void StudentManagementSystem::insertStudent(int id, const CMyString& name) {
    auto existingStudent = queryStudentById(id);
    if (existingStudent) {
        std::cout << "添加失败：学生ID " << id << " 已存在。" << std::endl;
        return;
    }
    Student student(id, name);
    studentById.Insert(Pair<int, Student>(id, student));
    studentByName.Insert(Pair<CMyString, Student>(name, student));
    autoSave();
}

void StudentManagementSystem::insertCourse(int id, const CMyString& name) {
    auto existingCourse = queryCourseById(id);
    if (existingCourse) {
        std::cout << "添加失败：课程ID " << id << " 已存在。" << std::endl;
        return;
    }
    Course course(id, name);
    courseById.Insert(Pair<int, Course>(id, course));
    courseByName.Insert(Pair<CMyString, Course>(name, course));
    autoSave();
}

void StudentManagementSystem::insertGrade(int studentId, int courseId, float score) {
    auto existingStudent = queryStudentById(studentId);
    auto existingCourse = queryCourseById(courseId);
    if (!existingStudent) {
        std::cout << "添加失败：学生ID " << studentId << " 不存在。" << std::endl;
        return;
    }
    if (!existingCourse) {
        std::cout << "添加失败：课程ID " << courseId << " 不存在。" << std::endl;
        return;
    }

    auto gradeNode = gradeByStudentId.FindNode(Pair<int, Grade>(studentId));
    while (gradeNode) {
        if (gradeNode->m_Val.m_Value.getCourseId() == courseId) {
            std::cout << "添加失败：该学生的该课程成绩已存在。" << std::endl;
            return;
        }
        gradeNode = gradeNode->m_pNext;
    }

    Grade grade(studentId, courseId, score);
    gradeByStudentId.Insert(Pair<int, Grade>(studentId, grade));
    gradeByCourseId.Insert(Pair<int, Grade>(courseId, grade));
    autoSave();
}

void StudentManagementSystem::removeStudentById(int studentId) {
    auto node = studentById.FindNode(Pair<int, Student>(studentId));

    if (node) {
        Student student = node->m_Val.m_Value;

        // 从 studentByName 中删除
        studentByName.Remove(Pair<CMyString, Student>(student.getName(), student));
              
        // 删除该学生的所有成绩记录
        while (true) {
            auto gradeNode = gradeByStudentId.FindNode(Pair<int, Grade>(studentId));
            if (!gradeNode) break;  // 如果没有找到更多的成绩记录，退出循环

            Grade grade = gradeNode->m_Val.m_Value;
            int courseId = gradeNode->m_Val.m_Value.getCourseId();
            // 从 gradeByCourseId 中删除对应的成绩记录
    /*        gradeByCourseId.Remove(Pair<int, Grade>(courseId), Pair<int, Grade>(studentId, gradeNode->m_Val.m_Value));*/
            gradeByCourseId.Remove(Pair<int, Grade>(courseId,grade));
           
            // 从 gradeByStudentId 中删除成绩记录
            //gradeByStudentId.Remove(Pair<int, Grade>(studentId), Pair<int, Grade>(studentId, gradeNode->m_Val.m_Value));
            gradeByStudentId.Remove(Pair<int, Grade>(studentId,grade));
        }

        // 最后删除学生记录
        studentById.Remove(Pair<int, Student>(studentId, student));

        std::cout << "学生 (ID: " << studentId << ", 姓名: " << student.getName() << ") 已成功删除。" << std::endl;
        saveToFile();
    }
    else {
        std::cout << "删除失败：学生ID " << studentId << " 不存在。" << std::endl;
    }
}

void StudentManagementSystem::removeCourseById(int courseId) {
    auto node = courseById.FindNode(Pair<int, Course>(courseId));
    if (node) {
        Course course = node->m_Val.m_Value;

        // 从 courseByName 中删除
        courseByName.Remove(Pair<CMyString, Course>(course.getName(),course));

        // 删除该课程的所有成绩记录
       
        while (true) {
            auto gradeNode = gradeByCourseId.FindNode(Pair<int, Grade>(courseId));
            if (!gradeNode) break;

            Grade grade = gradeNode->m_Val.m_Value;
            int studentId = gradeNode->m_Val.m_Value.getStudentId();

            // 从 gradeByStudentId 中删除对应的成绩记录
            gradeByStudentId.Remove(Pair<int, Grade>(studentId,grade));
          /*  gradeByStudentId.Remove(Pair<int, Grade>(studentId), Pair<int, Grade>(studentId, gradeNode->m_Val.m_Value));*/
            // 删除课程的所有成绩记录
            gradeByCourseId.Remove(Pair<int, Grade>(courseId,grade));
            /*gradeByCourseId.Remove(Pair<int, Grade>(courseId), Pair<int, Grade>(studentId, gradeNode->m_Val.m_Value));*/
        }
    

        // 最后删除课程记录
        courseById.Remove(Pair<int, Course>(courseId, course));

        std::cout << "课程 (ID: " << courseId << ", 名称: " << course.getName() << ") 已成功删除。" << std::endl;
        saveToFile();
    }
    else {
        std::cout << "删除失败：课程ID " << courseId << " 不存在。" << std::endl;
    }
}


//删除成绩仅需要删除一条记录
void StudentManagementSystem::removeGrade(int studentId, int courseId) {
    Grade gradeToRemove(studentId, courseId, 0);  // 成绩值不重要，只比较学生ID和课程ID

    // 检查成绩是否存在
    auto studentGradeNode = gradeByStudentId.FindNode(Pair<int, Grade>(studentId, gradeToRemove));
    bool gradeExists = false;
    while (studentGradeNode) {
        if (studentGradeNode->m_Val.m_Value.getCourseId() == courseId) {
            gradeExists = true;
            break;
        }
        studentGradeNode = studentGradeNode->m_pNext;
    }

    if (gradeExists) {
        gradeByStudentId.Remove(Pair<int, Grade>(studentId, gradeToRemove));
        gradeByCourseId.Remove(Pair<int, Grade>(courseId, gradeToRemove));
        std::cout << "学生ID " << studentId << " 的课程ID " << courseId << " 的成绩记录已成功删除。" << std::endl;
        saveToFile();
    }
    else {
        std::cout << "删除失败：未找到学生ID " << studentId << " 的课程ID " << courseId << " 的成绩记录。" << std::endl;
    }
}


void StudentManagementSystem::modifyStudent(int id, const CMyString& newName) {
    Student newStudent(id, newName);
    auto nodeById = studentById.FindNode(Pair<int, Student>(id));

    if (nodeById) {
        Student oldStudent = nodeById->m_Val.m_Value;

        // 从 studentByName 中移除旧记录
        studentByName.Remove(Pair<CMyString, Student>(oldStudent.getName(), oldStudent));
        
        // 更新 studentById 中的记录
        nodeById->m_Val.m_Value = newStudent;

        // 在 studentByName 中插入更新后的记录
        studentByName.Insert(Pair<CMyString, Student>(newName, newStudent));

        std::cout << "学生信息已成功修改。" << std::endl;
        autoSave();
    }
    else {
        std::cout << "修改失败：学生ID " << id << " 不存在。" << std::endl;
    }
}

void StudentManagementSystem::modifyCourse(int id, const CMyString& newName) {
    auto node = courseById.FindNode(Pair<int, Course>(id));
    if (node) {
        Course oldCourse = node->m_Val.m_Value;
        courseByName.Remove(Pair<CMyString, Course>(oldCourse.getName(), oldCourse));

        Course newCourse(id, newName);
        node->m_Val.m_Value = newCourse;
        courseByName.Insert(Pair<CMyString, Course>(newName, newCourse));
        autoSave();
    }
}

void StudentManagementSystem::modifyGrade(int studentId, int courseId, float newScore) {
    Grade newGrade(studentId, courseId, newScore);
    auto nodeStudent = gradeByStudentId.FindNode(Pair<int, Grade>(studentId,newGrade));
    auto nodeCourse = gradeByCourseId.FindNode(Pair<int, Grade>(courseId, newGrade));
    
    if (nodeStudent && nodeCourse) {
        nodeStudent->m_Val.m_Value.setScore(newScore);
        nodeCourse->m_Val.m_Value.setScore(newScore);
        autoSave();
    }
}

// 文件操作实现
void StudentManagementSystem::saveToFile() {
    // 保存学生数据
    std::ofstream studentFile("students.dat", std::ios::binary);
    if (studentFile.is_open()) {
        auto root = studentById.GetRoot();
        saveStudentNode(studentFile, root);
        studentFile.close();
    }

    // 保存课程数据
    std::ofstream courseFile("courses.dat", std::ios::binary);
    if (courseFile.is_open()) {
        auto root = courseById.GetRoot();
        saveCourseNode(courseFile, root);
        courseFile.close();
    }

    // 保存成绩数据
    std::ofstream gradeFile("grades.dat", std::ios::binary | std::ios::trunc);
    if (gradeFile.is_open()) {
        auto root = gradeByStudentId.GetRoot();
        saveGradeNode(gradeFile, root);
        gradeFile.close();
    }
}

void StudentManagementSystem::loadFromFile() {
    loadStudentData("students.dat");
    loadCourseData("courses.dat");
    loadGradeData("grades.dat");
}

void StudentManagementSystem::loadStudentData(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "无法打开文件: " << filename << std::endl;
        return;
    }

    // 清空现有数据
    studentById.Clear(studentById.GetRoot());
    studentByName.Clear(studentByName.GetRoot());

    while (file.good()) {
        int id;
        file.read(reinterpret_cast<char*>(&id), sizeof(int));
        if (file.eof()) break;

        int nameLength;
        file.read(reinterpret_cast<char*>(&nameLength), sizeof(int));

        if (nameLength < 0 || nameLength > 1000000) {
            std::cout << "无效的名字长度: " << nameLength << std::endl;
            break;
        }

        char* buffer = new char[nameLength + 1];
        file.read(buffer, nameLength);
        buffer[nameLength] = '\0';

        CMyString name(buffer);
        delete[] buffer;

        Student student(id, name);
        studentById.Insert(Pair<int, Student>(id, student));

        // 检查是否已存在同名学生
        auto existingNode = studentByName.FindNode(Pair<CMyString, Student>(name));
        if (existingNode) {
            // 如果存在同名学生，将新学生添加到链表中
            auto newNode = new CAVLTree<Pair<CMyString, Student>>::TreeNode(Pair<CMyString, Student>(name, student));
            newNode->m_pNext = existingNode->m_pNext;
            existingNode->m_pNext = newNode;
        }
        else {
            // 如果不存在同名学，直接插入
            studentByName.Insert(Pair<CMyString, Student>(name, student));
        }
    }

    file.close();
}

void StudentManagementSystem::loadCourseData(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "无法打开文件: " << filename << std::endl;
        return;
    }

    // 清空现有数据
    courseById.Clear(courseById.GetRoot());
    courseByName.Clear(courseByName.GetRoot());

    while (file.good()) {
        int id;
        file.read(reinterpret_cast<char*>(&id), sizeof(int));
        if (file.eof()) break;

        int nameLength;
        file.read(reinterpret_cast<char*>(&nameLength), sizeof(int));

        if (nameLength < 0 || nameLength > 1000000) {
            std::cout << "无效的课程名称长度: " << nameLength << std::endl;
            break;
        }

        char* buffer = new char[nameLength + 1];
        file.read(buffer, nameLength);
        buffer[nameLength] = '\0';

        CMyString name(buffer);
        delete[] buffer;

        Course course(id, name);
        courseById.Insert(Pair<int, Course>(id, course));
        courseByName.Insert(Pair<CMyString, Course>(name, course));
    }

    file.close();
}

void StudentManagementSystem::loadGradeData(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "无法打开文件: " << filename << std::endl;
        return;
    }

    // 清空现有数据
    gradeByStudentId.Clear(gradeByStudentId.GetRoot());
    gradeByCourseId.Clear(gradeByCourseId.GetRoot());

    while (file.good()) {
        int studentId, courseId;
        float score;

        file.read(reinterpret_cast<char*>(&studentId), sizeof(int));
        if (file.eof()) break;

        file.read(reinterpret_cast<char*>(&courseId), sizeof(int));
        file.read(reinterpret_cast<char*>(&score), sizeof(float));

        Grade grade(studentId, courseId, score);
        gradeByStudentId.Insert(Pair<int, Grade>(studentId, grade));
        gradeByCourseId.Insert(Pair<int, Grade>(courseId, grade));
    }

    file.close();
}

// 助函数实现
void StudentManagementSystem::saveStudentNode(std::ofstream& file, CAVLTree<Pair<int, Student>>::TreeNode* node) {
    if (node == nullptr) return;

    const Student& student = node->m_Val.m_Value;
    int id = student.getId();
    file.write(reinterpret_cast<const char*>(&id), sizeof(int));
    student.getName().serialize(file);

    saveStudentNode(file, node->m_pLeft);
    saveStudentNode(file, node->m_pRight);
}

void StudentManagementSystem::saveCourseNode(std::ofstream& file, CAVLTree<Pair<int, Course>>::TreeNode* node) {
    if (node == nullptr) return;

    const Course& course = node->m_Val.m_Value;
    int id = course.getId();
    file.write(reinterpret_cast<const char*>(&id), sizeof(int));
    course.getName().serialize(file);

    saveCourseNode(file, node->m_pLeft);
    saveCourseNode(file, node->m_pRight);
}

void StudentManagementSystem::saveGradeNode(std::ofstream& file, CAVLTree<Pair<int, Grade>>::TreeNode* node) {
    if (node == nullptr) return;

    saveGradeNode(file, node->m_pLeft);

    // 保存当前节点及其链表中的所有成绩记录
    auto current = node;
    while (current != nullptr) {
        const Grade& grade = current->m_Val.m_Value;
        int studentId = grade.getStudentId();
        int courseId = grade.getCourseId();
        float score = grade.getScore();
        file.write(reinterpret_cast<const char*>(&studentId), sizeof(int));
        file.write(reinterpret_cast<const char*>(&courseId), sizeof(int));
        file.write(reinterpret_cast<const char*>(&score), sizeof(float));
        current = current->m_pNext;
    }

    saveGradeNode(file, node->m_pRight);
}

// 构造函数
StudentManagementSystem::StudentManagementSystem() {
    loadFromFile();
}

// 自动保存函数
void StudentManagementSystem::autoSave() {
    saveToFile();
}

// 清空所有数据
void StudentManagementSystem::clearAllData() {
    std::ofstream studentFile("students.dat", std::ios::trunc);
    studentFile.close();
    std::ofstream courseFile("courses.dat", std::ios::trunc);
    courseFile.close();
    std::ofstream gradeFile("grades.dat", std::ios::trunc);
    gradeFile.close();

    std::cout << "所有数据文件已清空。" << std::endl;
}

// 显示所有数据
void StudentManagementSystem::displayAllData() {
    std::cout << "学生信息：" << std::endl;
    auto studentRoot = studentById.GetRoot();
    if (studentRoot) {
        displayStudentNode(studentRoot);
    }
    else {
        std::cout << "  (无学生数据)" << std::endl;
    }

    std::cout << "\n课程信息：" << std::endl;
    auto courseRoot = courseById.GetRoot();
    if (courseRoot) {
        displayCourseNode(courseRoot);
    }
    else {
        std::cout << "  (无课程数据)" << std::endl;
    }

    std::cout << "\n成绩信息：" << std::endl;
    auto gradeRoot = gradeByStudentId.GetRoot();
    if (gradeRoot) {
        displayGradeNode(gradeRoot);
    }
    else {
        std::cout << "  (无成绩数据)" << std::endl;
    }
}

void StudentManagementSystem::displayStudentNode(CAVLTree<Pair<int, Student>>::TreeNode* node) {
    if (node == nullptr) return;
    displayStudentNode(node->m_pLeft);
    std::cout << "  " << node->m_Val.m_Value << std::endl;
    displayStudentNode(node->m_pRight);
}

void StudentManagementSystem::displayCourseNode(CAVLTree<Pair<int, Course>>::TreeNode* node) {
    if (node == nullptr) return;
    displayCourseNode(node->m_pLeft);
    std::cout << "  " << node->m_Val.m_Value << std::endl;
    displayCourseNode(node->m_pRight);
}

void StudentManagementSystem::displayGradeNode(CAVLTree<Pair<int, Grade>>::TreeNode* node) {
    if (node == nullptr) return;
    displayGradeNode(node->m_pLeft);

    // 显示当前节点的成绩信息
    auto current = node;
    while (current) {
        std::cout << "  " << current->m_Val.m_Value << std::endl;
        current = current->m_pNext;
    }

    displayGradeNode(node->m_pRight);
}

CVector<Student*> StudentManagementSystem::queryStudentsByName(const CMyString& name) {
    CVector<Student*> result;
    auto node = studentByName.FindNode(Pair<CMyString, Student>(name));
    while (node) {
        result.push_back(&(node->m_Val.m_Value));
        node = node->m_pNext;
    }
    return result;
}

CVector<Course*> StudentManagementSystem::queryCoursesByName(const CMyString& name) {
    CVector<Course*> result;
    auto node = courseByName.FindNode(Pair<CMyString, Course>(name));
    while (node) {
        result.push_back(&(node->m_Val.m_Value));
        node = node->m_pNext;
    }
    return result;
}

void StudentManagementSystem::queryStudent(int id, const CMyString& name) {
    if (id != -1) {
        auto node = studentById.FindNode(Pair<int, Student>(id));
        if (node) {
            std::cout << node->m_Val.m_Value << std::endl;
        }
        else {
            std::cout << "未找到ID为 " << id << " 的学生。" << std::endl;
        }
    }
    else if (!name.empty()) {
        auto node = studentByName.FindNode(Pair<CMyString, Student>(name));
        if (node) {
            while (node) {
                std::cout << node->m_Val.m_Value << std::endl;
                node = node->m_pNext;
            }
        }
        else {
            std::cout << "未找到姓名为 " << name << " 的学生。" << std::endl;
        }
    }
    else {
        std::cout << "请提供学生ID或姓名进行查询。" << std::endl;
    }
}

void StudentManagementSystem::queryCourse(int id, const CMyString& name) {
    if (id != -1) {
        auto node = courseById.FindNode(Pair<int, Course>(id));
        if (node) {
            std::cout << node->m_Val.m_Value << std::endl;
        }
        else {
            std::cout << "未找到ID为 " << id << " 的课程。" << std::endl;
        }
    }
    else if (!name.empty()) {
        auto node = courseByName.FindNode(Pair<CMyString, Course>(name));
        if (node) {
            while (node) {
                std::cout << node->m_Val.m_Value << std::endl;
                node = node->m_pNext;
            }
        }
        else {
            std::cout << "未找到名称为 " << name << " 的课程。" << std::endl;
        }
    }
    else {
        std::cout << "请提供课程ID或名称进行查询。" << std::endl;
    }
}

void StudentManagementSystem::queryGrade(int studentId, int courseId) {
    if (studentId != -1 && courseId != -1) {
        // 查询特定学生的特定课程成绩
        auto gradeNode = gradeByStudentId.FindNode(Pair<int, Grade>(studentId));
        bool found = false;
        while (gradeNode) {
            if (gradeNode->m_Val.m_Value.getCourseId() == courseId) {
                std::cout << gradeNode->m_Val.m_Value << std::endl;
                found = true;
                break;
            }
            gradeNode = gradeNode->m_pNext;
        }
        if (!found) {
            std::cout << "未找到学生ID " << studentId << " 的课程ID " << courseId << " 的成绩记录。" << std::endl;
        }
    }
    else if (studentId != -1) {
        // 查询特定学生的所有成绩
        auto gradeNode = gradeByStudentId.FindNode(Pair<int, Grade>(studentId));
        if (gradeNode) {
            while (gradeNode) {
                std::cout << gradeNode->m_Val.m_Value << std::endl;
                gradeNode = gradeNode->m_pNext;
            }
        }
        else {
            std::cout << "未找到学生ID " << studentId << " 的任何成绩记录。" << std::endl;
        }
    }
    else if (courseId != -1) {
        // 查询特定课程的所有成绩
        auto gradeNode = gradeByCourseId.FindNode(Pair<int, Grade>(courseId));
        if (gradeNode) {
            while (gradeNode) {
                //// 确只打印有效的成绩记录
                //Student* student = queryStudentById(gradeNode->m_Val.m_Value.getStudentId());
                //if (student) {
                //    std::cout << "学生ID: " << student->getId()
                //        << ", 姓名: " << student->getName()
                //        << ", 课程ID: " << courseId
                //        << ", 成绩: " << gradeNode->m_Val.m_Value.getScore() << std::endl;
                //}
                std::cout << gradeNode->m_Val.m_Value << std::endl;
                gradeNode = gradeNode->m_pNext;
            }
        }
        else {
            std::cout << "未找到课程ID " << courseId << " 的任何成绩记录。" << std::endl;
        }
    }
    else {
        std::cout << "请提供学生ID或课程ID进行查询。" << std::endl;
    }
}

void StudentManagementSystem::generateTestData() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 100);

    // 直接写入学生数据
    std::ofstream studentFile("students.dat", std::ios::binary);
    for (int i = 1; i <= 100000; ++i) {
        int id = i;
        /*CMyString name = CMyString("S") + CMyString::to_string(i);*/
        CMyString name =CMyString::to_string(i);
        studentFile.write(reinterpret_cast<const char*>(&id), sizeof(int));
        int nameLength = name.length();
        studentFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(int));
        studentFile.write(name.c_str(), nameLength);
    }
    studentFile.close();

    // 直接写入程数据
    std::ofstream courseFile("courses.dat", std::ios::binary);
    for (int i = 1; i <= 1000; ++i) {
        int id = i;
       /* CMyString name = CMyString("C") + CMyString::to_string(i);*/
        CMyString name = CMyString::to_string(i);
        courseFile.write(reinterpret_cast<const char*>(&id), sizeof(int));
        int nameLength = name.length();
        courseFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(int));
        courseFile.write(name.c_str(), nameLength);
    }
    courseFile.close();

    // ???选课记录
    std::vector<std::pair<int, int>> allPairs;
    for (int studentId = 1; studentId <= 100000; ++studentId) {
        for (int courseId = 1; courseId <= 1000; ++courseId) {
            allPairs.emplace_back(studentId, courseId);
        }
    }
    std::shuffle(allPairs.begin(), allPairs.end(), gen);

    // 直接写入成绩数据
    std::ofstream gradeFile("grades.dat", std::ios::binary);
    for (int i = 0; i < 1000000; ++i) {
        int studentId = allPairs[i].first;
        int courseId = allPairs[i].second;
        float score = static_cast<float>(dis(gen));
        gradeFile.write(reinterpret_cast<const char*>(&studentId), sizeof(int));
        gradeFile.write(reinterpret_cast<const char*>(&courseId), sizeof(int));
        gradeFile.write(reinterpret_cast<const char*>(&score), sizeof(float));
    }
    gradeFile.close();

    std::cout << "生成了 100,000 名学生，1,000 门课程和 1,000,000 条选课记录。" << std::endl;

    // 重新加载数据到内存
    loadFromFile();
}

void StudentManagementSystem::insertSampleData() {
    // 插入学生数据
    insertStudent(100001, "王小二");
    insertStudent(100002, "李四四四四");
    insertStudent(100003, "王小二");

    // 插入课程数据
    insertCourse(1001, "C++");
    insertCourse(1002, "16位微机");
    insertCourse(1003, "C++");

    // 插入成绩数据
    insertGrade(100001, 1001, 61);
    insertGrade(100001, 1002, 62);
    insertGrade(100001, 1003, 63);
    insertGrade(100002, 1001, 71);
    insertGrade(100002, 1002, 72);
    insertGrade(100002, 1003, 73);
    insertGrade(100003, 1001, 81);
    insertGrade(100003, 1002, 82);
    insertGrade(100003, 1003, 83);

    std::cout << "示例数据已成功插入。" << std::endl;
}

void StudentManagementSystem::PrintStudentTreeByName() {
    std::cout << "Student Tree (by Name):" << std::endl;
    PrintTreeByName(studentByName.GetRoot(), 0);
}

void StudentManagementSystem::PrintCourseTreeByName() {
    std::cout << "Course Tree (by Name):" << std::endl;
    PrintTreeByName(courseByName.GetRoot(), 0);
}

void StudentManagementSystem::PrintTreeByName(CAVLTree<Pair<CMyString, Student>>::TreeNode* node, int depth) {
    if (node == nullptr) {
        return;
    }

    PrintTreeByName(node->m_pRight, depth + 1);

    std::string indent(depth * 4, ' ');
    std::cout << indent << node->m_Val.m_Key << ": ";

    auto current = node;
    while (current != nullptr) {
        std::cout << current->m_Val.m_Value.getId() << " ";
        current = current->m_pNext;
    }
    std::cout << std::endl;

    PrintTreeByName(node->m_pLeft, depth + 1);
}

void StudentManagementSystem::PrintTreeByName(CAVLTree<Pair<CMyString, Course>>::TreeNode* node, int depth) {
    if (node == nullptr) {
        return;
    }

    PrintTreeByName(node->m_pRight, depth + 1);

    std::string indent(depth * 4, ' ');
    std::cout << indent << node->m_Val.m_Key << ": ";

    auto current = node;
    while (current != nullptr) {
        std::cout << current->m_Val.m_Value.getId() << " ";
        current = current->m_pNext;
    }
    std::cout << std::endl;

    PrintTreeByName(node->m_pLeft, depth + 1);
}
