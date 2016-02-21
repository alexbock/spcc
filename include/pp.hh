#pragma once

#include <vector>
#include <memory>

struct buffer;
struct token;

std::unique_ptr<buffer> perform_pp_phase1(buffer& src);
std::unique_ptr<buffer> perform_pp_phase2(buffer& src);
std::vector<token> perform_pp_phase3(buffer& src);
std::vector<token> perform_pp_phase4(std::vector<token>& tokens);