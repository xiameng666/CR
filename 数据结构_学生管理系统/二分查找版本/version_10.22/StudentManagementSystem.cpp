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
            std::cout << "data.bin �ļ������ڣ��������µ������ļ���" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "��ʼ��ѧ������ϵͳʱ��������: " << e.what() << std::endl;
        // �������������һЩ����ָ��߼�
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
        // ����Ƿ��Ѵ��ڸ�ѧ���ĳɼ��������������£��������
        bool found = false;
        for (auto& existingGrade : m_courseGradeIndex[courseIndex]) {
            if (existingGrade.getStudentId() == grade.getStudentId()) {
                existingGrade = grade;  // �������гɼ�
                found = true;
                /*std::cout << "�����˿γ� " << grade.getCourseId() << " ѧ�� " << grade.getStudentId() << " �ĳɼ�" << std::endl;*/
                break;
            }
        }
        if (!found) {
            m_courseGradeIndex[courseIndex].push_back(grade);
         /*   std::cout << "����˿γ� " << grade.getCourseId() << " ѧ�� " << grade.getStudentId() << " ���³ɼ�" << std::endl;*/
        }
    }
}

void StudentManagementSystem::updateStudentGradeIndex(const Grade& grade) {
    int studentIndex = binarySearch(m_students, grade.getStudentId());
    if (studentIndex != -1) {
        if (studentIndex >= m_studentGradeIndex.size()) {
            m_studentGradeIndex.resize(studentIndex + 1);
        }
        // ����Ƿ��Ѵ��ڸÿγ̵ĳɼ��������������£��������
        bool found = false;
        for (auto& existingGrade : m_studentGradeIndex[studentIndex]) {
            if (existingGrade.getCourseId() == grade.getCourseId()) {
                existingGrade = grade;  // �������гɼ�
                found = true;
               /* std::cout << "������ѧ�� " << grade.getStudentId() << " �γ� " << grade.getCourseId() << " �ĳɼ�" << std::endl;*/
                break;
            }
        }
        if (!found) {
            m_studentGradeIndex[studentIndex].push_back(grade);
         /*   std::cout << "�����ѧ�� " << grade.getStudentId() << " �γ� " << grade.getCourseId() << " ���³ɼ�" << std::endl;*/
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

// �����д����������º���ʵ��

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
        throw StudentManagementException("ѧ��ID�Ѵ���");
    }
    Student newStudent(id, name);
    if (newStudent.getName().empty()) {
        throw StudentManagementException("ѧ����������Ϊ��");
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
        throw StudentManagementException("�γ�ID�Ѵ���");
    }
    Course newCourse(id, name);
    if (newCourse.getName().empty()) {
        throw StudentManagementException("�γ����Ʋ���Ϊ��");
    }
    int index = findInsertPosition(m_courses, id);
    updateCourseNameIndex(newCourse, index);
    m_courses.insert(index, newCourse);
    saveToFile();
}

void StudentManagementSystem::insertGrade(int studentId, int courseId, float score) {
    if (binarySearch(m_students, studentId) == -1) {
        throw StudentManagementException("ѧ��ID������");
    }
    if (binarySearch(m_courses, courseId) == -1) {
        throw StudentManagementException("�γ�ID������");
    }

    // ����Ƿ��Ѵ��ڸ�ѧ���ĸÿγ̳ɼ�
    for (const auto& grade : m_grades) {
        if (grade.getStudentId() == studentId && grade.getCourseId() == courseId) {
            // ����Ѵ��ڣ���ʾ�û�������
            std::cout << "�ɼ���¼�Ѵ��ڣ������и���" << std::endl;
            return;
        }
    }

    // ��������ڣ�����µĳɼ���¼
    Grade newGrade(studentId, courseId, score);
    m_grades.push_back(newGrade);
    updateCourseGradeIndex(newGrade);
    updateStudentGradeIndex(newGrade);
    saveToFile();
    std::cout << "�³ɼ������" << std::endl;
}

// �����д����������º���ʵ��

void StudentManagementSystem::deleteStudentById(int studentId) {
    int index = binarySearch(m_students, studentId);
    if (index == -1) {
        throw StudentManagementException("ѧ��ID������");
    }
    // ɾ��ѧ��
    m_students.erase(index);
    // ɾ����صĳɼ���¼
    for (int i = 0; i < m_grades.size(); ) {
        if (m_grades[i].getStudentId() == studentId) {
            m_grades.erase(i);
        }
        else {
            ++i;
        }
    }

    //// �� m_courseGradeIndex ��ɾ����ѧ���ĳɼ���¼
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

    // ��������
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::deleteCourseById(int courseId) {
    int index = binarySearch(m_courses, courseId);
    if (index == -1) {
        throw StudentManagementException("�γ�ID������");
    }
    // ɾ���γ�
    m_courses.erase(index);
    // ɾ����صĳɼ���¼
    for (int i = 0; i < m_grades.size(); ) {
        if (m_grades[i].getCourseId() == courseId) {
            m_grades.erase(i);
        }
        else {
            ++i;
        }
    }

    // �� m_studentGradeIndex ��ɾ���ÿγ̵ĳɼ���¼
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
    // ��������
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::deleteGrade(int studentId, int courseId) {
    if (binarySearch(m_students, studentId) == -1) {
        throw StudentManagementException("ѧ��ID������");
    }
    if (binarySearch(m_courses, courseId) == -1) {
        throw StudentManagementException("�γ�ID������");
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
        throw StudentManagementException("δ�ҵ���ѧ���ĸÿγ̳ɼ�");
    }

    // �� m_courseGradeIndex ��ɾ���óɼ���¼
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

    // �� m_studentGradeIndex ��ɾ���óɼ���¼
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

    // ��������
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::saveToFile() const {
    std::ofstream file("data.bin", std::ios::binary);

    // д��ѧ������
    size_t studentCount = m_students.size();
    file.write(reinterpret_cast<const char*>(&studentCount), sizeof(size_t));

    // д��ѧ������
    for (const auto& student : m_students) {
        int id = student.getId();
        file.write(reinterpret_cast<const char*>(&id), sizeof(int));
        student.getName().serialize(file);
    }

    // д��γ�����
    size_t courseCount = m_courses.size();
    file.write(reinterpret_cast<const char*>(&courseCount), sizeof(size_t));

    // д��γ�����
    for (const auto& course : m_courses) {
        int id = course.getId();
        file.write(reinterpret_cast<const char*>(&id), sizeof(int));
        course.getName().serialize(file);
    }

    // д��ɼ�����
    size_t gradeCount = m_grades.size();
    file.write(reinterpret_cast<const char*>(&gradeCount), sizeof(size_t));

    // д��ɼ�����
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
        std::cerr << "�޷����ļ� data.bin" << std::endl;
        return;
    }

    try {
        // �����������
        m_students.clear();
        m_courses.clear();
        m_grades.clear();
        m_studentNameIndex.clear();
        m_courseNameIndex.clear();
        m_courseGradeIndex.clear();
        m_studentGradeIndex.clear();

        // ��ȡѧ������
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

        // ��ȡ�γ�����
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

        // ��ȡ�ɼ�����
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

        // �ؽ�����
        updateAllIndexes();
    }
    catch (const std::exception& e) {
        std::cerr << "�����ļ�ʱ��������: " << e.what() << std::endl;
        // ����������ݣ���ȷ������һ����
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
    std::cout << "\n===== ѧ����Ϣ =====\n";
    for (const auto& student : m_students) {
        std::cout << "ID: " << student.getId() << ", ����: " << student.getName() << "\n";
    }

    std::cout << "\n===== �γ���Ϣ =====\n";
    for (const auto& course : m_courses) {
        std::cout << "ID: " << course.getId() << ", ����: " << course.getName() << "\n";
    }

    std::cout << "\n===== �ɼ���Ϣ =====\n";
    for (const auto& grade : m_grades) {
        std::cout << "ѧ��ID: " << grade.getStudentId()
            << ", �γ�ID: " << grade.getCourseId()
            << ", �ɼ�: " << grade.getScore() << "\n";
    }
}

void StudentManagementSystem::formatAndClearData() {
    // ����������ݽṹ
    m_students.clear();
    m_courses.clear();
    m_grades.clear();
    m_studentNameIndex.clear();
    m_courseNameIndex.clear();
    m_courseGradeIndex.clear();
    m_studentGradeIndex.clear();

    // ����һ���յ������ļ�
    std::ofstream file("data.bin", std::ios::binary | std::ios::trunc);
    file.close();

    std::cout << "���������ѱ���գ������ļ��ѱ���ʽ����" << std::endl;
}

void StudentManagementSystem::generateLargeDataset() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    std::uniform_int_distribution<> studentDis(1, 100000);
    std::uniform_int_distribution<> courseDis(1, 1000);

    // Ԥ�����ڴ�
    m_students.reserve(100000);
    m_courses.reserve(1000);
    m_grades.reserve(1000000);

    // ��ʱ������������
    m_indexingEnabled = false;

    // ����10����ѧ��
    std::cout << "����ѧ������..." << std::endl;
    for (int i = 1; i <= 100000; ++i) {
        CMyString name("S");
        name.append(i);
        m_students.push_back(Student(i, name));
        if (i % 10000 == 0) {
            std::cout << "������ " << i << " ��ѧ��" << std::endl;
        }
    }

    // ����1000�ſγ�
    std::cout << "���ɿγ�����..." << std::endl;
    for (int i = 1; i <= 1000; ++i) {
        CMyString name("C");
        name.append(i);
        m_courses.push_back(Course(i, name));
    }

    // ����100����ѡ�μ�¼
    std::cout << "���ɳɼ�����..." << std::endl;
    for (int i = 0; i < 1000000; ++i) {
        int studentId = studentDis(gen);
        int courseId = courseDis(gen);
        float score = static_cast<float>(dis(gen));
        m_grades.push_back(Grade(studentId, courseId, score));
        if ((i + 1) % 100000 == 0) {
            std::cout << "������ " << (i + 1) << " ���ɼ���¼" << std::endl;
        }
    }

    // ���������������²�������������
    m_indexingEnabled = true;
    std::cout << "��������..." << std::endl;
    updateAllIndexes();

    std::cout << "�������ݵ��ļ�..." << std::endl;
    saveToFile();

    std::cout << "�����ݼ�������ɡ�" << std::endl;
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
    // �����������
    m_studentNameIndex.clear();
    m_courseNameIndex.clear();
    m_courseGradeIndex.clear();
    m_studentGradeIndex.clear();

    // ����ѧ����������
    for (int i = 0; i < m_students.size(); ++i) {
        updateStudentNameIndex(m_students[i], i);
    }

    // ���¿γ���������
    for (int i = 0; i < m_courses.size(); ++i) {
        updateCourseNameIndex(m_courses[i], i);
    }

    // ���µ���������С
    m_courseGradeIndex.resize(m_courses.size());
    m_studentGradeIndex.resize(m_students.size());

    // ���³ɼ�����
    for (const auto& grade : m_grades) {
        int studentIndex = binarySearch(m_students, grade.getStudentId());
        int courseIndex = binarySearch(m_courses, grade.getCourseId());

        if (studentIndex != -1 && courseIndex != -1) {
            // ����Ƿ��Ѵ��ڸ�ѧ���ĳɼ��������������£��������
            bool foundInCourse = false;
            for (auto& existingGrade : m_courseGradeIndex[courseIndex]) {
                if (existingGrade.getStudentId() == grade.getStudentId()) {
                    existingGrade = grade;  // �������гɼ�
                    foundInCourse = true;
                    break;
                }
            }
            if (!foundInCourse) {
                m_courseGradeIndex[courseIndex].push_back(grade);
            }

            // ����Ƿ��Ѵ��ڸÿγ̵ĳɼ��������������£��������
            bool foundInStudent = false;
            for (auto& existingGrade : m_studentGradeIndex[studentIndex]) {
                if (existingGrade.getCourseId() == grade.getCourseId()) {
                    existingGrade = grade;  // �������гɼ�
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
    // ���Ը�����ҪΪ��������Ԥ�����ڴ�
}

void StudentManagementSystem::modifyStudentName(int studentId, const CMyString& newName) {
    int index = binarySearch(m_students, studentId);
    if (index == -1) {
        throw StudentManagementException("ѧ��ID������");
    }

    CMyString oldName = m_students[index].getName();
    m_students[index] = Student(studentId, newName);

    // ����ѧ����������
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::modifyCourseName(int courseId, const CMyString& newName) {
    int index = binarySearch(m_courses, courseId);
    if (index == -1) {
        throw StudentManagementException("�γ�ID������");
    }

    CMyString oldName = m_courses[index].getName();
    m_courses[index] = Course(courseId, newName);

    // ���¿γ���������
    updateAllIndexes();
    saveToFile();
}

void StudentManagementSystem::modifyGrade(int studentId, int courseId, float newScore) {
    if (binarySearch(m_students, studentId) == -1) {
        throw StudentManagementException("ѧ��ID������");
    }
    if (binarySearch(m_courses, courseId) == -1) {
        throw StudentManagementException("�γ�ID������");
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
        throw StudentManagementException("δ�ҵ���ѧ���ĸÿγ̳ɼ�");
    }

     // ���³ɼ�����
    for (const auto& grade : m_grades) {
        updateCourseGradeIndex(grade);
        updateStudentGradeIndex(grade);
    }
    saveToFile();
}