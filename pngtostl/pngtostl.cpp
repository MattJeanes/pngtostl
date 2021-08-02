/* Copyright (c) 2021, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PNG_DEBUG 3
#include <png.h>
#include <math.h>
#include <string>
#include <vector>
#include <fstream>

struct vec {
    float m_x;
    float m_y;
    float m_z;
};

struct tri {
    vec m_n;
    vec m_p1;
    vec m_p2;
    vec m_p3;
};

 /* Global parameters set via command line options. */
int OptNegative = 1;
int OptLevels = 20;
float OptReliefHeight = 1;
float OptBaseHeight = .2;

/* Emits a triangle in the format of STL files. We use a normal set to
 * zero, so it is up to the slicer to calculate the real normal: for this
 * reason the function should be called with the three vertexes in
 * counter clockwise order from the POV of looking at the face from outside
 * the solid. */
tri emitTriangle(float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3)
{
    tri tri;
    tri.m_n.m_x = 0;
    tri.m_n.m_y = 0;
    tri.m_n.m_z = 0;

    tri.m_p1.m_x = x1;
    tri.m_p1.m_y = y1;
    tri.m_p1.m_z = z1;

    tri.m_p2.m_x = x2;
    tri.m_p2.m_y = y2;
    tri.m_p2.m_z = z2;

    tri.m_p3.m_x = x3;
    tri.m_p3.m_y = y3;
    tri.m_p3.m_z = z3;

    return tri;
}

/* Transforms the box originating at x,y and with x-y dimension xsize,ysize
 * and height (from the Z plane) zheight, into the corresponding set
 * of triangles.
 *
 * The x,y pair identifies the bottom-left corner of the box. */
void boxToTriangles(std::vector<tri> &triangles, float x, float y, float xsize, float ysize, float zheight) {
    /* Bottom side. */
    triangles.push_back(emitTriangle(x, y, 0, x, y + ysize, 0, x + xsize, y, 0));
    triangles.push_back(emitTriangle(x + xsize, y, 0, x, y + ysize, 0, x + xsize, y + ysize, 0));
    /* Top side. */
    triangles.push_back(emitTriangle(x, y, zheight, x + xsize, y, zheight, x, y + ysize, zheight));
    triangles.push_back(emitTriangle(x + xsize, y, zheight, x + xsize, y + ysize, zheight, x, y + ysize, zheight));
    /* Left side. */
    triangles.push_back(emitTriangle(x, y, 0, x, y, zheight, x, y + ysize, 0));
    triangles.push_back(emitTriangle(x, y + ysize, 0, x, y, zheight, x, y + ysize, zheight));
    /* Right side. */
    float rx = x + xsize;
    triangles.push_back(emitTriangle(rx, y, 0, rx, y + ysize, 0, rx, y, zheight));
    triangles.push_back(emitTriangle(rx, y + ysize, 0, rx, y + ysize, zheight, rx, y, zheight));
    /* Front side. */
    triangles.push_back(emitTriangle(x, y, 0, x + xsize, y, zheight, x, y, zheight));
    triangles.push_back(emitTriangle(x, y, 0, x + xsize, y, 0, x + xsize, y, zheight));
    /* Back side. */
    float by = y + ysize;
    triangles.push_back(emitTriangle(x, by, 0, x, by, zheight, x + xsize, by, zheight));
    triangles.push_back(emitTriangle(x, by, 0, x + xsize, by, zheight, x + xsize, by, 0));
}

void write_stl(const char* filename, std::vector<tri> triangles) {

    //binary file
    std::string header_info = "PngToStl";
    char head[80];
    std::memcpy(head, header_info.c_str(), sizeof(head) - 1);
    char attribute[2] = "0";
    unsigned long nTriLong = triangles.size();

    std::ofstream myfile;

    myfile.open(filename, std::ios::out | std::ios::binary);
    myfile.write(head, sizeof(head));
    myfile.write((char*)&nTriLong, 4);

    //write down every triangle
    for (std::vector<tri>::iterator it = triangles.begin(); it != triangles.end(); it++) {
        //normal vector coordinates

        myfile.write((char*)&it->m_n.m_x, 4);
        myfile.write((char*)&it->m_n.m_y, 4);
        myfile.write((char*)&it->m_n.m_z, 4);

        //p1 coordinates
        myfile.write((char*)&it->m_p1.m_x, 4);
        myfile.write((char*)&it->m_p1.m_y, 4);
        myfile.write((char*)&it->m_p1.m_z, 4);

        //p2 coordinates
        myfile.write((char*)&it->m_p2.m_x, 4);
        myfile.write((char*)&it->m_p2.m_y, 4);
        myfile.write((char*)&it->m_p2.m_z, 4);

        //p3 coordinates
        myfile.write((char*)&it->m_p3.m_x, 4);
        myfile.write((char*)&it->m_p3.m_y, 4);
        myfile.write((char*)&it->m_p3.m_z, 4);

        myfile.write(attribute, 2);
    }

    myfile.close();

    printf("wrote file to %s", filename);
}


/* Load a PNG and returns it as a raw RGB representation, as an array of bytes.
 * As a side effect the function populates widthptr, heigthptr with the
 * size of the image in pixel. The integer pointed by alphaptr is set to one.
 * if the image is of type RGB_ALPHA, otherwise it's set to zero.
 *
 * This function is able to load both RGB and RGBA images, but it will always
 * return data as RGB, discarding the alpha channel. */
