#include "VoxelShader.hpp"

#include "Library.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool VoxelShader::init() {
    if (!Shader::init()) {
        std::cerr << "Error initializing diffuse shader" << std::endl;
        return false;
    }

    addAttribute("vertPos"); 

    addUniform("P");
    addUniform("V");
    addUniform("M");

    addUniform("isOutline");

    addUniform("voxelData");
}

/* Visualize voxels */
void VoxelShader::render(std::vector<Volume::Voxel> & voxels, glm::mat4 P, glm::mat4 V) {
    if (!enabled) {
        return;
    }

    bind();

    /* Bind projeciton, view, inverise view matrices */
    loadMat4(getUniform("P"), &P);
    loadMat4(getUniform("V"), &V);

    /* Bind mesh */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::cube->vaoId));

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    if (pos >= 0 && Library::cube->vertBufId) {
        CHECK_GL_CALL(glEnableVertexAttribArray(pos));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::cube->vertBufId));
        CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
    }

    /* IBO */
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Library::cube->eleBufId));

    glm::mat4 M;
    for (auto v : voxels) {
        /* Only render voxels that contain data */
        if (v.data.r || v.data.g || v.data.b || v.data.a) {
            M = glm::mat4(1.f);
            M *= glm::translate(glm::mat4(1.f), v.spatial.position);
            M *= glm::scale(glm::mat4(1.f), v.spatial.scale);
            loadMat4(getUniform("M"), &M);

            loadVec4(getUniform("voxelData"), v.data);

            /* Draw shape */
            loadBool(getUniform("isOutline"), false);
            CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)Library::cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr));

            /* Draw outline */
            if (drawOutline) {
                loadBool(getUniform("isOutline"), true);
                CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)Library::cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr));
                CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }
        }
   }

    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindVertexArray(0));
    unbind();
}