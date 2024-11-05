#include "StudentManagementSystem.h"
#include <fstream>
#include <random>
#include <ctime>

StudentManagementSystem::StudentManagementSystem() {
    try {
        std::ifstream file("data.bin", std::ios::binary);
        if (file.good()) {
            file.close();
            loadFromFile();
        }
        else {
            std::cout << "data.bin 文件不存在，将创建新的数据文件。" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "初始化学生管理系统时发生错误: " << e.what() << std::endl;
        // 可以在这里添加一些错误恢复逻辑
    }
}

int StudentManagementSystem::binarySearch(const CVector<Student>& vec, int id) const {
    int left = 0, right = vec.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (vec[mid].getId() == id) return mid;
        if (vec[mid].getId() < id) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

int StudentManagementSystem::binarySearch(const CVector<Course>& vec, int id) const {
    int left = 0, right = vec.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (vec[mid].getId() == id) return mid;
        if (vec[mid].getId() < id) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

int StudentManagementSystem::binarySearchByName(const CVector<CVector<int>>& index, const CMyString& name) const {
    int left = 0, right = index.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (mid < 0 || mid >= index.size() || index[mid].empty()) {
            ++left;
            continue;
        }
        int firstIndex = index[mid][0];
        const CMyString& compareName = (firstIndex < m_students.size())
            ? m_students[firstIndex].getName()
            : m_courses[firstIndex - m_students.size()].getName();
        int cmp = name.compare(compareName);
        if (cmp == 0) return mid;
        if (cmp < 0) right = mid - 1;
        else left = mid + 1;
    }
    return -1;
}

void StudentManagementSystem::updateStudentNameIndex(const Student& student, int index) {
    const CMyString& name = student.getName();
 /*   std::cout << "Updating index for student: ID = " << student.getId() << ", Name = '" << name << "', Index = " << index << std::endl;*/

 /*   if (name.empty()) {
        std::cout << "Warning: Student name is empty!" << std::endl;
        return;
    }*/

    int left = 0, right = m_studentNameIndex.size() - 1;
    int insertPos = m_studentNameIndex.size();

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (mid >= m_studentNameIndex.size() || m_studentNameIndex[mid].empty()) {
            right = mid - 1;
            continue;
        }
        int firstStudentIndex = m_studentNameIndex[mid][0];
        if (firstStudentIndex < 0 || firstStudentIndex >= m_students.size()) {
            right = mid - 1;
            continue;
        }
        int cmp = name.compare(m_students[firstStudentIndex].getName());
        if (cmp == 0) {
            m_studentNameIndex[mid].push_back(index);
           /* std::cout << "Updated existing student name index: name = '" << name << "', index = " << index << std::endl;*/
            return;
        }
        else if (cmp < 0) {
            right = mid - 1;
            insertPos = mid;
        }
        else {
            left = mid + 1;
        }
    }

    CVector<int> newEntry;
    newEntry.push_back(index);
    if (insertPos == m_studentNameIndex.size()) {
        m_studentNameIndex.push_back(newEntry);
    }
    else {
        m_studentNameIndex.insert(insertPos, newEntry);
    }
  /*  std::cout << "Added new student name index: name = '" << name << "', index = " << index << std::endl;*/
}

void StudentManagementSystem::updateCourseNameIndex(const Course& course, int index) {
    const CMyString& name = course.getName();
  

    //if (name.empty()) {
    //    std::cout << "Warning: Course name is empty!" << std::endl;
    //    return;
    //}

    int left = 0, right = m_courseNameIndex.size() - 1;
    int insertPos = m_courseNameIndex.size();

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (mid >= m_courseNameIndex.size() || m_courseNameIndex[mid].empty()) {
            right = mid - 1;
            continue;
        }
        int firstCourseIndex = m_courseNameIndex[mid][0] - m_students.size();
        if (firstCourseIndex < 0 || firstCourseIndex >= m_courses.size()) {
            right = mid - 1;
            continue;
        }
        int cmp = name.compare(m_courses[firstCourseIndex].getName());
        if (cmp == 0) {
            m_courseNameIndex[mid].push_back(index + m_students.size());
        /*    std::cout << "Updated existing course name index: name = '" << name << "', index = " << index << std::endl;*/
            return;
        }
        else if (cmp < 0) {
            right = mid - 1;
            insertPos = mid;
        }
        else {
            left = mid + 1;
        }
    }

    CVector<int> newEntry;
    newEntry.push_back(index + m_students.size());
    if (insertPos == m_courseNameIndex.size()) {
        m_courseNameIndex.push_back(newEntry);
    }
    else {
        m_courseNameIndex.insert(insertPos, newEntry);
    }
 /*   std::cout << "Added new course name index: name = '" << name << "', index = " << index << std::endl;*/
}

