#include <QFileDialog>
#include <QPushButton>
#include <QDebug>

#include "imageshape.h"


ImageShapeCustomDialog::ImageShapeCustomDialog(ImageShape* imgS, QString title, QWidget *parent, char* execLabel, void (*callback)(), bool * ischeck)
    :PreviewAttrDialog(imgS, title, parent, execLabel, callback, ischeck)
{
    m_imgShape = imgS;
    m_imgShape->SetPenal(this);

    QPushButton *texButton = new QPushButton("Set Texture");
    layoutNextElement->addWidget(texButton);
    connect(texButton,SIGNAL(clicked()),this,SLOT(LoadTextureImage()));
    connect(texButton,SIGNAL(clicked()),this,SLOT(ValueUpdated()));
}

void ImageShapeCustomDialog::SetNewSize(double w, double h)
{
    m_returnWidth->blockSignals(true);
    m_returnHeight->blockSignals(true);
    m_returnWidth->setValue(w);
    m_returnHeight->setValue(h);
    m_returnWidth->blockSignals(false);
    m_returnHeight->blockSignals(false);
}

void ImageShapeCustomDialog::LoadTextureImage(int cur)
{
    m_imgShape->m_curTexture = cur;
    LoadTextureImage();
}

void ImageShapeCustomDialog::LoadTextureImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, QPushButton::tr("Open Image"), "/home/", QPushButton::tr("Image Files (*.png *.jpg *.bmp)"));
    int channel = (Session::get()->channel() <  ACTIVE_CHANNELS) ? Session::get()->channel() : (ACTIVE_CHANNELS-1);
    m_imgShape->m_fileName[channel] = fileName;
    m_imgShape->m_texUpdate  = 1 << channel;

   /* switch(m_imgShape->m_curTexture)
    {
    case 0:
        m_imgShape->m_SMFile = fileName;
        m_imgShape->m_texUpdate = ImageShape::UPDATE_SM;
        break;
    case 1:
        m_imgShape->m_DarkFile = fileName;
        m_imgShape->m_texUpdate = ImageShape::UPDATE_DARK;
        break;
    case 2:
        m_imgShape->m_BrightFile = fileName;
        m_imgShape->m_texUpdate = ImageShape::UPDATE_BRIGHT;
        break;
    case 4:
        m_imgShape->m_DispFile = fileName;
        m_imgShape->m_texUpdate = ImageShape::UPDATE_DISP;
        break;
    }*/
}

ImageShape::ImageShape(float w, float h)
{
    m_width = w;
    m_height = h;
    m_texUpdate = UPDATE_SM|UPDATE_DARK|UPDATE_BRIGHT;
    m_curTexture = 0;
}

ImageShape::~ImageShape()
{
    glDeleteTextures(1,&m_texDark);
    glDeleteTextures(1,&m_texBright);
    glDeleteTextures(1,&m_texSM);
    glDeleteTextures(1,&m_texDisp);
}

void ImageShape::getBBox(BBox& bbox) const{
    bbox.P[0].set(-m_width, -m_height); //_radX>0?-_radX:_radX, _radY>0?-_radY:_radY);
    bbox.P[1].set(m_width, m_height); //_radX>0?_radX:-_radX, _radY>0?_radY:-_radY);
}

void ImageShape::onApplyT(const Matrix3x3& tM)
{
    Vec3 v = tM*Vec3(m_width, m_height, 0);
    m_width = v.x;
    m_height = v.y;

    v = tM*Vec3(0, 0, 1);
    pP()->set(P() + Point(v.x/v.z, v.y/v.z));
}

/*
void ImageShape::calAverageNormal()
{
    if(!m_SMimg.isNull())
    {
        _shaderParam.m_averageNormal = QVector2D(0.0,0.0);
        int m = 0;
        long int av_r = 0,av_g = 0, av_b;
        for(int i=0;i<m_SMimg.width();i++)
            for(int j=0;j<m_SMimg.height();j++)
            {
                QRgb a_color = m_SMimg.pixel(i,j);
                if(qAlpha(a_color)>m_alpha_th)
                {
                    float r = qRed(a_color);
                    float g = qGreen(a_color);
                    av_r += r;
                    av_g += g;
                    m++;
                }
            }
        float f_r = (float)av_r*2/m/255-1;
        float f_g = (float)av_g*2/m/255-1;
//        float f_b = 1-f_r*f_r-f_g*f_g;
        //this rgb is normalized
//        if(f_b>=0)
//            f_b = sqrt(f_b);
//        else
//            //if this rgb is not normalized
//            f_b = av_b*2/m/255-1;
        _shaderParam.m_averageNormal = QVector2D(f_r,f_g);
        qDebug()<<"new normal"<< _shaderParam.m_averageNormal;
    }
}
*/

void ImageShape::SetPenal(ImageShapeCustomDialog *penal)
{
    m_penal = penal;
}
