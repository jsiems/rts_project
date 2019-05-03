
#include "texman.h"

// ********** private functions **********

int loadTexture(char *name) {
    // create name path
    // e.g. textures/name.png
    int name_len = strlen(name);
    char *texname = malloc(name_len + 20);
    texname[name_len] = '\0';
    strcpy(texname, "textures/\0");
    strcat(texname, name);
    strcat(texname, ".png\0");

    unsigned char *image_data;
    int image_width, image_height, nr_channels;
    unsigned int texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    image_data = stbi_load(texname, &image_width, &image_height, &nr_channels, STBI_rgb_alpha);
    if(image_data == NULL) {
        printf("Failed to load texture %s\n", texname);
        return -1;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image_data);
    
    free(texname);

    return texture;
}

int appendTexture(struct TexMan *texman, char *name) {
    // allocate memory for the next texture
    struct Texture *newtex = malloc(sizeof(*newtex));
    if(newtex == 0) {
        printf("error allocating memory for new texture\n");
        exit(1);
    }

    // init texture struct variables
    newtex->name = malloc(strlen(name) + 1);
    strcpy(newtex->name, name);
    newtex->id = loadTexture(name);

    if(newtex->id == -1) {
        free(newtex->name);
        free(newtex);
        return -1;
    }

    // add the texture to the list
    if(texman->head == 0) {
        texman->head = newtex;
        texman->tail = newtex;
    }
    else {
        texman->tail->next = newtex;
        texman->tail = newtex;
    }
    newtex->next = 0;

    return 0;
}

// ********** public functions **********

int getTextureId(struct TexMan *texman, char *name) {
    uint8_t found = 0;
    struct Texture *current = texman->head;
    // check if the texture has already been loaded
    while(current != 0) {
        if(strcmp(name, current->name) == 0 ) {
            found = 1;
            break;
        }
        current = current->next;
    }

    // load texture if it has not been loaded
    if(!found) {
        // Attempt to load texture. If it cannot be loaded, return -1
        if(appendTexture(texman, name) == -1) {
            return -1;
        }
        current = texman->tail;
    }

    return current->id;
}

void initTexMan(struct TexMan *texman) {
    texman->head = 0;
    texman->tail = 0;
}

void destroyTexMan(struct TexMan *texman) {
    struct Texture *current = texman->head;
    while(current != 0) {
        free(current->name);
        struct Texture *temp = current;
        current = current->next;
        free(temp);
    }
}