void StudentManagementSystem::updateCourseGradeIndex(const Grade& grade) {
    int courseIndex = binarySearch(m_courses, grade.getCourseId());
    if (courseIndex != -1) {
        if (courseIndex >= m_courseGradeIndex.size()) {
            m_courseGradeIndex.resize(courseIndex + 1);
        }
        // 检查是否已存在该学生的成绩，如果存在则更新，否则添加
        bool found = false;
        for (auto& existingGrade : m_courseGradeIndex[courseIndex]) {
            if (existingGrade.getStudentId() == grade.getStudentId()) {
                existingGrade = grade;  // 更新现有成绩
                found = true;
                /*std::cout << "更新了课程 " << grade.getCourseId() << " 学生 " << grade.getStudentId() << " 的成绩" << std::endl;*/
                break;
            }
        }
        if (!found) {
            m_courseGradeIndex[courseIndex].push_back(grade);
         /*   std::cout << "添加了课程 " << grade.getCourseId() << " 学生 " << grade.getStudentId() << " 的新成绩" << std::endl;*/
        }
    }
}

void StudentManagementSystem::updateStudentGradeIndex(const Grade& grade) {
    int studentIndex = binarySearch(m_students, grade.getStudentId());
    if (studentIndex != -1) {
        if (studentIndex >= m_studentGradeIndex.size()) {
            m_studentGradeIndex.resize(studentIndex + 1);
        }
        // 检查是否已存在该课程的成绩，如果存在则更新，否则添加
        bool found = false;
        for (auto& existingGrade : m_studentGradeIndex[studentIndex]) {
            if (existingGrade.getCourseId() == grade.getCourseId()) {
                existingGrade = grade;  // 更新现有成绩
                found = true;
               /* std::cout << "更新了学生 " << grade.getStudentId() << " 课程 " << grade.getCourseId() << " 的成绩" << std::endl;*/
                break;
            }
        }
        if (!found) {
            m_studentGradeIndex[studentIndex].push_back(grade);
         /*   std::cout << "添加了学生 " << grade.getStudentId() << " 课程 " << grade.getCourseId() << " 的新成绩" << std::endl;*/
        }
    }
}


CMyString StudentManagementSystem::queryStudentNameById(int studentId) const {
    int index = binarySearch(m_students, studentId);
    if (index != -1) {
        return m_students[index].getName();
    }
    return CMyString();
}

CVector<int> StudentManagementSystem::queryStudentIdByName(const CMyString& studentName) const {
    CVector<int> result;
    std::cout << "Searching for student name: " << studentName << std::endl;
    std::cout << "Total number of name indexes: " << m_studentNameIndex.size() << std::endl;

    int index = binarySearchByName(m_studentNameIndex, studentName);
    std::cout << "Binary search result index: " << index << std::endl;

    if (index != -1) {
        std::cout << "Number of students with this name: " << m_studentNameIndex[index].size() << std::endl;
        for (int i = 0; i < m_studentNameIndex[index].size(); ++i) {
            int studentIndex = m_studentNameIndex[index][i];
            std::cout << "Student index: " << studentIndex << std::endl;
            if (studentIndex >= 0 && studentIndex < m_students.size()) {
                int id = m_students[studentIndex].getId();
                std::cout << "Adding student ID: " << id << std::endl;
                result.push_back(id);
            }
        }
    }
    else {
        std::cout << "No students found with the name: " << studentName << std::endl;
    }
    return result;
}

