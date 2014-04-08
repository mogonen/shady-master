#include <string>
#include <sstream>

#include "fileio.h"
#include "shape.h"
#include "MeshShape/cmesh.h"
#include "MeshShape/meshshape.h"
#include "MeshShape/curvededge.h"
#include "ellipseshape.h"

#ifndef MODELING_MODE
#include "Renderer/imageshape.h"
#endif

struct SVLoad{
    ShapeVertex* sv;
    int parent_id, pair_id;
    SVLoad(ShapeVertex* sv_, int parent, int pair){
        sv = sv_;
        parent_id = parent;
        pair_id = pair;
    }
};

struct EdgeLoad
{
    Edge_p e;
    int f0, c0, f1, c1;
    EdgeLoad(Edge_p e_, int f0_, int c0_, int f1_, int c1_ )
    {
        e = e_;
        f0 = f0_;
        c0 = c0_;
        f1 = f1_;
        c1 = c1_;
    }
};

typedef std::map<int, SVLoad*>          SVLoadMap;
typedef std::map<int, Vertex_p>         VertexLoadMap;
typedef std::map<int, Edge_p>           EdgeLoadMap;
typedef std::map<int, Face_p>           FaceLoadMap;
typedef std::vector<std::string>        StringVec;

void split(const std::string &s, char delim, StringVec &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty())
            elems.push_back(item);
    }
}

bool DefaultIO::load(const char *fname)
{

    _infile.open (fname);
    std::string readline;
    bool isok = true;

    Shape* pShape = 0;
    ShapeList shapes;

    while(std::getline(_infile, readline))
    {
        if (readline.empty())
            continue;

        StringVec toks;
        std::string line = readline;
        split(readline, ' ', toks);
        std::string label = toks[0];

        if (label.compare("s")==0)
        {
            pShape = parseShape(line.c_str());
            if (!pShape){
                isok = false;
                break;
            }
            shapes.push_back(pShape);
        }
        else if (pShape && label.compare("sv")==0)
        {
            float px, py, nx, ny, nz;
            int id, parent, pair;

            sscanf(toks[1].c_str(),"%d/%d/%d", &id, &parent, &pair);
            sscanf(toks[2].c_str(),"%f", &px);
            sscanf(toks[3].c_str(),"%f", &py);

            sscanf(toks[4].c_str(),"%f", &nx);
            sscanf(toks[5].c_str(),"%f", &ny);
            sscanf(toks[6].c_str(),"%f", &nz);

            ShapeVertex* sv = pShape->addVertex(Point(px, py));
            sv->pN()->set(nx, ny, nz);
            Load* load = new Load(sv);
            load->keys[Load::PARENT] = parent;
            load->keys[Load::PAIR] = pair;
            _loadmap[id] = load;

        }
        else if (pShape && label.compare("#shapedata")==0)
        {
            switch(pShape->type())
            {
            case MESH_SHAPE:
                parseMeshShape((MeshShape*)pShape);
                break;

            case ELLIPSE_SHAPE:
                 parseEllipseShape((EllipseShape*)pShape);
                break;

            case IMAGE_SHAPE:
                 parseImageShape((ImageShape*)pShape);
                break;
            }
        }

    }

    _infile.close();
    FOR_ALL_ITEMS(LoadMap, _loadmap)
    {
        //int id = it->first;
        Load* load = it->second;
        if (load->keys[Load::PARENT])
        {
            ((Draggable_p)(_loadmap[load->keys[Load::PARENT]]->pObj))->adopt((Draggable_p)load->pObj);
        }

        if (load->keys[Load::PAIR])
        {
           ((ShapeVertex_p)(_loadmap[load->keys[Load::PAIR]]->pObj))->setPair((ShapeVertex_p)load->pObj);
        }
    }

    FOR_ALL_ITEMS(ShapeList, shapes)
    {
        Shape* pShape = (*it);
        pShape->update();
        Session::get()->insertShape(pShape);
    }
    return isok;
}


bool DefaultIO::read(Shape * pShape)
{

    switch(pShape->type())
    {
    case MESH_SHAPE:
        parseMeshShape((MeshShape*)pShape);
        break;

    case ELLIPSE_SHAPE:
         parseEllipseShape((EllipseShape*)pShape);
        break;

    case IMAGE_SHAPE:
         parseImageShape((ImageShape*)pShape);
        break;
    }
}

Shape* DefaultIO::parseShape(const char* line)
{

    char label[1];
    float x, y;
    int type, id, parent;
    sscanf(line,"%s %f %f %d/%d/%d",&label, &x, &y, &type, &id, &parent);
    if (label[0] != 's')
        return 0;

    Shape* pShape = 0;
    switch(type){

    case MESH_SHAPE:
        pShape = new MeshShape();
        break;

    case ELLIPSE_SHAPE:
        pShape = new EllipseShape();
        break;

    case IMAGE_SHAPE:
        pShape = new ImageShape();
        break;
    }

    pShape->translate(Vec2(x,y));
    return pShape;
}

