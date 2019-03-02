#include "fileutils.h"
#include <QtCore>
#include <QDebug>

using namespace std;

void getHeader(headerType *header, FILE *fp) {
    fread(header, sizeof(headerType),1,fp);
}

void writeHeader( headerType *header, FILE *fp) {
    fwrite(header, sizeof(headerType),1 , fp);
}

void getPreview( int previewAddr, previewType *preview, FILE* fp ) {
    fseek(fp, previewAddr, SEEK_SET);
    fread(&preview->resolutionX, sizeof(preview->resolutionX),1,fp);
    fread(&preview->resolutionY, sizeof(preview->resolutionY),1,fp);
    fread(&preview->imageAddress, sizeof(preview->imageAddress),1,fp);
    fread(&preview->dataLength, sizeof(preview->dataLength),1,fp);
    fread(&preview->padding, sizeof(preview->padding),1,fp);
    preview->imageData = static_cast<char*>(malloc(static_cast<unsigned long>(preview->dataLength)));
    fread(preview->imageData, static_cast<unsigned long>(preview->dataLength), 1, fp);
}

void writePreview( previewType *preview, FILE* fp) {
    fwrite(&preview->resolutionX, sizeof(previewType::resolutionX),1,fp);
    fwrite(&preview->resolutionY, sizeof(previewType::resolutionY),1,fp);
    fwrite(&preview->imageAddress, sizeof(previewType::imageAddress),1,fp);
    fwrite(&preview->dataLength, sizeof(previewType::dataLength),1,fp);
    fwrite(&preview->padding, sizeof(previewType::padding),1,fp);
    fwrite(preview->imageData, static_cast<unsigned long>(preview->dataLength), 1, fp);
}

void getLayerDefs( int layerDefsAddr, int nrLayersString, vector<layerDefType> *layerDefs, FILE *fp ) {
    layerDefType inLayerDef;

    fseek(fp, layerDefsAddr, SEEK_SET);
    for (int i=0; i<nrLayersString; i++) {
        fread(&inLayerDef, sizeof (inLayerDef),1,fp);
        layerDefs->push_back(inLayerDef);
    }
}

void writeLayerDefs( int nrLayersString, vector<layerDefType> *layerDefs, FILE *fp ) {
    for (unsigned long i=0; i<static_cast<unsigned long>(nrLayersString); i++) {
        fwrite( &layerDefs->at(i), sizeof(layerDefType), 1, fp);
    }
}

void getImages( vector <vector <unsigned char> > *rawData, vector<layerDefType> *layerDefs, int nLayers, FILE *fp) {
    unsigned char eol;
    for (vector<layerDefType>::size_type i=0; i<static_cast<vector<layerDefType>::size_type>(nLayers); i++) {
        fseek(fp, layerDefs->at(i).imageAddress, SEEK_SET);
        vector<unsigned char> temp(static_cast<unsigned long>(layerDefs->at(i).dataLength),0); // This creates a new empty vector with size datalength
        fread(&temp.at(0),static_cast<vector<layerDefType>::size_type>(layerDefs->at(i).dataLength),1,fp);
        rawData->push_back(temp);
        fread(&eol, 1, 1, fp);
    }
}

void writeImages(vector <vector <unsigned char> > *rawData, int nLayers,FILE *fp) {
    for (unsigned long i=0; i<static_cast<unsigned long>(nLayers); i++) {
        fwrite(&rawData->at(i)[0], rawData->at(i).size(), 1, fp);
    }
}

void decodeImage( layerDefType *layerDef, FILE *fp, pixelType (&pixelBuf)[1440][2560] ) {
    unsigned char inByte;
    char nr=0;
    char val=0;
    int x=0;
    int y=0;
    int i;
    pixelType color;

    fseek(fp, layerDef->imageAddress, SEEK_SET);

    for (i=0; i<layerDef->dataLength; i++) {
        fread(&inByte, sizeof(inByte), 1, fp);
        nr = static_cast<char>(inByte&127);
        val = static_cast<char>(inByte>>7);
        color.color=val;
        color.tempColor=val;

        while (nr!=0) {
            pixelBuf[x][y]=color;
            x++;
            if (x==1440) {
                x=0; y++;
            }
            nr--;
        }
    }
}

void decodeImageFromRaw( vector<unsigned char> rd, pixelType (&pixelBuf)[1440][2560]) {
    char nr=0;
    char val=0;
    int x=0;
    int y=0;
    pixelType color;

    for (auto inByte=rd.begin(); inByte!=rd.end(); inByte++) {
        nr = static_cast<char>(*inByte&127);
        val = static_cast<char>(*inByte>>7);
        color.color=val;
        color.tempColor=val;
        while (nr!=0) {
            pixelBuf[x][y]=color;
            //if (color.color!=0) qDebug() << "Not Zero:" << x << " " << y;
            x++;
            if (x==1440) {
                x=0; y++;
            }
            nr--;
        }
    }
}

