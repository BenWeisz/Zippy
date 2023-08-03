#include "Zippy.hpp"

int main() {
    zippy_up("../test-folder", "test-folder.zip");
    zippy_down("test-folder.zip");

    return 0;
}