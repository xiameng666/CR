#include "UserInterface.h"
#include "StudentManagementSystem.h"

int main() {
    StudentManagementSystem system;
    UserInterface ui(system);
    ui.run();
  /*  ui.runAutomatedTest();*/
    return 0;
}