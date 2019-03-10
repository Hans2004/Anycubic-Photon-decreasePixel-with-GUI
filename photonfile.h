#ifndef PHOTONFILE_H
#define PHOTONFILE_H

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <string>
#include <memory>

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
    //char*   imageData;
    vector<unsigned char> imageData; // Much safer than using a char*
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
    PhotonFile( string fileName);
    ~PhotonFile();
    headerType getHeader();
    void decodeImageFromRaw( unsigned long layer, pixelType (&pixelBuf)[1440][2560]);
    void writeFile(string fileName);
    void decreasePixel(unsigned long layer);
    friend ostream& operator<<(ostream &os, const PhotonFile &ff);

private:
    headerType header;
    previewType preview1;
    previewType preview2;
    vector<layerDefType> layerDefs;
    vector< vector<unsigned char> > rawData;
    pixelType pixelBuf[1440][2560];
    FILE *fp=nullptr;
    FILE *wp=nullptr;

    void readHeader();
    void writeHeader();
    void readPreview();
    void writePreviews();
    void readLayerDefs();
    void writeLayerDefs();
    void readImages();
    void writeImages();
    void decodeImage( layerDefType *layerDef, FILE *fp, pixelType (&pixelBuf)[1440][2560] );
    void encodeImageToRaw( unsigned long layer, pixelType (&pixelBuf)[1440][2560]);
    static void printProgress (double percentage);
};

#endif // PHOTONFILE_H