#define PNG_BYTES_TO_CHECK 8
unsigned char* PngLoad(FILE* fp, int* widthptr, int* heightptr, int* alphaptr) {
    unsigned char buf[PNG_BYTES_TO_CHECK];
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height, j;
    int color_type, row_bytes;
    unsigned char** imageData, * rgb;

    /* Check signature */
    size_t tmp = fread(buf, 1, PNG_BYTES_TO_CHECK, fp);
    if (tmp != PNG_BYTES_TO_CHECK)
        return NULL;
    if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK))
        return NULL; /* Not a PNG image */

    /* Initialize data structures */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
        NULL, NULL, NULL);
    if (png_ptr == NULL) {
        return NULL; /* Out of memory */
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return NULL;
    }

    /* Error handling code */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    /* Set the I/O method */
    png_init_io(png_ptr, fp);

    /* Undo the fact that we read some data to detect the PNG file */
    png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

    /* Read the PNG in memory at once */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* Get image info */
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    if (color_type != PNG_COLOR_TYPE_RGB &&
        color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    /* Get the image data */
    imageData = png_get_rows(png_ptr, info_ptr);
    row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    rgb = (unsigned char*)malloc(row_bytes * height);
    if (!rgb) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    for (j = 0; j < height; j++) {
        unsigned char* dst = rgb + (j * width * 3);
        unsigned char* src = imageData[j];
        unsigned int i;

        for (i = 0; i < width; i++) {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst += 3;
            src += (color_type == PNG_COLOR_TYPE_RGB_ALPHA) ? 4 : 3;
        }
    }

    /* Free the image and resources and return */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    *widthptr = width;
    *heightptr = height;
    if (alphaptr) *alphaptr = (color_type == PNG_COLOR_TYPE_RGB_ALPHA);
    return rgb;
}

/* Convert the specified PNG file into an STL where thickness of any point is
 * proportional to the image level of gray. */
void pngtostl(const char* filename, const char* outputFilename) {
    FILE* fp;
    errno_t err;
    if((err = fopen_s(&fp, filename, "rb")) != 0) {
        perror("Opening the PNG file");
        exit(1);
    }

    /* Load the PNG file in memory as RGB raw data. */
    int w, h;
    unsigned char* rgb = PngLoad(fp, &w, &h, NULL);
    if (rgb == NULL) {
        fprintf(stderr, "Error parsing the PNG file.\n");
        exit(1);
    }
    fclose(fp);
    float max = 0;

    /* Calculate the max white level in the image. */
    unsigned char* p = rgb;
    for (int j = 0; j < w * h; j++) {
        float lum = (float)(p[0] + p[1] + p[2]) / 3;
        if (lum > max) max = lum;
        p += 3;
    }

    std::vector<tri> triangles;

    /* Emit the STL file. */
    p = rgb;
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            float lum = (float)(p[0] + p[1] + p[2]) / 3;
            int level = round((OptLevels - 1) * lum / max);
            if (OptNegative) level = OptLevels - level - 1;
            float height = OptBaseHeight + (OptReliefHeight * level / OptLevels);
            boxToTriangles(triangles, x, y, 1, 1, height);
            p += 3;
        }
    }

    write_stl(outputFilename, triangles);
}

/* Show help on wrong call or if --help is given. */
void showHelp(void) {
    fprintf(stderr, "png2stl image.png output.stl [... options ...]\n"
        "  --relief-height <mm> | Relief height.      Default: 1 mm\n"
        "  --base-height <mm>   | Fixed base height.  Default: .2 mm\n"
        "  --levels             | Number of different levels (heights/greys). Default: 20\n"
        "  --negative           | Use thicker plastic for black (default).\n"
        "  --positive           | Use thicker plastic for white.\n");
}

int main(int argc, char** argv) {
    const char* filename = NULL;
    const char* outputFilename = NULL;

    /* Check arity and parse additional args if any. */
    if (argc < 3) {
        showHelp();
        exit(0);
    }

    int j;
    for (j = 1; j < argc; j++) {
        int moreargs = j + 1 < argc;

        if (!strcmp(argv[j], "--relief-height") && moreargs) {
            OptReliefHeight = atof(argv[++j]);
        }
        else if (!strcmp(argv[j], "--base-height") && moreargs) {
            OptBaseHeight = atof(argv[++j]);
        }
        else if (!strcmp(argv[j], "--levels") && moreargs) {
            OptLevels = atoi(argv[++j]);
            if (OptLevels < 2) OptLevels = 2;
        }
        else if (!strcmp(argv[j], "--positive")) {
            OptNegative = 0;
        }
        else if (!strcmp(argv[j], "--negative")) {
            OptNegative = 1;
        }
        else if (!strcmp(argv[j], "--help")) {
            showHelp();
            exit(0);
        }
        else if (argv[j][0] != '-' && filename == NULL) {
            filename = argv[j];
        }
        else if (argv[j][0] != '-' && outputFilename == NULL) {
            outputFilename = argv[j];
        }
        else {
            fprintf(stderr, "Invalid options.");
            showHelp();
            exit(1);
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "No PNG filename given\n");
        exit(1);
    }

    if (outputFilename == NULL) {
        fprintf(stderr, "No output filename given\n");
        exit(1);
    }

    pngtostl(filename, outputFilename);
    return 0;
}
