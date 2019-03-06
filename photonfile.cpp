#include "photonfile.h"
using namespace std;

PhotonFile::PhotonFile( FILE *file )
{
    fp = file;
    readHeader();
    readPreview();
    readLayerDefs();
    readImages();
}

PhotonFile::~PhotonFile() {
    if (header.preview0Addr!=0) {
        delete[] preview1.imageData;
        delete[] preview2.imageData;
        rawData.clear();
    }
}

headerType PhotonFile::getHeader() {
    return header;
}

void PhotonFile::readHeader() {
    fread(&header, sizeof(headerType),1,fp);
}

void PhotonFile::writeHeader( headerType *header, FILE *fp) {
    fwrite(header, sizeof(headerType),1 , fp);
}

void PhotonFile::readPreview() {
    fseek(fp, header.preview0Addr, SEEK_SET);
    fread(&preview1.resolutionX, sizeof(preview1.resolutionX),1,fp);
    fread(&preview1.resolutionY, sizeof(preview1.resolutionY),1,fp);
    fread(&preview1.imageAddress, sizeof(preview1.imageAddress),1,fp);
    fread(&preview1.dataLength, sizeof(preview1.dataLength),1,fp);
    fread(&preview1.padding, sizeof(preview1.padding),1,fp);
    preview1.imageData = new char[static_cast<unsigned long>(preview1.dataLength)];
    fread(preview1.imageData, static_cast<unsigned long>(preview1.dataLength), 1, fp);

    fseek(fp, header.preview1Addr, SEEK_SET);
    fread(&preview2.resolutionX, sizeof(preview2.resolutionX),1,fp);
    fread(&preview2.resolutionY, sizeof(preview2.resolutionY),1,fp);
    fread(&preview2.imageAddress, sizeof(preview2.imageAddress),1,fp);
    fread(&preview2.dataLength, sizeof(preview2.dataLength),1,fp);
    fread(&preview2.padding, sizeof(preview2.padding),1,fp);
    preview2.imageData = new char[static_cast<unsigned long>(preview2.dataLength)];
    fread(preview2.imageData, static_cast<unsigned long>(preview2.dataLength), 1, fp);
}

void PhotonFile::writePreview( previewType *preview, FILE* fp) {
    fwrite(&preview->resolutionX, sizeof(previewType::resolutionX),1,fp);
    fwrite(&preview->resolutionY, sizeof(previewType::resolutionY),1,fp);
    fwrite(&preview->imageAddress, sizeof(previewType::imageAddress),1,fp);
    fwrite(&preview->dataLength, sizeof(previewType::dataLength),1,fp);
    fwrite(&preview->padding, sizeof(previewType::padding),1,fp);
    fwrite(preview->imageData, static_cast<unsigned long>(preview->dataLength), 1, fp);
}

void PhotonFile::readLayerDefs() {
    layerDefType inLayerDef;

    fseek(fp, header.layerDefsAddr, SEEK_SET);
    for (int i=0; i<header.nrLayers; i++) {
        fread(&inLayerDef, sizeof (inLayerDef),1,fp);
        layerDefs.push_back(inLayerDef);
    }
}

void PhotonFile::writeLayerDefs( int nrLayersString, vector<layerDefType> *layerDefs, FILE *fp ) {
    for (unsigned long i=0; i<static_cast<unsigned long>(nrLayersString); i++) {
        fwrite( &layerDefs->at(i), sizeof(layerDefType), 1, fp);
    }
}

void PhotonFile::readImages() {
    unsigned char eol;
    for (vector<layerDefType>::size_type i=0; i<static_cast<vector<layerDefType>::size_type>(header.nrLayers); i++) {
        fseek(fp, layerDefs.at(i).imageAddress, SEEK_SET);
        vector<unsigned char> temp(static_cast<unsigned long>(layerDefs.at(i).dataLength),0); // This creates a new empty vector with size datalength
        fread(&temp.at(0),static_cast<vector<layerDefType>::size_type>(layerDefs.at(i).dataLength),1,fp);
        rawData.push_back(temp);
        fread(&eol, 1, 1, fp);
    }
}

void PhotonFile::writeImages(vector <vector <unsigned char> > *rawData, int nLayers,FILE *fp) {
    for (unsigned long i=0; i<static_cast<unsigned long>(nLayers); i++) {
        fwrite(&rawData->at(i)[0], rawData->at(i).size(), 1, fp);
    }
}

void PhotonFile::decodeImage( layerDefType *layerDef, FILE *fp, pixelType (&pixelBuf)[1440][2560] ) {
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

void PhotonFile::decodeImageFromRaw( unsigned long layer, pixelType (&pixelBuf)[1440][2560]) {
    char nr=0;
    char val=0;
    int x=0;
    int y=0;
    pixelType color;

    for (auto inByte=rawData[layer].begin(); inByte!=rawData[layer].end(); inByte++) {
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

void PhotonFile::encodeImageToRaw( unsigned long layer, pixelType (&pixelBuf)[1440][2560]) {
    unsigned char runLen=0;
    unsigned char prevColor=0;
    unsigned char data=0;

    vector<unsigned char>().swap(rawData[layer]); // This is guaranteed to empty rd

    prevColor=static_cast<unsigned char>(pixelBuf[0][0].color);
    for (int y=0; y<2560; y++) {
        for (int x=0; x<1440; x++) {
            if ( (pixelBuf[x][y].tempColor!=prevColor) || (runLen==125) ) {
                data=static_cast<unsigned char>((prevColor<<7)|runLen);
                runLen=1;
                rawData[layer].push_back(data);
                prevColor=static_cast<unsigned char>(pixelBuf[x][y].tempColor);
            } else {
                runLen++;
            }
        }
    }
    data=static_cast<unsigned char>((prevColor<<7)|runLen); // Skinners Constant
    rawData[layer].push_back(data);
}

void PhotonFile::decreasePixel( unsigned long layer ) {
    decodeImageFromRaw(layer, pixelBuf);

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
    encodeImageToRaw(layer, pixelBuf);
    layerDefs.at(layer).dataLength=static_cast<int>(rawData[layer].size());
    if (layer<static_cast<unsigned long>(header.nrLayers-1)) {
        layerDefs.at(layer+1).imageAddress = static_cast<int>(static_cast<unsigned long>(layerDefs.at(layer).imageAddress)+rawData[layer].size());
    }
}

void PhotonFile::writeFile( FILE *wp ) {
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

    writeHeader(&header,wp);
    writePreview(&preview1,wp);
    writePreview(&preview2,wp);
    writeLayerDefs(header.nrLayers, &layerDefs, wp);
    writeImages(&rawData, header.nrLayers,wp);
}

void PhotonFile::printProgress (double percentage)
{
    int val = static_cast<int>(percentage * 100);
    int lpad = static_cast<int>(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
}
