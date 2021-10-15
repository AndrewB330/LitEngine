#include <lit/gl/context.hpp>
#include <lit/gl/vertex_buffer.hpp>
#include <glm/glm.hpp>
#include <iostream>

int main() {
    lit::gl::VertexData<glm::vec4, glm::vec3> a;
    LIT_ASSERT((lit::gl::__CheckVertexDataSize<glm::vec4, glm::vec3>()), "ok");
    LIT_ASSERT((lit::gl::__CheckVertexDataSize<glm::vec4, glm::ivec3>()), "ok");
    LIT_ASSERT((lit::gl::__CheckVertexDataSize<glm::vec4, glm::ivec2>()), "ok");
    LIT_ASSERT((lit::gl::__CheckVertexDataSize<glm::vec4, glm::ivec2, glm::dvec2>()), "ok");
    LIT_ASSERT((lit::gl::__CheckVertexDataSize<glm::dvec4, glm::ivec2, glm::dvec2>()), "ok");

    std::cout << sizeof(a) << std::endl;
    return 0;
}