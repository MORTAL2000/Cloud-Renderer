#include "VoxelizeShader.hpp"

#include "IO/Window.hpp"
#include "Library.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool VoxelizeShader::init(Volume *vol, int width, int height) {
    if (!Shader::init()) {
        std::cerr << "Error initializing volume shader" << std::endl;
        return false;
    }

    addAttribute("vertPos");
    
    addUniform("P");
    addUniform("V");
    addUniform("M");
    addUniform("Vi");

    addUniform("camPos");

    addUniform("center");
    addUniform("scale");

    addUniform("volume");
    addUniform("xBounds");
    addUniform("yBounds");
    addUniform("zBounds");
    addUniform("dimension");
    addUniform("voxelize");

    addUniform("normalStep");
    addUniform("visibilityContrib");

    addUniform("positionMap");
    addUniform("mapWidth");
    addUniform("mapHeight");

    this->volume = vol;
    positionMap = new Texture();
    positionMap->width = width;
    positionMap->height = height;
    initFBO();

    return true;
}

void VoxelizeShader::voxelize(glm::mat4 P, glm::mat4 V, glm::vec3 camPos) {
    /* Reset volume and position map */
    volume->clear();
    clearPositionMap();

    /* Disable quad visualization */
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
    
    /* Populate volume */
    bind();
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + volume->volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volume->volId));
    CHECK_GL_CALL(glBindImageTexture(0, volume->volId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));

    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + positionMap->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, positionMap->textureId));
    CHECK_GL_CALL(glBindImageTexture(1, positionMap->textureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F));
    loadInt(getUniform("mapWidth"), positionMap->width);
    loadInt(getUniform("mapHeight"), positionMap->height);

    loadFloat(getUniform("normalStep"), normalStep);
    loadFloat(getUniform("visibilityContrib"), visibilityContrib);
    renderMesh(P, V, camPos, true);
    CHECK_GL_CALL(glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
    CHECK_GL_CALL(glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F));
    unbind();

    /* Reset state */
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    /* Update CPU representation of volume */
    volume->updateVoxelData();
}

void VoxelizeShader::renderMesh(glm::mat4 P, glm::mat4 V, glm::vec3 camPos, bool vox) {
    loadMat4(getUniform("P"), &P);
    loadMat4(getUniform("V"), &V);
    glm::mat4 Vi = V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMat4(getUniform("Vi"), &Vi);
 
    loadVec3(getUniform("camPos"), camPos);
    loadInt(getUniform("dimension"), volume->dimension);
    loadVec2(getUniform("xBounds"), volume->xBounds);
    loadVec2(getUniform("yBounds"), volume->yBounds);
    loadVec2(getUniform("zBounds"), volume->zBounds);
    loadVec3(getUniform("center"), volume->quadPosition);
    loadFloat(getUniform("scale"), volume->quadScale.x);
    /* Boolean denotes whether to voxelize or not */
    loadBool(getUniform("voxelize"), vox);

    /* Bind quad */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Vertices VBO */
    // TODO : unnecessary?
    int pos = getAttribute("vertPos");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::quad->vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

    glm::mat4 M = glm::mat4(1.f);
    M *= glm::translate(glm::mat4(1.f), volume->quadPosition);
    M *= glm::scale(glm::mat4(1.f), glm::vec3(volume->quadScale.x));
    loadMat4(getUniform("M"), &M);
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
}

void VoxelizeShader::initFBO() {
    ///* Generate the FBO for the shadow depth */
    //CHECK_GL_CALL(glGenFramebuffers(1, &positionMapFBOId));

    /* Generate the texture */
    CHECK_GL_CALL(glGenTextures(1, &positionMap->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, positionMap->textureId));
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, positionMap->width, positionMap->height, 0, GL_RGBA, GL_FLOAT, NULL));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    ///* Bind with framebuffer's depth buffer */
    //CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, positionMapFBOId));
    //CHECK_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionMap->textureId, 0));
    //CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void VoxelizeShader::clearPositionMap() {
    CHECK_GL_CALL(glClearTexImage(positionMap->textureId, 0, GL_RGBA, GL_FLOAT, nullptr));
}