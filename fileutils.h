#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <string>

using namespace std;

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

typedef struct {
    char    header[8];
    float   bedX;
    float   bedY;
    float   bedZ;
    int     padding0[3];
    float   layerHeight;
    float   expTime;
    float   expBottom;
    float   offTime;
    int     bottomLayers;
    int     resolutionX;
    int     resolutionY;
    int     preview0Addr;
    int     layerDefsAddr;
    int     nrLayers;
    int     preview1Addr;
    int     unknown6;
    int     projTypeCastMirror;
    int     padding1[6];
} headerType;

typedef struct {
    int     resolutionX;
    int     resolutionY;
    int     imageAddress;
    int     dataLength;
    int     padding[4];
    char*   imageData;
} previewType;

typedef struct {
    float   layerHeight;
    float   expTime;
    float   offTime;
    int     imageAddress;
    int     dataLength;
    int     padding[4];
} layerDefType;

typedef struct {
    char color;
    char tempColor;
} pixelType;

void getHeader(headerType *header, FILE *fp);
void writeHeader( headerType *header, FILE *fp);
void getPreview( int previewAddr, previewType *preview, FILE* fp );
void writePreview( previewType *preview, FILE* fp);
void getLayerDefs( int layerDefsAddr, int nrLayersString, vector<layerDefType> *layerDefs, FILE *fp );
void writeLayerDefs( int nrLayersString, vector<layerDefType> *layerDefs, FILE *fp );
void getImages( vector <vector <unsigned char> > *rawData, vector<layerDefType> *layerDefs, int nLayers, FILE *fp);
void writeImages(vector <vector <unsigned char> > *rawData, int nLayers,FILE *fp);
void decodeImage( layerDefType *layerDef, FILE *fp, pixelType (&pixelBuf)[1440][2560] );
void decodeImageFromRaw( vector<unsigned char> rd, pixelType (&pixelBuf)[1440][2560]);
void encodeImageToRaw( vector<unsigned char> *rd, pixelType (&pixelBuf)[1440][2560]);
void decreasePixel( pixelType (&pixelBuf)[1440][2560]);
void printProgress (double percentage);

#endif // FILEUTILS_H
