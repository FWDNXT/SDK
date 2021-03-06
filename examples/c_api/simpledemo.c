/*
Run simple object classification on Micron DLA
*/
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../../api.h"
//#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image.h"

static void print_help()
{
    printf("Syntax: simpledemo <image file>[-c <categories file>] [-s <microndla.bin file>]\n");
}

#define BYTE2FLOAT 0.003921568f // 1/255

//convert rgb image into float and arrange into column first ordering
void rgb2float_cmajor(float *dst, const unsigned char *src, int width, int height, int cp, int srcstride, const float *mean, const float *std)
{
    int c, i, j;
    float std1[3];
    for(i = 0; i < cp; i++)
        std1[i] = 1 / std[i];
    for(c = 0; c < cp; c++)
        for(i = 0; i < height; i++)
            for(j = 0; j < width; j++)
                dst[c*width*height + i*width + j] = (src[c + cp*j + srcstride*i] * BYTE2FLOAT - mean[c]) * std1[c];
}

float *sortdata;
//sort the vector
int sortcmp(const void *a, const void *b)
{
    float diff = sortdata[*(int *)b] - sortdata[*(int *)a];
    if (diff > 0)
        return 1;
    else if (diff < 0)
        return -1;
    return 0;
}

int main(int argc, char **argv)
{
    const char *image = "./dog224.jpg";//input image
    const char *categ = "./categories.txt";//categories list
    const char *outbin = "save.bin";//file with microndla inference engine instructions
    int i;

    unsigned *noutdims;
    uint64_t **outshapes;
    //program arguments ------------------------
    for(i = 1; i < argc; i++) {
        if(argv[i][0] != '-')
            image = argv[i];
        else switch(argv[i][1])
        {
        case 'c':// categories
            if(i+1 < argc)
                categ = argv[++i];
            break;
        case 's':// output file
            if(i+1 < argc)
                outbin = argv[++i];
            break;
        default:
            print_help();
            return -1;
        }
    }
    if(argc==1)
    {
        print_help();
        return -1;
    }
// initialize FPGA: load hardware and load instructions into memory
    printf("Initializing Micron DLA inference engine\n");
    int noutputs;
    void* sf_handle = ie_init(NULL, outbin, &noutputs, &noutdims, &outshapes, 0);
    unsigned ninputs = 1;
    float *input = NULL;
    uint64_t input_elements = 0;
//fetch input image
    if(image)
    {
        float mean[3] = {0.485, 0.456, 0.406};
        float std[3] = {0.229, 0.224, 0.225};
        int width, height, cp;
        unsigned char *bitmap = (unsigned char *)stbi_load(image, &width, &height, &cp, 0);
        if(!bitmap)
        {
            printf("The image %s could not be loaded\n", image);
            return -1;
        }
        input = (float *)malloc(sizeof(float) * cp * width * height);
        rgb2float_cmajor(input, bitmap, width, height, cp, width * cp, mean, std);
        input_elements = width * height * cp;
        free(bitmap);
    }
    else{
        fprintf(stderr, "Image is NULL\n");
        exit(1);
    }
    uint64_t output_elements;
    float *output;
    uint64_t outsz = 1;
    for(int x = 0; x < noutdims[0]; x++){
        outsz *= outshapes[0][x];
    }
    output_elements = outsz;
    output = (float*) malloc(output_elements*sizeof(float));
    int err = 0;
// run inference
    printf("Running Micron DLA inference engine\n");
    err = ie_run(sf_handle, (const float * const *)&input, &input_elements, ninputs, &output, &output_elements, noutputs);
    if(err==-1)
        return -1;
    if(input)
        free(input);
//read categories list
    FILE *fp = fopen(categ, "r");
    char **categories = (char **)calloc(output_elements, sizeof(char *));
    if(fp)
    {
        char line[300];
        int i = 0;
        while (i < output_elements && fgets(line, sizeof(line), fp))
        {
            char *p = strchr(line, '\n');
            if(p)
                *p = 0;
            categories[i++] = strdup(line);
        }
        fclose(fp);
    }
//print out the results
    printf("-------------- Results --------------\n");
    int* idxs = (int *)malloc(sizeof(int) * output_elements);
    for(i = 0; i < output_elements; i++)
        idxs[i] = i;
    sortdata = output;
    qsort(idxs, output_elements, sizeof(int), sortcmp);
    for(i = 0; i < 5; i++)
        printf("%s (%d) -- %.4f\n", categories[idxs[i]] ? categories[idxs[i]] : "", idxs[i], output[idxs[i]]);
//free allocated memory
    free(idxs);
    for(i = 0; i < output_elements; i++)
        if(categories[i])
            free(categories[i]);
    free(categories);
    ie_free(sf_handle);
    if(output)
        free(output);
    printf("\ndone\n");
    return 0;
}

