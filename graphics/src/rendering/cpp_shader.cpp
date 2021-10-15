#include <lit/rendering/cpp_shader.hpp>

#include <functional>
#include <fstream>
#include <regex>

void ReplaceAll(std::string &str, const std::string &source, const std::string &target) {
    auto regex_source = std::regex(source);
    while (std::regex_search(str, regex_source)) {
        str = std::regex_replace(str, regex_source, target);
    }
}

void ForAllMatches(const std::string &str, const std::string &regex,
                   const std::function<void(const std::vector<std::string>)> &func) {
    auto regex_source = std::regex(regex);
    std::smatch matches;
    std::string::const_iterator start(str.cbegin());
    while (std::regex_search(start, str.end(), matches, regex_source)) {
        std::vector<std::string> values(matches.begin(), matches.end());
        func(values);
        start = matches.suffix().first;
    }
}

using namespace glm;
using namespace lit::rendering;

std::string CppShader::GetGlslCode() {
    auto name = GetLocation();
    std::ifstream fin(name);

    std::string code((std::istreambuf_iterator<char>(fin)),
                     std::istreambuf_iterator<char>());

    size_t pos_start = code.find(GetName());
    pos_start = code.find('{', pos_start) + 1;

    size_t pos_end = pos_start;
    int balance = 0;

    for (auto index = pos_start; index < code.size(); index++) {
        balance += (code[index] == '{' ? 1 : (code[index] == '}' ? -1 : 0));
        if (balance == -1) {
            pos_end = index;
            break;
        }
    }

    if (pos_start == std::string::npos || pos_end == std::string::npos) {
        return "";
    }

    code = code.substr(pos_start, pos_end - pos_start);

    std::stringstream ss(code);
    std::string line;

    code = "#version 450\n\n"; // todo: parameter

    std::string prev;
    while (std::getline(ss, line, '\n')) {
        if (line.length() > 4) line = line.substr(4);
        if (line.length() > 4) line = line.substr(4);
        if (prev.empty() && line.empty()) continue;
        code += line + "\n";
        prev = line;
    }

    ReplaceAll(code, R"(uniform\s*\(\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*\);)", "uniform $1 $2;");
    ReplaceAll(code, R"(uniform_struct\s*\(\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*\);)", "uniform $1 $2;");
    ReplaceAll(code, R"(\s*:\s*UniformHolder\s*)", " ");
    ReplaceAll(code, R"(uniform_field\s*\(\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*\);)", "$1 $2;");

    ReplaceAll(code, R"(shader_in\s*\(\s*(.*?)\s*,\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*\);)",
               "layout ($1) in $2 $3;");
    ReplaceAll(code, R"(shader_out\s*\(\s*(.*?)\s*,\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*(,\s*.*?)?\);)",
               "layout ($1) out $2 $3;");
    ReplaceAll(code, R"(discard\(\))", "discard");

    ReplaceAll(code, R"((\.[xyzw][xyzw][xyzw][xyzw])\(\))", "$1");
    ReplaceAll(code, R"((\.[xyzw][xyzw][xyzw])\(\))", "$1");
    ReplaceAll(code, R"((\.[xyzw][xyzw])\(\))", "$1");

    // todo: Remove? Writing shader code to file, for debugging
    std::ofstream fout(GetName() + ".glsl");
    fout << code << std::endl;

    return code;
}

std::string CppShader::GetLocation() const {
    return "";
}

std::string CppShader::GetName() const {
    return "";
}

std::vector<lit::gl::Attachment> FragShader::GetOutputTypes() const {
    return m_outputs;
}

template<>
void lit::rendering::SetUniform<int>(GLint location, int val) {
    glUniform1i(location, val);
}

template<>
void lit::rendering::SetUniform<float>(GLint location, float val) {
    glUniform1f(location, val);
}

template<>
void lit::rendering::SetUniform<glm::ivec3>(GLint location, glm::ivec3 val) {
    glUniform3i(location, val.x, val.y, val.z);
}

template<>
void lit::rendering::SetUniform<glm::vec3>(GLint location, glm::vec3 val) {
    glUniform3f(location, val.x, val.y, val.z);
}

template<>
void lit::rendering::SetUniform<glm::vec2>(GLint location, glm::vec2 val) {
    glUniform2f(location, val.x, val.y);
}

template<>
void lit::rendering::SetUniform<glm::mat4>(GLint location, glm::mat4 val) {
    glUniformMatrix4fv(location, 1, GL_FALSE, &val[0][0]);
}
