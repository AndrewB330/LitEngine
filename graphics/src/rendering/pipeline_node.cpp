#include <lit/rendering/pipeline_node.hpp>

using namespace lit::rendering;

float verticesRect[] = {
        1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
};

unsigned int indicesRect[] = {0, 1, 2,
                              0, 2, 3};

unsigned int VAO_Rect, VBO_Rect, EBO_Rect;

void lit::rendering::DrawQuad() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        glGenVertexArrays(1, &VAO_Rect);
        glGenBuffers(1, &VBO_Rect);
        glGenBuffers(1, &EBO_Rect);

        glBindVertexArray(VAO_Rect);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_Rect);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRect), verticesRect, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Rect);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesRect), indicesRect, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(VAO_Rect);
    glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);
}

float verticesCube[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
};

unsigned int indicesCube[] = {
        0, 1, 3,
        0, 3, 2,

        6, 7, 5,
        6, 5, 4,

        0, 5, 1, // bottom
        0, 4, 5, // bottom

        6, 3, 7, // top
        6, 2, 3, // top

        0, 2, 6,
        0, 6, 4,

        5, 7, 3,
        5, 3, 1,
};

unsigned int VAO_Cube, VBO_Cube, EBO_Cube;

void lit::rendering::DrawCube() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        glGenVertexArrays(1, &VAO_Cube);
        glGenBuffers(1, &VBO_Cube);
        glGenBuffers(1, &EBO_Cube);

        glBindVertexArray(VAO_Cube);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_Cube);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Cube);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCube), indicesCube, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(VAO_Cube);
    glDrawElements(GL_TRIANGLES, 3 * 12, GL_UNSIGNED_INT, nullptr);
}



float verticesCube_n[] = {
        // left
        0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        -1.0f, 0.0f, 0.0f,
        // right
        1.0f, 0.0f, 0.0f,
        +1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        +1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        +1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
        +1.0f, 0.0f, 0.0f,
        // bottom
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, -1.0f, 0.0f,
        // top
        0.0f, 1.0f, 0.0f,
        0.0f, +1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, +1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, +1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, +1.0f, 0.0f,
        // back
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
        // front
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, +1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, +1.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, +1.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, +1.0f,
};

unsigned int indicesCube_n[] = {
        // left
        0, 1, 3,
        0, 3, 2,
        // right
        7, 5, 4,
        6, 7, 4,
        // top
        11, 9, 8,
        10, 11, 8,
        // top
        12, 13, 15,
        12, 15, 14,
        // back
        16, 17, 19,
        16, 19, 18,
        // front
        23, 21, 20,
        22, 23, 20,
};

unsigned int VAO_Cube_n, VBO_Cube_n, EBO_Cube_n;

void lit::rendering::DrawCubeWithNormals() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        glGenVertexArrays(1, &VAO_Cube_n);
        glGenBuffers(1, &VBO_Cube_n);
        glGenBuffers(1, &EBO_Cube_n);

        glBindVertexArray(VAO_Cube_n);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_Cube_n);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube_n), verticesCube_n, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Cube_n);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCube_n), indicesCube_n, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(VAO_Cube_n);
    glDrawElements(GL_TRIANGLES, sizeof(indicesCube_n) / sizeof(int), GL_UNSIGNED_INT, nullptr);
}