void encodeImageToRaw( vector<unsigned char> *rd, pixelType (&pixelBuf)[1440][2560]) {
    unsigned char runLen=0;
    unsigned char prevColor=0;
    unsigned char data=0;

    vector<unsigned char>().swap(*rd); // This is guaranteed to empty rd

    prevColor=static_cast<unsigned char>(pixelBuf[0][0].color);
    for (int y=0; y<2560; y++) {
        for (int x=0; x<1440; x++) {
            if ( (pixelBuf[x][y].tempColor!=prevColor) || (runLen==125) ) {
                data=static_cast<unsigned char>((prevColor<<7)|runLen);
                runLen=1;
                rd->push_back(data);
                prevColor=static_cast<unsigned char>(pixelBuf[x][y].tempColor);
            } else {
                runLen++;
            }
        }
    }
    data=static_cast<unsigned char>((prevColor<<7)|runLen); // Skinners Constant
    rd->push_back(data);
}

void decreasePixel( pixelType (&pixelBuf)[1440][2560]) {
    for (int y=1; y<2560-1; y++) {
        for (int x=1; x<1440-1; x++) {
            int clr=pixelBuf[x][y].color +
                    pixelBuf[x-1][y].color +
                    pixelBuf[x-1][y-1].color +
                    pixelBuf[x][y-1].color +
                    pixelBuf[x+1][y-1].color +
                    pixelBuf[x+1][y].color +
                    pixelBuf[x+1][y+1].color +
                    pixelBuf[x][y+1].color +
                    pixelBuf[x-1][y+1].color;
            if (clr<9) pixelBuf[x][y].tempColor=0; else pixelBuf[x][y].tempColor=1;
        }
    }
}

void printProgress (double percentage)
{
    int val = static_cast<int>(percentage * 100);
    int lpad = static_cast<int>(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
}

/*
// [[gnu::unused]]
int main(int argc, char* argv[]) {
    headerType header;
    previewType preview1;
    previewType preview2;
    vector<layerDefType> layerDefs;
    vector< vector<unsigned char> > rawData;
    static pixelType pixelBuf[1440][2560];

    if (argc<3) {
        cout << endl;
        cout << "Usage: decreasePixel file1.photon file2.photon" << endl;
        cout << endl;
        return(EXIT_FAILURE);
    }

    string inFile=argv[1];
    string outFile=argv[2];
    FILE *fp=fopen(inFile.c_str(), "rb");
    if (fp==nullptr) {
        cout << endl << "Error! File: " << inFile << " does not exist!" << endl;
        return(EXIT_FAILURE);
    }

    getHeader(&header, fp);
    getPreview( header.preview0Addr, &preview1, fp );
    getPreview( header.preview1Addr, &preview2, fp );
    getLayerDefs( header.layerDefsAddr, header.nrLayers, &layerDefs, fp );
    getImages(&rawData, &layerDefs, header.nrLayers,fp);

    cout << "Parameters for file     : " << inFile << endl;
    cout << "Bed Dimensions (X, Y, Z): " << header.bedX << ", " << header.bedY << ", " << header.bedZ << endl;
    cout << "Layer Height            : " << header.layerHeight << endl;
    cout << "Exposure Time           : " << header.expTime << endl;
    cout << "Bottom Exposure Time    : " << header.expBottom << endl;
    cout << "Off Time                : " << header.offTime << endl;
    cout << "#Bottom Layers          : " << header.bottomLayers << endl;
    cout << "Total #Layers           : " << header.nrLayers << endl;
    cout << endl;

    int nextAddr=layerDefs[0].imageAddress;
    for (unsigned long i=0; i<static_cast<unsigned long>(header.nrLayers); i++) {
        //cout << "Converting layer: " << i << endl;
        printProgress(static_cast<double>(i*1.0/header.nrLayers));
        layerDefs[i].imageAddress=nextAddr;
        decodeImageFromRaw(rawData[i], pixelBuf);
        decreasePixel(pixelBuf);
        encodeImageToRaw(&rawData[i], pixelBuf);
        layerDefs[i].dataLength=static_cast<int>(rawData[i].size());
        nextAddr+=rawData[i].size();
    }

    cout << endl;


    int nextFp = sizeof(headerType);
    header.preview0Addr = nextFp;
    nextFp += sizeof(previewType::padding) +
            sizeof(previewType::dataLength) +
            sizeof(previewType::resolutionX) +
            sizeof(previewType::resolutionY) +
            sizeof(previewType::imageAddress);
    preview1.imageAddress = nextFp;
    nextFp += static_cast<unsigned long>(preview1.dataLength);
    header.preview1Addr = nextFp;
    nextFp += sizeof(previewType::padding) +
            sizeof(previewType::dataLength) +
            sizeof(previewType::resolutionX) +
            sizeof(previewType::resolutionY) +
            sizeof(previewType::imageAddress);
    preview2.imageAddress = nextFp;
    nextFp += static_cast<unsigned long>(preview2.dataLength);
    header.layerDefsAddr = nextFp;
    nextFp += static_cast<int>(sizeof(layerDefType)) * header.nrLayers;
    for (unsigned long i=0; i<static_cast<unsigned long>(header.nrLayers); i++) {
        layerDefs.at(i).imageAddress=nextFp;
        nextFp+=rawData.at(i).size();
    }

    FILE *wp=fopen(outFile.c_str(),"wb");
    writeHeader(&header,wp);
    writePreview(&preview1,wp);
    writePreview(&preview2,wp);
    writeLayerDefs(header.nrLayers, &layerDefs, wp);
    writeImages(&rawData, header.nrLayers,wp);

    free(preview1.imageData);
    free(preview2.imageData);
    fclose(fp);
    fclose(wp);
    return(EXIT_SUCCESS);
}
*/