CMyString StudentManagementSystem::queryCourseNameById(int courseId) const {
    int index = binarySearch(m_courses, courseId);
    if (index != -1) {
        return m_courses[index].getName();
    }
    return CMyString();
}

CVector<int> StudentManagementSystem::queryCourseIdByName(const CMyString& courseName) const {
    CVector<int> result;
    std::cout << "Searching for course name: " << courseName << std::endl;
    std::cout << "Total number of course name indexes: " << m_courseNameIndex.size() << std::endl;

    int index = binarySearchByName(m_courseNameIndex, courseName);
    std::cout << "Binary search result index: " << index << std::endl;

    if (index != -1) {
        std::cout << "Number of courses with this name: " << m_courseNameIndex[index].size() << std::endl;
        for (int i = 0; i < m_courseNameIndex[index].size(); ++i) {
            int courseIndex = m_courseNameIndex[index][i] - m_students.size();
            std::cout << "Course index: " << courseIndex << std::endl;
            if (courseIndex >= 0 && courseIndex < m_courses.size()) {
                int id = m_courses[courseIndex].getId();
                std::cout << "Adding course ID: " << id << std::endl;
                result.push_back(id);
            }
        }
    }
    else {
        std::cout << "No courses found with the name: " << courseName << std::endl;
    }
    return result;
}

CVector<Grade> StudentManagementSystem::queryGradesByCourseId(int courseId) const {
    CVector<Grade> result;
    int index = binarySearch(m_courses, courseId);
    if (index != -1 && index < m_courseGradeIndex.size()) {
        for (const auto& grade : m_courseGradeIndex[index]) {
            if (binarySearch(m_students, grade.getStudentId()) != -1) {
                result.push_back(grade);
            }
        }
    }
    return result;
}

CVector<Grade> StudentManagementSystem::queryGradesByStudentId(int studentId) const {
    int index = binarySearch(m_students, studentId);
    if (index != -1 && index < m_studentGradeIndex.size()) {
        return m_studentGradeIndex[index];
    }
    return CVector<Grade>();
}

// 在现有代码后添加以下函数实现

int StudentManagementSystem::findInsertPosition(const CVector<Student>& vec, int id) const {
    int left = 0, right = vec.size();
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (vec[mid].getId() < id) {
            left = mid + 1;
        }
        else {
            right = mid;
        }
    }
    return left;
}

int StudentManagementSystem::findInsertPosition(const CVector<Course>& vec, int id) const {
    int left = 0, right = vec.size();
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (vec[mid].getId() < id) {
            left = mid + 1;
        }
        else {
            right = mid;
        }
    }
    return left;
}

void StudentManagementSystem::insertStudent(int id, const CMyString& name) {
    if (binarySearch(m_students, id) != -1) {
        throw StudentManagementException("学生ID已存在");
    }
    Student newStudent(id, name);
    if (newStudent.getName().empty()) {
        throw StudentManagementException("学生姓名不能为空");
    }
    int index = findInsertPosition(m_students, id);
    updateStudentNameIndex(newStudent, index);
    m_students.insert(index, newStudent);

    //std::cout << "Inserting student: ID = " << id << ", Name = '" << name << "', Index = " << index << std::endl;
    //std::cout << "Current number of students: " << m_students.size() << std::endl;
    //std::cout << "Current number of name indexes: " << m_studentNameIndex.size() << std::endl;

   

    //std::cout << "After update, number of name indexes: " << m_studentNameIndex.size() << std::endl;

    saveToFile();
}

void StudentManagementSystem::insertCourse(int id, const CMyString& name) {
    if (binarySearch(m_courses, id) != -1) {
        throw StudentManagementException("课程ID已存在");
    }
    Course newCourse(id, name);
    if (newCourse.getName().empty()) {
        throw StudentManagementException("课程名称不能为空");
    }
    int index = findInsertPosition(m_courses, id);
    updateCourseNameIndex(newCourse, index);
    m_courses.insert(index, newCourse);
    saveToFile();
}