bool DefaultIO::parseMeshShape(MeshShape * pMS)
{

    Mesh_p pMesh = pMS->mesh();

    std::string line;
    VertexLoadMap vertmap;
    FaceLoadMap facemap;

    while(std::getline(_infile, line))
    {
        if (line.empty())
            break;

        StringVec toks;
        split(line, ' ', toks);
        std::string label = toks[0];

        if (label.compare("f")==0)
        {

            int size = toks.size()-3;
            Face_p pF = pMesh->addFace(size, toks[size+2].compare("1")==0 );

            int fid;
            sscanf(toks[1].c_str(),"%d", &fid);
            facemap[fid] = pF;
            for(int i=0; i < size; i++){
                int svid;
                sscanf(toks[i+2].c_str(),"%d", &svid);
                ShapeVertex_p sv = (ShapeVertex_p)_loadmap[svid]->pObj;
                VertexLoadMap::iterator it = vertmap.find(svid);
                Vertex_p pV = 0;
                if (it == vertmap.end()){//create new vertex
                    pV = pMS->addMeshVertex(sv);
                    vertmap[svid] = pV;
                }else
                    pV = it->second;

                pV->set(pF->C(i));
            }
            pF->update();
        }
        else if (label.compare("e")==0)
        {
            int c0f, c0i,c1f, c1i, sv1, sv2;
            sscanf(toks[1].c_str(), "%d/%d", &c0f, &c0i);
            sscanf(toks[2].c_str(), "%d/%d", &c1f, &c1i);
            sscanf(toks[3].c_str(), "%d", &sv1);
            sscanf(toks[4].c_str(), "%d", &sv2);

            Edge_p pE = pMesh->addEdge(0);
            pE->set(facemap[c0f]->C(c0i), 0);
            pE->set(facemap[c1f]->C(c1i), 1);
            pE->pData = new CurvedEdge(pE, 0);

            if (sv1 && sv2)
                pE->pData->init((ShapeVertex_p)_loadmap[sv1]->pObj, (ShapeVertex_p)_loadmap[sv2]->pObj);
            else
                pE->pData->init(0,0);
        }
    }
}

bool DefaultIO::parseEllipseShape(EllipseShape* pShape)
{
    std::string line;
    std::getline(_infile, line);
    StringVec toks;
    split(line, ' ', toks);
    if (toks[0].compare("rad"))
    {
        float rx, ry;
        sscanf(toks[1].c_str(), "%f", &rx);
        sscanf(toks[2].c_str(), "%f", &ry);
        pShape->setRad(rx, ry);
    }
}

bool DefaultIO::parseImageShape(ImageShape* pShape)
{
#ifndef MODELING_MODE

    /*legend:
     *
        double                      m_alpha_th;
        double                      m_stretch;
        double                      m_assignedDepth;
        double                      m_width;
        double                      m_height;

        int                         m_texUpdate;
        bool                        m_shadowCreator;
        GLuint                      m_texSM;
        GLuint                      m_texDark;
        GLuint                      m_texBright;
        GLuint                      m_texDisp;

        QString                     m_SMFile;
        QString                     m_DarkFile;
        QString                     m_BrightFile;
        QString                     m_DispFile;
    */

    float alpha, strech, depth, width, height;
    std::string line;
    std::getline(_infile, line);
    sscanf(line.c_str(), "%f, %f, %f, %f, %f", &alpha, &strech, &depth, &width, &height);
    pShape->m_alpha_th = alpha;
    pShape->m_stretch = strech;
    pShape->m_assignedDepth = depth;
    pShape->m_width = width;
    pShape->m_height = height;

    int text, shadow, sm, dark, bright, disp;
    std::getline(_infile, line);
    sscanf(line.c_str(), "%d, %d, %d, %d, %d", &text, &shadow, &sm, &dark, &bright, &disp);
    //pShape->m_texUpdate         = text;
    pShape->m_shadowCreator     = shadow;
    pShape->m_texSM             = sm;
    pShape->m_texDark           = dark;
    pShape->m_texBright         = bright;
    pShape->m_texDisp           = disp;

    int channel = 0;
    while(std::getline(_infile, line)){
        if (line.empty())
            break;
        pShape->m_fileName[channel] =  QString::fromStdString(line);
        pShape->m_texUpdate = pShape->m_texUpdate  | (1 << channel);
        channel++;
    }

#endif
}