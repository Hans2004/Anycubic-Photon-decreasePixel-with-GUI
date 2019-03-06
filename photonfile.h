#ifndef PHOTONFILE_H
#define PHOTONFILE_H

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

class PhotonFile
{
public:
    PhotonFile( FILE *fp );
    ~PhotonFile();
    headerType getHeader();
    void decodeImageFromRaw( unsigned long layer, pixelType (&pixelBuf)[1440][2560]);
    void writeFile( FILE *wp );
    void decreasePixel(unsigned long layer);

private:
    headerType header;
    previewType preview1;
    previewType preview2;
    vector<layerDefType> layerDefs;
    vector< vector<unsigned char> > rawData;
    pixelType pixelBuf[1440][2560];
    FILE *fp;

    void readHeader();
    void writeHeader( headerType *header, FILE *fp);
    void readPreview();
    void writePreview( previewType *preview, FILE* fp);
    void readLayerDefs();
    void writeLayerDefs( int nrLayersString, vector<layerDefType> *layerDefs, FILE *fp );
    void readImages();
    void writeImages(vector <vector <unsigned char> > *rawData, int nLayers,FILE *fp);
    void decodeImage( layerDefType *layerDef, FILE *fp, pixelType (&pixelBuf)[1440][2560] );
    void encodeImageToRaw( unsigned long layer, pixelType (&pixelBuf)[1440][2560]);
    void printProgress (double percentage);
};

#endif // PHOTONFILE_H
