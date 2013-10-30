#ifndef MESHSHAPE_H
#define MESHSHAPE_H

#include "../Shape.h"
#include "CMesh.h"
#include "MeshData.h"

using namespace dlfl;

class MeshShape : public Shape
{

protected:
    Mesh_p _control;

    //overridden functions
    void render() const;
    void onClick(const Point &, Click_e);
    void onUpdate();
    //void onOutdate();


    void render(Edge_p) const;
    void render(Face_p) const;
    void execOP(const Point&);

    Vertex_p addMeshVertex();
    Vertex_p addMeshVertex(const Point&);

public:

    MeshShape(Mesh_p control = 0);

    //primitives
    static MeshShape*       insertGrid(const Point& p, double len, int n, int m, MeshShape* pMS = 0);
    static MeshShape*       insertNGon(const Point& p, int n, int segv, double rad, MeshShape* pMS = 0);
    static MeshShape*       insertTorus(const Point& p, int n, double rad, MeshShape* pMS = 0);


    static bool             isSMOOTH;
    static double           EXTRUDE_T;

    enum OPERATION_e        {NONE, EXTRUDE_EDGE, EXTRUDE_FACE, DELETE_FACE, SPLIT_FACE,
                             INSERT_SEGMENT, INSERT_GRID, INSERT_2NGON, INSERT_TORUS, INSERT_SPINE};

    enum SELECTION_e        {NOSELECT, EDGE, FACE, CORNER, EDGE_EDGE};

    static void             setOPMODE(OPERATION_e eMode);
    static SELECTION_e      GetSelectMode();
    inline static bool      IsSelectMode(SELECTION_e eMode);
    void                    makeSmoothTangents();

    static double GRID_LEN;
    static int GRID_N;
    static int GRID_M;
    static int NGON_N;
    static int NGON_SEG_V;
    static double NGON_RAD;
    static int    TORUS_N;
    static double TORUS_RAD;

private:

    static OPERATION_e _OPMODE;

    //mesh operations
    void                    insertSegment(Edge_p, const Point&);
    void                    diagonalDivide(Corner_p);
    Face_p                  extrude(Face_p, double);
    Edge_p                  extrude(Edge_p, double);
    void                    deleteFace(Face_p);

    //helper functions
    void                    onSplitEdge(Corner_p, double t);

};

//Callbacks
void                        onInsertEdge(Edge_p);
void                        onAddFace(Face_p);
Bezier*                     initCurve(Edge_p);


void                        executeOperation();



#endif // MESHSHAPE_H
