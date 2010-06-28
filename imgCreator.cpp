#include "imgCreator.hpp"
#include "xmlUtils.hpp"

#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <windows.h>

#include <gd.h>
#include <gdfontt.h>

using namespace std;


inline unsigned int rr(
                       unsigned int start,
                       unsigned int end
                       )
{
    if(start == end)
        return end;

    return (rand() % (end - start)) + start;
}


void Creator::createLastPlayedChart(
                                    const std::vector<const DOMElement *> & arr, 
                                    const std::basic_string<TCHAR> & file, 
                                    const std::string & font,
                                    int fsize
                                    ) 
                                    const
{
    cerr << __FUNCTION__;

    int bounds[8];

    std::vector<char> v(font.begin(), font.end());
    v.push_back('\0');

    std::stringstream fulltext;

    for(std::vector<const DOMElement *>::const_iterator i = arr.begin();
        i != arr.end(); ++i)
    {
        SYSTEMTIME t;
        std::wstringstream iss, oss;

        iss << (*i)->getElementsByTagName(XS("time"))->item(0)->getTextContent();

        iss >> t.wYear >> t.wMonth >> t.wDay >> t.wDayOfWeek >>
            t.wHour >> t.wMinute >> t.wSecond >> t.wMilliseconds;


        oss << (*i)->getElementsByTagName(XS("artist"))->item(0)->getTextContent() << " - "
            << (*i)->getElementsByTagName(XS("track"))->item(0)->getTextContent();

        std::wstring temp = oss.str();
        std::vector<wchar_t> m(temp.begin(), temp.end());
        m.push_back(L'\0');

        char sss[800];
        ::WideCharToMultiByte(CP_UTF8, 0, &m[0], (int) m.size(), sss, sizeof(sss), 0, 0);

        fulltext << std::setfill('0') << std::setw(2) << t.wHour << ":"
            << std::setfill('0') << std::setw(2) << t.wMinute << " ";
        fulltext << sss << endl;
    }



    // get bounding rectangle for the text
    std::string fulltext_str(fulltext.str());
    std::vector<char> fulltext_cstr(fulltext_str.begin(), fulltext_str.end());
    fulltext_cstr.push_back('\0');

    char * err = gdImageStringFT(0, &bounds[0], 0, &v[0], fsize, 0.0, 0, 0, &fulltext_cstr[0]);
    if(err)
        cerr << __FUNCTION__ << "gdImageStringFT: " << err;

    int x = bounds[2]-bounds[6] + 6;
    int y = bounds[3]-bounds[7] + 6;

    gdImagePtr p = gdImageCreate(x, y);

    unsigned int 
        r = rr(170, 255), 
        g = rr(170, 255), 
        b = rr(170, 255);

    int brush = gdImageColorResolve(p, r, g, b);
    int pen = gdImageColorResolve(p, 0, 0, 0);

    gdImageFilledRectangle(p, 1, 1, p->sx - 2, p->sy - 2, brush);
    gdImageRectangle(p, 0, 0, p->sx - 1, p->sy - 1, pen);

    x = 3 - bounds[6];
    y = 3 - bounds[7];

    err = gdImageStringFT(p, &bounds[0], pen, &v[0], fsize, 0.0, x, y, &fulltext_cstr[0]);
    if(err) {
        gdImageDestroy(p);
        cerr << __FUNCTION__ << "gdImageStringFT: " << err;
    }

    int size;
    std::ofstream of(file.c_str(), ios::binary | ios::out);
    void * pwrite = gdImagePngPtr(p, &size);
    of.write((char *)pwrite, size);
    gdFree(pwrite);
    gdImageDestroy(p);
}



void Creator::createDynaLastPlayed(const std::vector<const DOMElement *> & arr, const std::basic_string<TCHAR> & file) const
{
    cerr << __FUNCTION__;

    DOMImplementation * impl = DOMImplementationRegistry::getDOMImplementation(XS("LS"));
    DOMImplementationLS * impl_ls = (DOMImplementationLS *) impl;


    DOMDocument * doc = impl->createDocument(XS("root"), XS("signature"), 0);
    //doc->setEncoding(XS("UTF-8"));

    DOMElement * items = doc->createElement(XS("items"));



    int x = 5, y = 5;

    for(std::vector<const DOMElement *>::const_iterator i = arr.begin();
        i != arr.end(); ++i, y += 20)
    {
        SYSTEMTIME t;
        std::wstringstream iss, oss;
        std::wstring ws;

        iss << (*i)->getElementsByTagName(XS("time"))->item(0)->getTextContent();

        iss >> t.wYear >> t.wMonth >> t.wDay >> t.wDayOfWeek >>
            t.wHour >> t.wMinute >> t.wSecond >> t.wMilliseconds;


        oss << std::setfill(L'0') << std::setw(2) << t.wMonth << L"/"
            << std::setfill(L'0') << std::setw(2) << t.wDay << L" "
            << std::setfill(L'0') << std::setw(2) << t.wHour << L":"
            << std::setfill(L'0') << std::setw(2) << t.wMinute << L":"
            << std::setfill(L'0') << std::setw(2) << t.wSecond << " - "
            << (*i)->getElementsByTagName(XS("artist"))->item(0)->getTextContent() << " - "
            << (*i)->getElementsByTagName(XS("track"))->item(0)->getTextContent();

        DOMElement * text = doc->createElement(XS("text"));
        DOMElement * string = doc->createElement(XS("string"));
        DOMElement * position = doc->createElement(XS("position"));
        DOMElement * color = doc->createElement(XS("color"));

        string->appendChild(doc->createTextNode((XMLCh*) oss.str().c_str()));
        color->appendChild(doc->createTextNode(XS("0x000000")));

        oss.str(L"");
        oss << x;
        oss >> ws;
        position->setAttribute(XS("x"), (XMLCh*) ws.c_str());

        oss.str(L"");
        oss << y;
        oss >> ws;
        position->setAttribute(XS("y"), (XMLCh*) ws.c_str());

        text->appendChild(string);
        text->appendChild(position);
        text->appendChild(color);

        items->appendChild(text);
    }

    doc->getDocumentElement()->appendChild(items);


    DOMLSSerializer* writer = impl->createLSSerializer();
    DOMConfiguration* dc = writer->getDomConfig();


    writer->writeToString(doc, 0);
    writer->release();

    delete doc;
}
