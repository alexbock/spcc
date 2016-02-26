#include "options.hh"

options::config options::state;

void options::parse(int argc, char** argv) {
    // TODO real options
    for (int i = 1; i < argc; ++i) {
        state.input_filenames.push_back(argv[i]);
    }
}