void StudentManagementSystem::insertGrade(int studentId, int courseId, float score) {
    if (binarySearch(m_students, studentId) == -1) {
        throw StudentManagementException("学生ID不存在");
    }
    if (binarySearch(m_courses, courseId) == -1) {
        throw StudentManagementException("课程ID不存在");
    }

    // 检查是否已存在该学生的该课程成绩
    for (const auto& grade : m_grades) {
        if (grade.getStudentId() == studentId && grade.getCourseId() == courseId) {
            // 如果已存在，提示用户并返回
            std::cout << "成绩记录已存在，不进行更新" << std::endl;
            return;
        }
    }

    // 如果不存在，添加新的成绩记录
    Grade newGrade(studentId, courseId, score);
    m_grades.push_back(newGrade);
    updateCourseGradeIndex(newGrade);
    updateStudentGradeIndex(newGrade);
    saveToFile();
    std::cout << "新成绩已添加" << std::endl;
}

// 在现有代码后添加以下函数实现

void StudentManagementSystem::deleteStudentById(int studentId) {
    int index = binarySearch(m_students, studentId);
    if (index == -1) {
        throw StudentManagementException("学生ID不存在");
    }
    // 删除学生
    m_students.erase(index);
    // 删除相关的成绩记录
    for (int i = 0; i < m_grades.size(); ) {
        if (m_grades[i].getStudentId() == studentId) {
            m_grades.erase(i);
        }
        else {
            ++i;
        }
    }

    //// 从 m_courseGradeIndex 中删除该学生的成绩记录
    //for (auto& courseGrades : m_courseGradeIndex) {
    //    for (int i = 0; i < courseGrades.size(); ) {
    //        if (courseGrades[i].getStudentId() == studentId) {
    //            courseGrades.erase(i);
    //        }
    //        else {
    //            ++i;
    //        }
    //    }
    //}

    // 更新索引
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::deleteCourseById(int courseId) {
    int index = binarySearch(m_courses, courseId);
    if (index == -1) {
        throw StudentManagementException("课程ID不存在");
    }
    // 删除课程
    m_courses.erase(index);
    // 删除相关的成绩记录
    for (int i = 0; i < m_grades.size(); ) {
        if (m_grades[i].getCourseId() == courseId) {
            m_grades.erase(i);
        }
        else {
            ++i;
        }
    }

    // 从 m_studentGradeIndex 中删除该课程的成绩记录
    for (auto& studentGrades : m_studentGradeIndex) {
        for (int i = 0; i < studentGrades.size(); ) {
            if (studentGrades[i].getCourseId() == courseId) {
                studentGrades.erase(i);
            }
            else {
                ++i;
            }
        }
    }
    // 更新索引
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::deleteGrade(int studentId, int courseId) {
    if (binarySearch(m_students, studentId) == -1) {
        throw StudentManagementException("学生ID不存在");
    }
    if (binarySearch(m_courses, courseId) == -1) {
        throw StudentManagementException("课程ID不存在");
    }
    bool found = false;
    for (int i = 0; i < m_grades.size(); ++i) {
        if (m_grades[i].getStudentId() == studentId && m_grades[i].getCourseId() == courseId) {
            m_grades.erase(i);
            found = true;
            break;
        }
    }
    if (!found) {
        throw StudentManagementException("未找到该学生的该课程成绩");
    }

    // 从 m_courseGradeIndex 中删除该成绩记录
    int courseIndex = binarySearch(m_courses, courseId);
    if (courseIndex != -1 && courseIndex < m_courseGradeIndex.size()) {
        auto& courseGrades = m_courseGradeIndex[courseIndex];
        for (int i = 0; i < courseGrades.size(); ++i) {
            if (courseGrades[i].getStudentId() == studentId) {
                courseGrades.erase(i);
                break;
            }
        }
    }

    // 从 m_studentGradeIndex 中删除该成绩记录
    int studentIndex = binarySearch(m_students, studentId);
    if (studentIndex != -1 && studentIndex < m_studentGradeIndex.size()) {
        auto& studentGrades = m_studentGradeIndex[studentIndex];
        for (int i = 0; i < studentGrades.size(); ++i) {
            if (studentGrades[i].getCourseId() == courseId) {
                studentGrades.erase(i);
                break;
            }
        }
    }

    // 更新索引
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::saveToFile() const {
    std::ofstream file("data.bin", std::ios::binary);

    // 写入学生数量
    size_t studentCount = m_students.size();
    file.write(reinterpret_cast<const char*>(&studentCount), sizeof(size_t));

    // 写入学生数据
    for (const auto& student : m_students) {
        int id = student.getId();
        file.write(reinterpret_cast<const char*>(&id), sizeof(int));
        student.getName().serialize(file);
    }

    // 写入课程数量
    size_t courseCount = m_courses.size();
    file.write(reinterpret_cast<const char*>(&courseCount), sizeof(size_t));

    // 写入课程数据
    for (const auto& course : m_courses) {
        int id = course.getId();
        file.write(reinterpret_cast<const char*>(&id), sizeof(int));
        course.getName().serialize(file);
    }

    // 写入成绩数量
    size_t gradeCount = m_grades.size();
    file.write(reinterpret_cast<const char*>(&gradeCount), sizeof(size_t));

    // 写入成绩数据
    for (const auto& grade : m_grades) {
        int studentId = grade.getStudentId();
        int courseId = grade.getCourseId();
        float score = grade.getScore();
        file.write(reinterpret_cast<const char*>(&studentId), sizeof(int));
        file.write(reinterpret_cast<const char*>(&courseId), sizeof(int));
        file.write(reinterpret_cast<const char*>(&score), sizeof(float));
    }

    file.close();
}

void StudentManagementSystem::loadFromFile() {
    std::ifstream file("data.bin", std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件 data.bin" << std::endl;
        return;
    }

    try {
        // 清空现有数据
        m_students.clear();
        m_courses.clear();
        m_grades.clear();
        m_studentNameIndex.clear();
        m_courseNameIndex.clear();
        m_courseGradeIndex.clear();
        m_studentGradeIndex.clear();

        // 读取学生数据
        size_t studentCount;
        file.read(reinterpret_cast<char*>(&studentCount), sizeof(size_t));
        if (file.fail()) {
            throw std::runtime_error("Failed to read student count");
        }

        m_students.reserve(studentCount);
        for (size_t i = 0; i < studentCount; ++i) {
            int id;
            int nameLength;
            file.read(reinterpret_cast<char*>(&id), sizeof(int));
            file.read(reinterpret_cast<char*>(&nameLength), sizeof(int));

            char* nameBuffer = new char[nameLength + 1];
            file.read(nameBuffer, nameLength);
            nameBuffer[nameLength] = '\0';

            m_students.push_back(Student(id, CMyString(nameBuffer)));
            delete[] nameBuffer;
        }

        // 读取课程数据
        size_t courseCount;
        file.read(reinterpret_cast<char*>(&courseCount), sizeof(size_t));
        if (file.fail()) {
            throw std::runtime_error("Failed to read course count");
        }

        m_courses.reserve(courseCount);
        for (size_t i = 0; i < courseCount; ++i) {
            int id;
            int nameLength;
            file.read(reinterpret_cast<char*>(&id), sizeof(int));
            file.read(reinterpret_cast<char*>(&nameLength), sizeof(int));

            char* nameBuffer = new char[nameLength + 1];
            file.read(nameBuffer, nameLength);
            nameBuffer[nameLength] = '\0';

            m_courses.push_back(Course(id, CMyString(nameBuffer)));
            delete[] nameBuffer;
        }

        // 读取成绩数据
        size_t gradeCount;
        file.read(reinterpret_cast<char*>(&gradeCount), sizeof(size_t));
        if (file.fail()) {
            throw std::runtime_error("Failed to read grade count");
        }

        m_grades.reserve(gradeCount);
        for (size_t i = 0; i < gradeCount; ++i) {
            int studentId, courseId;
            float score;
            file.read(reinterpret_cast<char*>(&studentId), sizeof(int));
            file.read(reinterpret_cast<char*>(&courseId), sizeof(int));
            file.read(reinterpret_cast<char*>(&score), sizeof(float));

            m_grades.push_back(Grade(studentId, courseId, score));
        }

        // 重建索引
        updateAllIndexes();
    }
    catch (const std::exception& e) {
        std::cerr << "加载文件时发生错误: " << e.what() << std::endl;
        // 清空所有数据，以确保数据一致性
        m_students.clear();
        m_courses.clear();
        m_grades.clear();
        m_studentNameIndex.clear();
        m_courseNameIndex.clear();
        m_courseGradeIndex.clear();
        m_studentGradeIndex.clear();
    }

    file.close();
}

void StudentManagementSystem::printAllInformation() const {
    std::cout << "\n===== 学生信息 =====\n";
    for (const auto& student : m_students) {
        std::cout << "ID: " << student.getId() << ", 姓名: " << student.getName() << "\n";
    }

    std::cout << "\n===== 课程信息 =====\n";
    for (const auto& course : m_courses) {
        std::cout << "ID: " << course.getId() << ", 名称: " << course.getName() << "\n";
    }

    std::cout << "\n===== 成绩信息 =====\n";
    for (const auto& grade : m_grades) {
        std::cout << "学生ID: " << grade.getStudentId()
            << ", 课程ID: " << grade.getCourseId()
            << ", 成绩: " << grade.getScore() << "\n";
    }
}

void StudentManagementSystem::formatAndClearData() {
    // 清空所有数据结构
    m_students.clear();
    m_courses.clear();
    m_grades.clear();
    m_studentNameIndex.clear();
    m_courseNameIndex.clear();
    m_courseGradeIndex.clear();
    m_studentGradeIndex.clear();

    // 创建一个空的数据文件
    std::ofstream file("data.bin", std::ios::binary | std::ios::trunc);
    file.close();

    std::cout << "所有数据已被清空，数据文件已被格式化。" << std::endl;
}

void StudentManagementSystem::generateLargeDataset() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    std::uniform_int_distribution<> studentDis(1, 100000);
    std::uniform_int_distribution<> courseDis(1, 1000);

    // 预分配内存
    m_students.reserve(100000);
    m_courses.reserve(1000);
    m_grades.reserve(1000000);

    // 暂时禁用索引更新
    m_indexingEnabled = false;

    // 生成10万名学生
    std::cout << "生成学生数据..." << std::endl;
    for (int i = 1; i <= 100000; ++i) {
        CMyString name("S");
        name.append(i);
        m_students.push_back(Student(i, name));
        if (i % 10000 == 0) {
            std::cout << "已生成 " << i << " 名学生" << std::endl;
        }
    }

    // 生成1000门课程
    std::cout << "生成课程数据..." << std::endl;
    for (int i = 1; i <= 1000; ++i) {
        CMyString name("C");
        name.append(i);
        m_courses.push_back(Course(i, name));
    }

    // 生成100万条选课记录
    std::cout << "生成成绩数据..." << std::endl;
    for (int i = 0; i < 1000000; ++i) {
        int studentId = studentDis(gen);
        int courseId = courseDis(gen);
        float score = static_cast<float>(dis(gen));
        m_grades.push_back(Grade(studentId, courseId, score));
        if ((i + 1) % 100000 == 0) {
            std::cout << "已生成 " << (i + 1) << " 条成绩记录" << std::endl;
        }
    }

    // 重新启用索引更新并更新所有索引
    m_indexingEnabled = true;
    std::cout << "更新索引..." << std::endl;
    updateAllIndexes();

    std::cout << "保存数据到文件..." << std::endl;
    saveToFile();

    std::cout << "大数据集生成完成。" << std::endl;
}

void StudentManagementSystem::batchInsertStudents(const CVector<Student>& students) {
    m_indexingEnabled = false;
    for (const auto& student : students) {
        m_students.push_back(student);
    }
    updateAllIndexes();
    m_indexingEnabled = true;
}

void StudentManagementSystem::batchInsertCourses(const CVector<Course>& courses) {
    m_indexingEnabled = false;
    for (const auto& course : courses) {
        m_courses.push_back(course);
    }
    updateAllIndexes();
    m_indexingEnabled = true;
}

void StudentManagementSystem::batchInsertGrades(const CVector<Grade>& grades) {
    m_indexingEnabled = false;
    for (const auto& grade : grades) {
        m_grades.push_back(grade);
    }
    updateAllIndexes();
    m_indexingEnabled = true;
}

void StudentManagementSystem::updateAllIndexes() {
    // 清空所有索引
    m_studentNameIndex.clear();
    m_courseNameIndex.clear();
    m_courseGradeIndex.clear();
    m_studentGradeIndex.clear();

    // 更新学生姓名索引
    for (int i = 0; i < m_students.size(); ++i) {
        updateStudentNameIndex(m_students[i], i);
    }

    // 更新课程名称索引
    for (int i = 0; i < m_courses.size(); ++i) {
        updateCourseNameIndex(m_courses[i], i);
    }

    // 重新调整索引大小
    m_courseGradeIndex.resize(m_courses.size());
    m_studentGradeIndex.resize(m_students.size());

    // 更新成绩索引
    for (const auto& grade : m_grades) {
        int studentIndex = binarySearch(m_students, grade.getStudentId());
        int courseIndex = binarySearch(m_courses, grade.getCourseId());

        if (studentIndex != -1 && courseIndex != -1) {
            // 检查是否已存在该学生的成绩，如果存在则更新，否则添加
            bool foundInCourse = false;
            for (auto& existingGrade : m_courseGradeIndex[courseIndex]) {
                if (existingGrade.getStudentId() == grade.getStudentId()) {
                    existingGrade = grade;  // 更新现有成绩
                    foundInCourse = true;
                    break;
                }
            }
            if (!foundInCourse) {
                m_courseGradeIndex[courseIndex].push_back(grade);
            }

            // 检查是否已存在该课程的成绩，如果存在则更新，否则添加
            bool foundInStudent = false;
            for (auto& existingGrade : m_studentGradeIndex[studentIndex]) {
                if (existingGrade.getCourseId() == grade.getCourseId()) {
                    existingGrade = grade;  // 更新现有成绩
                    foundInStudent = true;
                    break;
                }
            }
            if (!foundInStudent) {
                m_studentGradeIndex[studentIndex].push_back(grade);
            }
        }
    }
}

void StudentManagementSystem::reserveCapacity(size_t studentCount, size_t courseCount, size_t gradeCount) {
    m_students.reserve(studentCount);
    m_courses.reserve(courseCount);
    m_grades.reserve(gradeCount);
    // 可以根据需要为其他容器预分配内存
}

void StudentManagementSystem::modifyStudentName(int studentId, const CMyString& newName) {
    int index = binarySearch(m_students, studentId);
    if (index == -1) {
        throw StudentManagementException("学生ID不存在");
    }

    CMyString oldName = m_students[index].getName();
    m_students[index] = Student(studentId, newName);

    // 更新学生姓名索引
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::modifyCourseName(int courseId, const CMyString& newName) {
    int index = binarySearch(m_courses, courseId);
    if (index == -1) {
        throw StudentManagementException("课程ID不存在");
    }

    CMyString oldName = m_courses[index].getName();
    m_courses[index] = Course(courseId, newName);

    // 更新课程姓名索引
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::modifyGrade(int studentId, int courseId, float newScore) {
    if (binarySearch(m_students, studentId) == -1) {
        throw StudentManagementException("学生ID不存在");
    }
    if (binarySearch(m_courses, courseId) == -1) {
        throw StudentManagementException("课程ID不存在");
    }

    bool found = false;
    for (auto& grade : m_grades) {
        if (grade.getStudentId() == studentId && grade.getCourseId() == courseId) {
            grade = Grade(studentId, courseId, newScore);
            found = true;
            break;
        }
    }

    if (!found) {
        throw StudentManagementException("未找到该学生的该课程成绩");
    }

     // 更新成绩索引
    for (const auto& grade : m_grades) {
        updateCourseGradeIndex(grade);
        updateStudentGradeIndex(grade);
    }
    saveToFile();
}