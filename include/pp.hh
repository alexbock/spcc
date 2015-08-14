#pragma once

#include <vector>

struct buffer;
struct pp_token;

buffer perform_pp_phase1(buffer& src);
buffer perform_pp_phase2(buffer& src);
std::vector<pp_token> perform_pp_phase3(buffer& src);
